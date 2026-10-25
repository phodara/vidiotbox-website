#!/usr/bin/env python3
"""Rebuild the haiku lab's browser decoder (brew install emscripten).

The lab reuses the V8 model in haiku-generator/ and the headers in scripts/haiku-src/ read-only;
it writes only to haiku-lab/.
"""
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / 'scripts/haiku-lab-src'
shared = ROOT / 'scripts/haiku-src'
output = ROOT / 'haiku-lab'
model = json.loads((ROOT / 'haiku-generator/model-info.json').read_text())
assert hashlib.sha256((ROOT / 'haiku-generator/model.bin').read_bytes()).hexdigest() == model['sha256'], 'Model hash mismatch'
subprocess.run(['em++', str(source / 'browser_lab.cpp'), '-std=c++17', '-O3', '-DLLM_INT8_ACT=1',
                '-I', str(source), '-I', str(shared), '-I', str(shared / 'include'), '--no-entry',
                '-sMODULARIZE=1', '-sEXPORT_ES6=1', '-sENVIRONMENT=web,worker,node',
                '-sALLOW_MEMORY_GROWTH=1', '-sINITIAL_MEMORY=67108864', '-sMAXIMUM_MEMORY=268435456',
                '-sSTACK_SIZE=2097152', '-sFILESYSTEM=1', '-sINCOMING_MODULE_JS_API=["wasmBinary","print","printErr"]', '-sEXPORTED_RUNTIME_METHODS=["FS"]',
                '-sEXPORTED_FUNCTIONS=["_haiku_lab_generate"]', '-o', str(output / 'decoder.mjs')], check=True)
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
info = {
    'model': model['model'], 'model_path': '../haiku-generator/model.bin',
    'sha256': model['sha256'], 'model_bytes': model['model_bytes'],
    'decoder': 'C5 controlled grammar + lab concepts (scripts/haiku-lab-src)',
    'wasm_sha256': sha(output / 'decoder.wasm'), 'module_sha256': sha(output / 'decoder.mjs'),
    'source_sha256': {name: sha(source / name) for name in ('lab_host.cpp', 'lab_concepts.h', 'browser_lab.cpp')},
}
(output / 'model-info.json').write_text(json.dumps(info, indent=2) + '\n')
print('Built', output / 'decoder.wasm')
