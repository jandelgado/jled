#!/usr/bin/env python3
"""
JLed Documentation Site Generator

This script generates a static documentation site from git tags and the master branch.
It creates a version-aware microsite with navigation and version switching.
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import List, Dict, Any

import markdown
from markdown.extensions.toc import TocExtension
from jinja2 import Environment, FileSystemLoader
from packaging import version


def run_git_command(cmd: List[str], cwd: str = None) -> str:
    """Run a git command and return the output."""
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=True,
            cwd=cwd
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        print(f"Error running git command: {' '.join(cmd)}", file=sys.stderr)
        print(f"Error output: {e.stderr}", file=sys.stderr)
        raise


def get_versions() -> List[str]:
    """
    Fetch all git tags matching v*.*.*, filter out pre-releases,
    sort by semver, and return with 'master' at the beginning.
    """
    # Get all tags matching pattern
    tags_output = run_git_command(['git', 'tag', '-l', 'v*.*.*'])

    if not tags_output:
        tags = []
    else:
        tags = tags_output.split('\n')

    # Filter out pre-release tags (containing alpha, beta, rc, pre)
    stable_tags = []
    for tag in tags:
        if tag and not re.search(r'(alpha|beta|rc|pre)', tag, re.IGNORECASE):
            stable_tags.append(tag)

    # Sort by semver (newest first)
    try:
        sorted_tags = sorted(
            stable_tags,
            key=lambda t: version.parse(t.lstrip('v')),
            reverse=True
        )
    except Exception as e:
        print(f"Warning: Error sorting versions: {e}", file=sys.stderr)
        sorted_tags = stable_tags

    # Add master at the beginning
    return ['master'] + sorted_tags


def get_latest_stable(versions: List[str]) -> str:
    """Return the latest stable version (first non-master version)."""
    for v in versions:
        if v != 'master':
            return v
    return 'master'  # Fallback if no releases exist


def extract_readme_structure(readme_path: str) -> tuple[str, List[Dict[str, Any]]]:
    """
    Parse README.md and extract HTML content plus navigation structure.
    Returns (html_content, navigation_items).
    """
    with open(readme_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Filter out badge lines (CI/CD status badges)
    # Matches: ![text](url-with-badge) or [![text](badge)](link)
    badge_pattern = r'^\s*!?\[!\[.*?\]\(.*?\)\]\(.*?\)\s*$|^\s*!\[.*?\]\(.*?badge.*?\)\s*$'
    lines = content.split('\n')
    filtered_lines = [line for line in lines if not re.match(badge_pattern, line)]
    content = '\n'.join(filtered_lines)

    # Configure markdown with TOC extension
    md = markdown.Markdown(
        extensions=[
            'extra',
            'codehilite',
            TocExtension(
                toc_depth='1-3',
                anchorlink=True,
                permalink=False
            )
        ],
        extension_configs={
            'codehilite': {
                'css_class': 'highlight',
                'guess_lang': True,
            }
        }
    )

    # Convert markdown to HTML
    html = md.convert(content)

    # Extract navigation from TOC tokens
    navigation = []
    if hasattr(md, 'toc_tokens'):
        def extract_items(items, level=1):
            for item in items:
                navigation.append({
                    'level': level,
                    'text': item['name'],
                    'anchor': item['id']
                })
                # Recursively process children
                if 'children' in item:
                    extract_items(item['children'], level + 1)

        extract_items(md.toc_tokens)

    return html, navigation


def get_examples(examples_path: str) -> List[Dict[str, str]]:
    """
    Scan the examples/ directory and return a list of example subdirectories.
    Returns list of {name, path} dicts.
    """
    examples = []

    if not os.path.exists(examples_path):
        return examples

    try:
        for entry in sorted(os.listdir(examples_path)):
            entry_path = os.path.join(examples_path, entry)
            # Only include directories
            if os.path.isdir(entry_path):
                examples.append({
                    'name': entry,
                    'path': entry_path
                })
    except OSError as e:
        print(f"Warning: Error reading examples directory: {e}", file=sys.stderr)

    return examples


def copy_directory_contents(src: str, dst: str):
    """Copy all contents from src directory to dst directory."""
    if not os.path.exists(src):
        return

    os.makedirs(dst, exist_ok=True)

    for item in os.listdir(src):
        src_item = os.path.join(src, item)
        dst_item = os.path.join(dst, item)

        if os.path.isdir(src_item):
            shutil.copytree(src_item, dst_item, dirs_exist_ok=True)
        else:
            shutil.copy2(src_item, dst_item)


def checkout_version(version: str, work_dir: str):
    """Checkout a specific version in the given work directory."""
    run_git_command(['git', 'checkout', version], cwd=work_dir)


def generate_site(output_dir: str, script_dir: str):
    """
    Main site generation function.
    Iterates through all versions, generates pages, and creates index.
    """
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    # Set up Jinja2 environment
    templates_dir = os.path.join(script_dir, 'templates')
    jinja_env = Environment(loader=FileSystemLoader(templates_dir))

    # Get all versions
    print("Discovering versions...")
    versions = get_versions()
    latest_stable = get_latest_stable(versions)

    print(f"Found {len(versions)} versions: {', '.join(versions)}")
    print(f"Latest stable: {latest_stable}")

    # Create a temporary git worktree for clean checkouts
    repo_root = run_git_command(['git', 'rev-parse', '--show-toplevel'])

    with tempfile.TemporaryDirectory() as temp_dir:
        work_dir = os.path.join(temp_dir, 'worktree')

        # Create worktree
        print(f"Creating temporary worktree at {work_dir}...")
        run_git_command(['git', 'worktree', 'add', work_dir, 'master'])

        try:
            # Process each version
            for ver in versions:
                print(f"\nProcessing version: {ver}")

                # Checkout version
                checkout_version(ver, work_dir)

                # Set up paths
                version_dir = output_path / ver
                version_dir.mkdir(parents=True, exist_ok=True)

                readme_path = os.path.join(work_dir, 'README.md')
                doc_path = os.path.join(work_dir, 'doc')
                examples_path = os.path.join(work_dir, 'examples')

                # Extract README content and structure
                if os.path.exists(readme_path):
                    readme_html, navigation = extract_readme_structure(readme_path)
                else:
                    print(f"Warning: README.md not found for {ver}")
                    readme_html = "<p>Documentation not available for this version.</p>"
                    navigation = []

                # Get examples
                examples = get_examples(examples_path)
                print(f"  Found {len(examples)} examples")

                # Copy static assets
                if os.path.exists(doc_path):
                    copy_directory_contents(doc_path, str(version_dir / 'doc'))
                    print(f"  Copied doc/ directory")

                if os.path.exists(examples_path):
                    copy_directory_contents(examples_path, str(version_dir / 'examples'))
                    print(f"  Copied examples/ directory")

                # Render HTML page
                template = jinja_env.get_template('base.html')
                html_content = template.render(
                    current_version=ver,
                    versions=versions,
                    latest_stable=latest_stable,
                    navigation=navigation,
                    examples=examples,
                    readme_html=readme_html
                )

                # Write HTML file
                index_path = version_dir / 'index.html'
                with open(index_path, 'w', encoding='utf-8') as f:
                    f.write(html_content)

                print(f"  Generated {index_path}")

        finally:
            # Clean up worktree
            print("\nCleaning up worktree...")
            run_git_command(['git', 'worktree', 'remove', '--force', work_dir])

    # Generate root redirect page
    print("\nGenerating root redirect page...")
    redirect_template = jinja_env.get_template('redirect.html')
    redirect_html = redirect_template.render(latest_stable=latest_stable)

    with open(output_path / 'index.html', 'w', encoding='utf-8') as f:
        f.write(redirect_html)

    # Generate versions.json metadata
    print("Generating versions.json...")
    versions_data = {
        'versions': versions,
        'latest_stable': latest_stable
    }

    with open(output_path / 'versions.json', 'w', encoding='utf-8') as f:
        json.dump(versions_data, f, indent=2)

    print(f"\n✓ Site generation complete! Output: {output_dir}")
    print(f"  Total versions: {len(versions)}")
    print(f"  Latest stable: {latest_stable}")


def main():
    parser = argparse.ArgumentParser(
        description='Generate JLed documentation site from git versions'
    )
    parser.add_argument(
        '--output',
        '-o',
        default='./site-build',
        help='Output directory for generated site (default: ./site-build)'
    )

    args = parser.parse_args()

    # Get script directory for templates
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Ensure we're in a git repository
    try:
        run_git_command(['git', 'rev-parse', '--git-dir'])
    except subprocess.CalledProcessError:
        print("Error: This script must be run from within a git repository.", file=sys.stderr)
        sys.exit(1)

    # Generate site
    try:
        generate_site(args.output, script_dir)
    except Exception as e:
        print(f"Error generating site: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
