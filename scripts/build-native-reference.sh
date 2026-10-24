#!/usr/bin/env bash
# Build the native host decoder that parity.json is generated from, with the same float
# math as the WebAssembly build: no fused multiply-adds, and Emscripten's musl expf, logf,
# powf, sinf, cosf and erff linked in place of the macOS ones (which differ by about one
# unit in the last place and eventually flip a near-tie between two words).
#   scripts/build-native-reference.sh OUT
# OUT is the decoder binary to write; it reads the sources in scripts/haiku-src.
set -euo pipefail
OUT=${1:?usage: build-native-reference.sh OUT}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SRC=$ROOT/scripts/haiku-src
EMCC=$(command -v emcc) || { echo "emcc not found (brew install emscripten)" >&2; exit 1; }
MUSL=$(cd "$(dirname "$(readlink -f "$EMCC")")/.." && pwd)/libexec/system/lib/libc/musl
[ -d "$MUSL/src/math" ] || MUSL=$(cd "$(dirname "$(readlink -f "$EMCC")")" && pwd)/system/lib/libc/musl
[ -d "$MUSL/src/math" ] || { echo "musl sources not found next to $EMCC" >&2; exit 1; }
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
for f in expf logf powf sinf cosf erff exp2f_data logf_data powf_data __sindf __cosdf __rem_pio2f \
         __rem_pio2_large __math_oflowf __math_uflowf __math_xflowf __math_invalidf __math_divzerof; do
  clang -c -O2 -ffp-contract=off -w -nostdinc -I "$MUSL/src/include" -I "$MUSL/src/internal" \
    -I "$MUSL/arch/emscripten" -I "$MUSL/arch/generic" -I "$MUSL/include" \
    -isystem "$(clang -print-resource-dir)/include" "$MUSL/src/math/$f.c" -o "$WORK/$f.o"
done
# -fno-builtin keeps clang from fusing sinf and cosf into Apple's __sincosf_stret.
c++ -std=c++17 -O3 -ffp-contract=off -fno-builtin -DLLM_INT8_ACT=1 -I "$SRC" -I "$SRC/include" \
  "$SRC/host.cpp" "$WORK"/*.o -o "$OUT"
if nm -u "$OUT" | grep -qE '_(expf|logf|powf|sinf|cosf|erff|__sincosf_stret)$'; then
  echo "a float math function still comes from the system library" >&2; exit 1
fi
"$OUT" --self-test >/dev/null
echo "Built $OUT"
