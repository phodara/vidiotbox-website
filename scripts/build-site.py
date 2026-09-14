#!/usr/bin/env python3
"""Build the static site with content-versioned local image and script URLs."""

import argparse
import hashlib
import html
from pathlib import Path
import re
import shutil
from urllib.parse import parse_qsl, unquote, urlencode, urlsplit, urlunsplit

ROOT = Path(__file__).resolve().parents[1]
ASSET_EXTENSIONS = {'.png', '.jpg', '.jpeg', '.gif', '.svg', '.webp', '.ico', '.avif', '.js', '.css'}
QUOTED_URL = re.compile(r'''(?P<quote>['"])(?P<url>[^'"<>\r\n]+)(?P=quote)''')


def version_urls(text, page, root):
    """Version complete quoted URLs, including literal URLs in inline scripts."""
    def replace(match):
        original = match['url']
        try:
            url = urlsplit(html.unescape(original))
        except ValueError:
            return match[0]
        if url.scheme and url.scheme not in ('http', 'https'):
            return match[0]
        if url.netloc and url.netloc not in ('vidiotbox.net', 'www.vidiotbox.net'):
            return match[0]
        path = Path(unquote(url.path))
        if path.suffix.lower() not in ASSET_EXTENSIONS:
            return match[0]
        asset = ((root / str(path).lstrip('/')) if url.path.startswith('/') or url.netloc
                 else (page.parent / path)).resolve()
        if root not in asset.parents or not asset.is_file():
            return match[0]
        version = hashlib.sha256(asset.read_bytes()).hexdigest()[:12]
        query = [(key, value) for key, value in parse_qsl(url.query, keep_blank_values=True) if key != 'v']
        query.append(('v', version))
        result = urlunsplit((url.scheme, url.netloc, url.path, urlencode(query), url.fragment))
        if '&amp;' in original:
            result = result.replace('&', '&amp;')
        return match['quote'] + result + match['quote']

    return QUOTED_URL.sub(replace, text)


def build(root, output):
    root, output = root.resolve(), output.resolve()
    if output == root or output in root.parents:
        raise ValueError('Output must not be the source directory or its parent')
    if output.exists() and any(output.iterdir()):
        raise ValueError('Use an empty output directory to avoid leaving stale files')
    sources = []
    for source in sorted(root.rglob('*')):
        relative = source.relative_to(root)
        if output == source or output in source.parents:
            continue
        if any(part.startswith('.') for part in relative.parts) or source.is_symlink() or not source.is_file():
            continue
        if '.backup' in source.name:
            continue
        if relative.parts[0] in ('assets', 'docs', 'firmware', 'stats') or (
            len(relative.parts) == 1 and (source.suffix in ('.html', '.xml', '.txt', '.vcf') or source.name in ('CNAME', 'LICENSE'))
        ):
            sources.append(source)
    output.mkdir(parents=True, exist_ok=True)
    for source in sources:
        destination = output / source.relative_to(root)
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)
    pages = list(output.rglob('*.html'))
    for page in pages:
        page.write_text(version_urls(page.read_text(), page, output))
    print(f'Built {len(sources)} files; versioned asset references in {len(pages)} HTML pages: {output}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / '_site')
    args = parser.parse_args()
    build(ROOT, args.output)
