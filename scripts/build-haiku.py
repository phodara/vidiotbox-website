#!/usr/bin/env python3
"""Rebuild the browser decoder with Emscripten (brew install emscripten)."""
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / 'scripts/haiku-src'
output = ROOT / 'haiku-generator'
metadata = json.loads((output / 'model-info.json').read_text())
assert hashlib.sha256((output / 'model.bin').read_bytes()).hexdigest() == metadata['sha256'], 'Model hash mismatch'
subprocess.run(['em++', str(source / 'browser.cpp'), '-std=c++17', '-O3', '-DLLM_INT8_ACT=1',
                '-I', str(source), '-I', str(source / 'include'), '--no-entry',
                '-sMODULARIZE=1', '-sEXPORT_ES6=1', '-sENVIRONMENT=web,worker,node',
                '-sALLOW_MEMORY_GROWTH=1', '-sINITIAL_MEMORY=67108864', '-sMAXIMUM_MEMORY=268435456',
                '-sSTACK_SIZE=2097152', '-sFILESYSTEM=1', '-sINCOMING_MODULE_JS_API=["wasmBinary","print","printErr"]', '-sEXPORTED_RUNTIME_METHODS=["FS"]',
                '-sEXPORTED_FUNCTIONS=["_haiku_generate"]', '-o', str(output / 'decoder.mjs')], check=True)
metadata['wasm_sha256'] = hashlib.sha256((output / 'decoder.wasm').read_bytes()).hexdigest()
metadata['module_sha256'] = hashlib.sha256((output / 'decoder.mjs').read_bytes()).hexdigest()
(output / 'model-info.json').write_text(json.dumps(metadata, indent=2) + '\n')
print('Built', output / 'decoder.wasm')
