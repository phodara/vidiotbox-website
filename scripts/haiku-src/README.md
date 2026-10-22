# Haiku browser decoder source

This snapshot comes from the sibling `haiku-training` project:

- `research/decoder/host.cpp` → `host.cpp`
- `runtime/llm.h` → `llm.h`
- `c5/include/haiku*.h` → `include/`
- simulator-generated vocabulary, haiku and grammar headers → `generated/`
- `artifacts/haiku-cosmic-v8/model.bin` → `../../haiku-generator/model.bin` (V8, 21 Oct 2026; V7 was `haiku-cosmic-v7`)

`browser.cpp` is the browser adapter. It invokes the unchanged host decoder,
retains recent-word history per theme, resets theme-specific sampling defaults,
and frees each generation's scratch memory. The model and grammar are not
reimplemented in JavaScript. Source checksums are in `source-hashes.json`;
model identity and binary hashes are in `haiku-generator/model-info.json`.

## Rebuild

Install Emscripten (`brew install emscripten` on macOS), then from this repository:

```sh
python3 scripts/build-haiku.py
node scripts/test-haiku.mjs
python3 scripts/test-build-site.py
```

Commit both `haiku-generator/decoder.mjs` and `decoder.wasm` after rebuilding.
The Pages workflow serves the checked-in binaries, so deployment needs no C++
compiler. `model.bin` is a normal Git file (about 15 MB), not a Git LFS pointer.
The model is hash-checked before build and in the browser before inference.

`parity.json` contains two consecutive native INT8-activation host results for
each theme; tests compare the shipped WASM poems to those native results with
interleaved themes. If updating source/model, regenerate these fixtures using
the matching native host binary, compiled with `-ffp-contract=off` (Apple Silicon
fuses multiply-adds and WebAssembly does not, which can flip a near-tie between two words),
and review changes before accepting them.

## Local preview

```sh
python3 -m http.server 8080 --bind 127.0.0.1
```

Open http://127.0.0.1:8080/haiku-generator/ (file URLs cannot run this worker).
On the public site HTTPS is required for model hash verification.

Model download and inference run in a module worker. The page stays responsive
while generating. Each page visit starts fresh history. Auto mode waits 40
seconds before the next request; it does not emulate device prefetch timing.
The default export matches the C5 `haiku_grammar` deployment; device hardware
speed and its random entropy are not emulated.

The copied runtime and decoder retain the MIT notice shipped as
`haiku-generator/LICENSE.txt`; the website's existing license is unchanged.
