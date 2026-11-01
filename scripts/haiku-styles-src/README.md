# Haiku style decoder source

The Style menu on `generated-haikus.html` offers three kinds of poem (and Mixed, one of the three at random):

- **Free** (value `standard`): the unchanged main decoder, `haiku-generator/worker.js` (built from `../haiku-src/`),
  the same poems as the boards.
- **One-thing** and **Two-part** (Space, Spring, Summer, Autumn and Winter): this decoder, `haiku-styles/worker.js`. One-thing keeps
  every line to one concept's space words; two-part writes line one from them and lines 2-3 from the
  concept's earthly image, across the cut. These are the haiku lab's styles, on the current rules.
  Since V8.17 (2026-10-30) both leave technical words (fusion, atoms, density, radio waves...) out of their space
  lines, a two-part line one names something in space (light counts), and a line on is/are loses to one with an
  action (`lab_rules_init()` and `attempt_line()` in the patch; LAB_SPACE_MISS and LAB_COPULA_PENALTY tune them).

Files:

- `styles.patch`: the lab's changes to `host.cpp` (concepts, the line bonus and miss penalty, the
  two-part switch at line two). `scripts/build-haiku-styles.py` applies it to `../haiku-src/host.cpp`
  and writes `styles_host.cpp` (generated; do not edit). New decoder rules therefore reach all three
  styles with a rebuild. If a new `host.cpp` no longer takes the patch, the build stops; update the patch.
- `lab_concepts.h`: the ten concepts, copied from `../haiku-lab-src/` so the lab and this page do not
  affect each other.
  Since 2026-10-31 it also holds SEASON_CONCEPTS: a season poem keeps its written opener, which picks the
  concept (summer, 2026-11-01: summer wind, summer light, night song, bees and beetles, water birds, and since V8.27
  cicadas and fireflies for its five added openers, with a cool shade as a two-part partner; autumn, 2026-10-31: falling leaves, autumn wind, cold rain and mist, crows, fading, with a still pond as a
  two-part partner; winter: falling snow, ice and frost, winter wind, fog, crows, with stillness and winter light as
  two-part partners). One-thing stays with that image; two-part sets it against the partner in lines 2-3. The
  concept's words survive the per-poem palette (`lab_keep`). `--lab-check` checks every word is in its season and
  every opener of a season with concepts has one. A theme without concepts is refused.
  Spring (2026-11-01, first draft, branch `spring-styles`) has concepts but no openers: blossoms, new growth, spring
  rain, the stream and birdsong, each the two-part partner of another. A season none of whose concepts names an
  opener draws its concept per case as space does, and the model writes line one, which must name one of the
  concept's nouns (`lab_name_cut()`, tuned by LAB_NAME_MISS). The Free style in spring keeps its openers.
- `browser_styles.cpp`: the browser adapter, exporting `haiku_styles_generate(style, case)`.

The haiku lab (`haiku-lab.html`, `../haiku-lab-src/`, `haiku-lab/`) is separate and unchanged.

## Rebuild and test

Rebuild after every update of `../haiku-src/` (the test fails until then):

```sh
python3 scripts/build-haiku-styles.py
node scripts/test-haiku-styles.mjs
```

Native build, for experiments (with `LAB_STYLE` unset it writes the same poems as `host.cpp`):

```sh
c++ -std=c++17 -O2 -DLLM_INT8_ACT=1 -I scripts/haiku-styles-src -I scripts/haiku-src -I scripts/haiku-src/include \
    scripts/haiku-styles-src/styles_host.cpp -o /tmp/styles
LAB_STYLE=2 /tmp/styles haiku-generator/model.bin grammar 20 space
```
