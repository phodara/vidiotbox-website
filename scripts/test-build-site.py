#!/usr/bin/env python3
"""Check cache versions against actual file changes and URL edge cases."""

import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('build_site', Path(__file__).with_name('build-site.py'))
build_site = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build_site)


class AssetVersions(unittest.TestCase):
    def test_changed_asset_only_and_idempotence(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder).resolve()
            (root / 'one.png').write_bytes(b'original')
            (root / 'menu.js').write_text('/* menu */')
            page = root / 'index.html'
            source = '<img src="one.png"><script src="menu.js"></script>'
            first = build_site.version_urls(source, page, root)
            self.assertEqual(first, build_site.version_urls(first, page, root))
            (root / 'one.png').write_bytes(b'changed')
            second = build_site.version_urls(first, page, root)
            self.assertNotEqual(first.split('<script')[0], second.split('<script')[0])
            self.assertEqual(first.split('<script')[1], second.split('<script')[1])
            self.assertEqual(second.count('?v='), 2)

    def test_paths_queries_external_urls_and_dynamic_image_list(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder).resolve()
            (root / 'assets').mkdir()
            (root / 'assets' / 'sign space.png').write_bytes(b'image')
            (root / 'docs').mkdir()
            page = root / 'docs' / 'index.html'
            for url in ('../assets/sign%20space.png', '/assets/sign%20space.png',
                        'https://www.vidiotbox.net/assets/sign%20space.png'):
                result = build_site.version_urls(f'"{url}?size=2&v=old#icon"', page, root)
                self.assertIn('?size=2&v=', result)
                self.assertIn('#icon', result)
                self.assertNotIn('v=old', result)
            source = "['assets/sign space.png', 'A sign']"
            self.assertIn('sign space.png?v=', build_site.version_urls(source, root / 'index.html', root))
            for url in ('https://example.com/sign.png', 'missing.png', 'data:image/png;base64,abc'):
                self.assertEqual(f'"{url}"', build_site.version_urls(f'"{url}"', page, root))

    def test_haiku_generator_assets_are_published(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder).resolve()
            generator = root / 'haiku-generator'
            generator.mkdir()
            for name in ('index.html', 'worker.js', 'decoder.mjs', 'decoder.wasm', 'model.bin', 'model-info.json'):
                (generator / name).write_bytes(b'fixture')
            source = root / 'scripts/haiku-src'
            source.mkdir(parents=True)
            (source / 'host.cpp').write_text('not a web asset')
            output = root / '_site'
            build_site.build(root, output)
            for asset in generator.iterdir():
                self.assertTrue((output / 'haiku-generator' / asset.name).is_file())
            self.assertFalse((output / 'scripts').exists())

    def test_build_excludes_private_and_backup_files(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder).resolve()
            (root / 'index.html').write_text('<img src="image.png">')
            (root / 'index.html.backup').write_text('backup')
            (root / '.git').mkdir()
            (root / '.git' / 'config').write_text('private')
            output = root / '_site'
            build_site.build(root, output)
            self.assertTrue((output / 'index.html').is_file())
            self.assertFalse((output / '.git').exists())
            self.assertFalse((output / 'index.html.backup').exists())
            with self.assertRaises(ValueError):
                build_site.build(root, output)


if __name__ == '__main__':
    unittest.main()
