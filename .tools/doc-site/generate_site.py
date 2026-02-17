#!/usr/bin/env python3
"""
JLed Documentation Site Generator

This script generates a static documentation site from git tags and the master branch.
It creates a version-aware microsite with navigation and version switching.
"""

import argparse
import html
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


# File filtering for example pages
INCLUDE_EXTENSIONS = {
    '.ino', '.cpp', '.cc', '.c', '.h', '.hpp',
    '.cmake', '.sh', '.py', '.txt', '.md',
    '.json', '.yaml', '.yml', '.ini'
}

INCLUDE_EXACT_NAMES = {'CMakeLists.txt', 'Makefile', 'README.md', 'Dockerfile'}

EXCLUDE_PATTERNS = {
    r'.*~$',  # Backup files
    r'\.o$', r'\.obj$', r'\.elf$', r'\.bin$', r'\.uf2$', r'\.hex$',
    r'\.a$', r'\.so$', r'\.dylib$', r'\.map$', r'\.dis$',  # Build artifacts
    r'^CMakeCache\.txt$', r'^cmake_install\.cmake$',  # CMake generated
    r'^\.', # Hidden files
}

EXCLUDE_DIRS = {'CMakeFiles', '.vscode', '.idea', 'build', 'dist', '__pycache__'}

MAX_FILE_SIZE = 500 * 1024  # 500KB


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

    # Remove Contents/ToC section — redundant with the sidebar navigation
    content = re.sub(
        r'^#{1,6}\s+Contents\s*\n.*?(?=^#|\Z)',
        '',
        content,
        flags=re.MULTILINE | re.DOTALL
    )

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


def should_include_file(filename: str, filepath: str) -> bool:
    """Check if file should be included in example page."""
    # Check size
    try:
        if os.path.getsize(filepath) > MAX_FILE_SIZE:
            return False
    except OSError:
        return False

    # Check exclude patterns
    for pattern in EXCLUDE_PATTERNS:
        if re.search(pattern, filename):
            return False

    # Check include by extension or exact name
    ext = os.path.splitext(filename)[1]
    if filename in INCLUDE_EXACT_NAMES:
        return True
    if ext in INCLUDE_EXTENSIONS:
        return True

    return False


def detect_language(filename: str) -> str:
    """Detect programming language from filename for Pygments."""
    ext = os.path.splitext(filename)[1]

    # Special filenames first
    if filename == 'CMakeLists.txt':
        return 'cmake'
    elif filename == 'Makefile':
        return 'make'
    elif filename == 'Dockerfile':
        return 'docker'

    # Extension mapping
    language_map = {
        '.ino': 'cpp',
        '.cpp': 'cpp',
        '.cc': 'cpp',
        '.c': 'c',
        '.h': 'cpp',
        '.hpp': 'cpp',
        '.cmake': 'cmake',
        '.sh': 'bash',
        '.py': 'python',
        '.txt': 'text',
        '.json': 'json',
        '.yaml': 'yaml',
        '.yml': 'yaml',
        '.ini': 'ini',
    }

    return language_map.get(ext, 'text')


def make_safe_anchor(filename: str) -> str:
    """Convert filename to safe HTML anchor ID."""
    safe = re.sub(r'[^a-zA-Z0-9]+', '-', filename)
    return safe.strip('-').lower()


def highlight_code(content: str, language: str) -> str:
    """Apply Pygments syntax highlighting to code content."""
    if language == 'text':
        escaped = html.escape(content)
        return f'<pre><code>{escaped}</code></pre>'

    try:
        from pygments import highlight
        from pygments.lexers import get_lexer_by_name
        from pygments.formatters import HtmlFormatter

        lexer = get_lexer_by_name(language, stripall=False)
        formatter = HtmlFormatter(
            cssclass='highlight',
            linenos=False,
            style='default',
            noclasses=False
        )

        return highlight(content, lexer, formatter)
    except Exception as e:
        print(f"    Warning: Failed to highlight code: {e}", file=sys.stderr)
        escaped = html.escape(content)
        return f'<pre><code>{escaped}</code></pre>'


def parse_markdown_content(content: str, base_path: str = '') -> str:
    """Parse markdown content to HTML, adjusting image paths."""
    # Adjust relative image paths for example READMEs
    if base_path:
        content = re.sub(
            r'!\[(.*?)\]\(\.\./\.\./doc/(.*?)\)',
            rf'![\1]({base_path}\2)',
            content
        )

    md = markdown.Markdown(
        extensions=['extra', 'codehilite'],
        extension_configs={
            'codehilite': {
                'css_class': 'highlight',
                'guess_lang': True,
            }
        }
    )

    return md.convert(content)


def get_example_files(example_path: str) -> List[Dict[str, Any]]:
    """Scan example directory and return list of files to display."""
    files = []

    for root, dirs, filenames in os.walk(example_path):
        # Filter out excluded directories
        dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]

        # Only process root directory
        if root != example_path:
            continue

        for filename in filenames:
            filepath = os.path.join(root, filename)

            if not should_include_file(filename, filepath):
                continue

            ext = os.path.splitext(filename)[1]
            files.append({
                'name': filename,
                'path': filepath,
                'extension': ext,
                'safe_name': make_safe_anchor(filename)
            })

    # Sort: .ino/.cpp first, then others, README.md last
    def sort_key(f):
        name = f['name']
        if name == 'README.md':
            return (2, name)
        elif f['extension'] in ['.ino', '.cpp']:
            return (0, name)
        else:
            return (1, name)

    return sorted(files, key=sort_key)


def generate_example_page(
    example: Dict[str, str],
    version_dir: Path,
    work_dir: str,
    jinja_env: Environment,
    current_version: str,
    versions: List[str],
    latest_stable: str
) -> None:
    """Generate HTML page for a single example."""
    example_name = example['name']
    example_path = example['path']

    print(f"  Generating page for example: {example_name}")

    # Get files to display
    files_info = get_example_files(example_path)

    # Process each file
    files_data = []
    readme_html = None

    for file_info in files_info:
        filename = file_info['name']
        filepath = file_info['path']

        # Read file content
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"    Warning: Could not read {filename}: {e}")
            continue

        # Handle README.md separately
        if filename == 'README.md':
            readme_html = parse_markdown_content(content, base_path='../../doc/')
            continue

        # Detect language and highlight
        language = detect_language(filename)
        highlighted = highlight_code(content, language)

        files_data.append({
            'name': filename,
            'safe_name': file_info['safe_name'],
            'highlighted_content': highlighted
        })

    # Render template
    template = jinja_env.get_template('example.html')
    html_content = template.render(
        example_name=example_name,
        current_version=current_version,
        versions=versions,
        latest_stable=latest_stable,
        files=files_data,
        readme_html=readme_html
    )

    # Write HTML file
    example_output_dir = version_dir / 'examples' / example_name
    example_output_dir.mkdir(parents=True, exist_ok=True)

    output_path = example_output_dir / 'index.html'
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(html_content)

    print(f"    Generated {output_path}")


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

                # Generate individual example pages
                if examples:
                    print(f"  Generating {len(examples)} example pages...")
                    for example in examples:
                        generate_example_page(
                            example=example,
                            version_dir=version_dir,
                            work_dir=work_dir,
                            jinja_env=jinja_env,
                            current_version=ver,
                            versions=versions,
                            latest_stable=latest_stable
                        )

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
