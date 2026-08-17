#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pytest>=8.0",
#     "markdown>=3.6",
#     "Jinja2>=3.1",
#     "packaging>=24.0",
#     "Pygments>=2.17",
# ]
# ///
"""Minimal test suite for the JLed doc-site generator.

Run with: uv run .tools/doc-site/test_generate_site.py
"""

import importlib.util
import os

import pytest

# Import the generator module by path (it is not on sys.path).
_MODULE_PATH = os.path.join(os.path.dirname(__file__), 'generate_site.py')
_spec = importlib.util.spec_from_file_location('generate_site', _MODULE_PATH)
gs = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(gs)


def _write(path, content=''):
    """Create a file (and parent dirs) with the given content."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)


def test_parse_doc_directives_happy(tmp_path):
    example = tmp_path / 'ex'
    _write(str(example / 'Src' / 'app.cpp'), 'int main() {}')
    readme = '<!-- doc:add Src/app.cpp -->\n<!-- doc:link Src -->\n'

    added, linked = gs.parse_doc_directives(readme, str(example))

    assert [f['name'] for f in added] == ['Src/app.cpp']
    assert added[0]['safe_name'] == 'src-app-cpp'
    assert [d['relpath'] for d in linked] == ['Src']


def test_parse_doc_directives_rejects_bad_targets(tmp_path):
    example = tmp_path / 'ex'
    _write(str(example / 'real.cpp'))
    _write(str(example / 'afile'))  # a file, not a directory
    _write(str(tmp_path / 'escape'))  # outside the example dir
    readme = (
        '<!-- doc:add ../escape -->\n'
        '<!-- doc:add missing.cpp -->\n'
        '<!-- doc:link afile -->\n'
    )

    added, linked = gs.parse_doc_directives(readme, str(example))

    assert added == []
    assert linked == []


def test_copy_linked_files_copies_only_linked(tmp_path):
    repo = tmp_path / 'repo'
    _write(str(repo / 'config.ini'), 'x=1')
    _write(str(repo / 'secret.txt'), 'private')  # not linked
    _write(str(repo / 'backup~'), 'bak')         # not linked
    os.makedirs(str(repo / 'sub'))
    readme = (
        'see [cfg](config.ini)\n'
        '[web](http://example.com/x)\n'
        '[top](#anchor)\n'
        '[dir](sub)\n'
    )
    out = tmp_path / 'out'
    out.mkdir()

    gs.copy_linked_files(readme, str(repo), str(repo), out)

    copied = {str(p.relative_to(out)) for p in out.rglob('*') if p.is_file()}
    assert copied == {'config.ini'}


def test_copy_linked_files_guards(tmp_path):
    repo = tmp_path / 'repo'
    _write(str(repo / 'kept.ini'), 'new')
    _write(str(tmp_path / 'outside.txt'), 'secret')  # outside the repo
    out = tmp_path / 'out'
    _write(str(out / 'kept.ini'), 'original')  # already present in output
    readme = '[esc](../outside.txt)\n[kept](kept.ini)\n'

    gs.copy_linked_files(readme, str(repo), str(repo), out)

    assert not (out / 'outside.txt').exists()
    # Existing dest is not overwritten.
    assert (out / 'kept.ini').read_text() == 'original'


if __name__ == '__main__':
    import sys
    sys.exit(pytest.main([__file__, '-q']))
