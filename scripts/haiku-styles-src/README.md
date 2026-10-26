# Haiku style decoder source

The Style menu on `generated-haikus.html` offers three kinds of poem:

- **Standard**: the unchanged main decoder, `haiku-generator/worker.js` (built from `../haiku-src/`),
  the same poems as the boards.
- **One-thing** and **Two-part** (Space only): this decoder, `haiku-styles/worker.js`. One-thing keeps
  every line to one concept's space words; two-part writes line one from them and lines 2-3 from the
  concept's earthly image, across the cut. These are the haiku lab's styles, on the current rules.

Files:

- `styles.patch`: the lab's changes to `host.cpp` (concepts, the line bonus and miss penalty, the
  two-part switch at line two). `scripts/build-haiku-styles.py` applies it to `../haiku-src/host.cpp`
  and writes `styles_host.cpp` (generated; do not edit). New decoder rules therefore reach all three
  styles with a rebuild. If a new `host.cpp` no longer takes the patch, the build stops; update the patch.
- `lab_concepts.h`: the ten concepts, copied from `../haiku-lab-src/` so the lab and this page do not
  affect each other.
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
