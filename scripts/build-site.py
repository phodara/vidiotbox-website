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


def render_haiku_about(page, *, staging=True):
    """Reuse the dialog verbatim; never maintain a second copy of its content."""
    dialog = re.search(r'<dialog id="aboutDialog"[^>]*>(.*?)</dialog>', page, re.S)
    if not dialog:
        raise ValueError('Missing Haiku-Matic About dialog')
    content, separator, _ = dialog[1].partition('<form method="dialog">')
    if not separator:
        raise ValueError('Missing About dialog close form')
    styles = re.search(r'<style>(.*?)</style>', page, re.S)[1]
    home = 'generated-haikus.html' if staging else './'
    robots = '<meta name="robots" content="noindex">' if staging else ''
    return f'''<!DOCTYPE html>
<!-- Generated from generated-haikus.html by scripts/build-site.py. Edit the dialog there. -->
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>About Haiku-Matic | How the AI Haiku Generator Works</title>
<meta name="description" content="Learn how Paul Hodara built Haiku-Matic: a browser-based neural network for 5-7-5 haiku. Explore its themes, poetic styles, training history, and credits.">
<meta name="author" content="Paul Hodara">
<link rel="canonical" href="https://haiku-matic.com/haiku-matic-about.html">
<link rel="icon" type="image/png" href="philosophy-assets/happy.png">
{robots}
<style>{styles}
/* This document has no animation, scripts, workers or model download. */
.about-page main {{max-width:760px;margin:auto;padding:26px 28px;}}
.about-page nav {{max-width:760px;margin:auto;padding:20px 28px 0;}}
.about-page h1 {{font-size:28px;line-height:1.2;text-transform:none;margin:0 0 24px;}}
.about-page h3 {{color:var(--cyan);font-size:13px;letter-spacing:2px;margin:0 0 20px;}}
@media(max-width:640px) {{.about-page main {{padding:20px 16px;}} .about-page nav {{padding:20px 16px 0;}}}}
</style>
</head>
<body class="about-page">
<nav aria-label="Haiku-Matic navigation"><a href="{home}">Open the haiku generator</a> · <a href="#instructionsTitle">Instructions</a> · <a href="#creditsTitle">Credits</a></nav>
<main>
<h1>About Haiku-Matic</h1>
{content}
<p><a href="{home}">Return to Haiku-Matic</a></p>
</main>
</body>
</html>
'''


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
        if relative.parts[0] in ('assets', 'docs', 'firmware', 'stats', 'haiku-generator', 'haiku-lab', 'haiku-styles') or (
            len(relative.parts) == 1 and (source.suffix in ('.html', '.xml', '.txt', '.vcf') or source.name in ('CNAME', 'LICENSE'))
        ):
            sources.append(source)
    output.mkdir(parents=True, exist_ok=True)
    for source in sources:
        destination = output / source.relative_to(root)
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)
    haiku_page = output / 'generated-haikus.html'
    if haiku_page.is_file():
        (output / 'haiku-matic-about.html').write_text(render_haiku_about(haiku_page.read_text()))
    pages = list(output.rglob('*.html'))
    for page in pages:
        page.write_text(version_urls(page.read_text(), page, output))
    print(f'Built {len(sources)} files; versioned asset references in {len(pages)} HTML pages: {output}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / '_site')
    args = parser.parse_args()
    build(ROOT, args.output)
