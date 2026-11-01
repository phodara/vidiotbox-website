#!/usr/bin/env python3
"""Write the sample haiku into generated-haikus.html (brew install emscripten).

The page fills its crawl in the browser, after a 15 MB model download, so search engines and a visitor whose
model is still loading see no haiku. This writes a few real ones into the page between the "samples" markers:
Space poems in the Free style (the page's default theme and style), each the first poem a fresh visitor gets
for that poem number, from the native host that matches the WebAssembly bit for bit
(scripts/build-native-reference.sh). The numbers come in a fixed random order, so the poems change only when the
generator does; a poem that repeats a line of an earlier sample is skipped, not chosen by taste.
Run it after every update of scripts/haiku-src, with the parity fixtures.
"""
import html
import json
import os
from pathlib import Path
import random
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
PAGE = ROOT / 'generated-haikus.html'
START, END = '<!-- samples: start (scripts/write-sample-haiku.py) -->', '<!-- samples: end -->'
COUNT = 6
CANDIDATES = random.Random(20261101).sample(range(1, 1000000000), 60)

with tempfile.TemporaryDirectory() as work:
    decoder = Path(work) / 'decoder'
    subprocess.run([str(ROOT / 'scripts/build-native-reference.sh'), str(decoder)], check=True, stdout=subprocess.DEVNULL)
    poems, seen = [], set()
    for case in CANDIDATES:
        if len(poems) == COUNT:
            break
        out = subprocess.run([str(decoder), str(ROOT / 'haiku-generator/model.bin'), 'grammar', '1', 'space'], check=True,
                             capture_output=True, text=True, env={**os.environ, 'HAIKU_START_CASE': str(case)}).stdout
        poem = json.loads(out)
        assert poem['complete'] and poem['syllables'] == [5, 7, 5] and poem['case'] == case, poem
        lines = {line.rstrip('.').rstrip().lower() for line in poem['lines']}
        if lines & seen:
            continue
        seen |= lines
        poems.append(poem)
    assert len(poems) == COUNT, 'not enough samples without a repeated line'

# The same markup as addPoem() in the page.
articles = ''.join(f'<article class="poem"><h3>Space · No. {p["case"]}</h3><p>'
                   + '<br>'.join(f'<span>{html.escape(line)}</span>' for line in p['lines']) + '</p></article>\n'
                   for p in poems)
page = PAGE.read_text()
assert page.count(START) == 1 and page.count(END) == 1, 'sample markers missing from generated-haikus.html'
before, rest = page.split(START)
_, after = rest.split(END)
PAGE.write_text(before + START + '\n' + articles + END + after)
for p in poems:
    print(f'No. {p["case"]}: ' + ' / '.join(p['lines']))
