# JLed Documentation Site Generator

This tool generates a static documentation microsite for JLed from git tags and the master branch. The generated site provides version-aware documentation with easy navigation between different versions.

## Overview

The site generator:
- Discovers all stable git tags (matching `v*.*.*`, excluding pre-releases)
- Generates a separate documentation page for each version
- Includes the master branch alongside released versions
- Provides version switching, page navigation, and examples listing
- Deploys automatically to GitHub Pages via GitHub Actions

## Generated Site Structure

```
/index.html              # Redirects to latest stable version
/versions.json           # Metadata: all versions, latest stable
/v1.0.0/
│   ├── index.html      # Full page with README + nav
│   ├── doc/           # Images/assets from doc/
│   └── examples/      # Example folders from examples/
│       ├── hello/
│       │   ├── index.html    # Example page with syntax-highlighted code
│       │   └── hello.ino
│       └── morse/
│           ├── index.html
│           ├── morse.ino
│           └── README.md
/v2.0.0/
│   ├── ...
/master/
    ├── index.html
    ├── doc/
    └── examples/
```

## Local Usage

### Prerequisites

Install Python dependencies:

```bash
pip install -r requirements.txt
```

Required packages:
- `markdown>=3.6` - README parsing and HTML rendering
- `Jinja2>=3.1` - HTML template rendering
- `packaging>=24.0` - Semantic version parsing and sorting

### Running the Generator

From the repository root:

```bash
python .tools/doc-site/generate_site.py --output /tmp/jled-docs
```

Options:
- `--output`, `-o`: Output directory for generated site (default: `./site-build`)

### Testing Locally

After generation, you can serve the site locally:

```bash
cd /tmp/jled-docs
python -m http.server 8000
```

Then open `http://localhost:8000` in your browser.

**Note:** The version switcher uses absolute paths (`/jled/VERSION/`), so it won't work correctly with `python -m http.server`. To test version switching, either:
- Use a server that supports path rewriting (e.g., nginx, http-server with proxy)
- Manually edit the URLs in the browser
- Test on the actual GitHub Pages deployment

## How It Works

### Version Discovery

1. Runs `git tag -l "v*.*.*"` to find all version tags
2. Filters out pre-release tags (containing `alpha`, `beta`, `rc`, `pre`)
3. Sorts versions using semantic versioning (newest first)
4. Adds `master` at the beginning of the list

### Content Generation

For each version:

1. **Checkout version** using `git worktree` (isolated from main working directory)
2. **Parse README.md**:
   - Convert markdown to HTML
   - Extract H1, H2, H3 headers with anchors for navigation
3. **Scan examples/** directory for subdirectories
4. **Copy assets**:
   - `doc/` folder → `VERSION/doc/`
   - `examples/` folder → `VERSION/examples/`
5. **Generate example pages** (US-03):
   - For each example, create `examples/<name>/index.html`
   - Scan example directory for source files (.ino, .cpp, .h, etc.)
   - Apply syntax highlighting using Pygments
   - Filter out backup files (*~) and build artifacts
   - Render README.md if present
6. **Render HTML** using Jinja2 template with:
   - Version selector dropdown
   - Page navigation (from README headers)
   - Examples list
   - README content

### Example Pages

Each example in `examples/` is processed to create an individual page at `examples/<name>/index.html`:

**Processing:**
- Scans example directory for source files (.ino, .cpp, .h, etc.)
- Filters out backup files (*~) and build artifacts
- Applies syntax highlighting using Pygments
- If README.md exists, renders it at the bottom

**Display:**
- Each file shown with clear header and highlighted code
- Files sorted: main source first (.ino, .cpp), then others, README last
- Sidebar navigation for quick access to specific files
- Version selector maintains context when switching versions

**Supported File Types:**
- C/C++: .ino, .cpp, .c, .h, .hpp
- Build: CMakeLists.txt, Makefile, .cmake
- Scripts: .sh, .py
- Config: .json, .yaml, .ini
- Docs: README.md, .txt

### Root Page

Generates `index.html` with meta-refresh redirect to latest stable version.

### Metadata

Creates `versions.json` with:
```json
{
  "versions": ["master", "v2.0.0", "v1.0.0"],
  "latest_stable": "v2.0.0"
}
```

## CI/CD Deployment

The site is automatically deployed to GitHub Pages on every push to `master` via the `.github/workflows/deploy-docs.yml` workflow.

### Workflow Steps

1. **Checkout** repository with full history (`fetch-depth: 0`)
2. **Setup Python** 3.13
3. **Install dependencies** from `requirements.txt`
4. **Generate site** to `./site-build`
5. **Deploy** to `gh-pages` branch using `peaceiris/actions-gh-pages@v4`

### First-Time Setup

After the first workflow run:

1. Go to repository **Settings** → **Pages**
2. Set **Source** to **Deploy from a branch**
3. Select **Branch**: `gh-pages` / `/ (root)`
4. Click **Save**

The site will be available at: `https://jandelgado.github.io/jled/`

## Architecture

### Key Components

- **`generate_site.py`** - Main orchestration script
  - `get_versions()` - Discover and sort versions
  - `extract_readme_structure()` - Parse README and extract navigation
  - `get_examples()` - Scan examples directory
  - `get_example_files()` - Scan and filter example files
  - `should_include_file()` - File filtering logic
  - `detect_language()` - Map file extensions to Pygments lexers
  - `highlight_code()` - Apply syntax highlighting
  - `parse_markdown_content()` - Parse markdown with image path adjustment
  - `generate_example_page()` - Generate individual example pages
  - `generate_site()` - Main generation loop

- **`templates/base.html`** - Main documentation page template
  - Split layout: sidebar (280px) + content area
  - Version selector dropdown
  - Page navigation from README headers
  - Examples list
  - GitHub-like styling

- **`templates/example.html`** - Example page template
  - Consistent styling with base template
  - File-by-file display with syntax highlighting
  - Navigation: version selector, back link, file list
  - Optional README rendering at bottom

- **`templates/redirect.html`** - Root redirect page

### Design Decisions

**Git Worktree:**
- Uses temporary git worktree for clean version checkouts
- Avoids disrupting main working directory
- Automatically cleaned up after generation

**Path Structure:**
- Version-based paths: `/jled/VERSION/`
- Allows parallel hosting of all versions
- Clean, bookmarkable URLs

**No JavaScript Framework:**
- Simple vanilla JS for version switching
- Minimal dependencies
- Fast page loads

**Static Generation:**
- No server-side rendering needed
- Can be hosted on any static file server
- GitHub Pages compatible

## Troubleshooting

### "Not in a git repository"

Ensure the script is run from within the JLed git repository.

### "No versions found"

Check that git tags exist matching `v*.*.*` pattern:
```bash
git tag -l "v*.*.*"
```

### "Template not found"

Ensure templates exist at `.tools/doc-site/templates/`:
- `base.html`
- `redirect.html`

### Version sorting issues

Versions are sorted using Python's `packaging.version.parse()`. Ensure tags follow semantic versioning (e.g., `v1.0.0`, `v2.1.3`).

### Images not loading

- Check that `doc/` folder exists in the version being built
- Verify image paths in README.md are relative (e.g., `doc/image.png`)
- Check browser console for 404 errors

## Maintenance

### Adding New Features

To modify the generated pages:
1. Edit `templates/base.html` for layout/styling changes
2. Update `generate_site.py` for content/logic changes
3. Test locally before committing
4. Push to master to deploy

### Updating Dependencies

Update `requirements.txt` and test locally:
```bash
pip install -r requirements.txt --upgrade
python generate_site.py --output /tmp/test
```

### Excluding Versions

To exclude specific versions from the site, modify the filtering logic in `get_versions()` in `generate_site.py`.

## Future Enhancements

Possible improvements:
- Search functionality across versions
- Dark mode toggle
- API documentation integration
- Changelog page
- Download links for releases
