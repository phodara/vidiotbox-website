#!/usr/bin/env python3
"""Rebuild the generator's style decoder (brew install emscripten).

styles_host.cpp is scripts/haiku-src/host.cpp with scripts/haiku-styles-src/styles.patch applied, so
the one-thing and two-part styles always use the current decoder rules. If a new host.cpp no longer
takes the patch, this stops; update styles.patch. Reads the V8 model in haiku-generator/ and the
headers in scripts/haiku-src/; writes only scripts/haiku-styles-src/styles_host.cpp and haiku-styles/.
Run it after every update of scripts/haiku-src (test-haiku-styles.mjs fails until then).
"""
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / 'scripts/haiku-styles-src'
shared = ROOT / 'scripts/haiku-src'
output = ROOT / 'haiku-styles'
model = json.loads((ROOT / 'haiku-generator/model-info.json').read_text())
assert hashlib.sha256((ROOT / 'haiku-generator/model.bin').read_bytes()).hexdigest() == model['sha256'], 'Model hash mismatch'
subprocess.run(['patch', '--quiet', '--forward', '--reject-file=-', '-o', str(source / 'styles_host.cpp'),
                str(shared / 'host.cpp'), str(source / 'styles.patch')], check=True)
subprocess.run(['em++', str(source / 'browser_styles.cpp'), '-std=c++17', '-O3', '-DLLM_INT8_ACT=1',
                '-I', str(source), '-I', str(shared), '-I', str(shared / 'include'), '--no-entry',
                '-sMODULARIZE=1', '-sEXPORT_ES6=1', '-sENVIRONMENT=web,worker,node',
                '-sALLOW_MEMORY_GROWTH=1', '-sINITIAL_MEMORY=67108864', '-sMAXIMUM_MEMORY=268435456',
                '-sSTACK_SIZE=2097152', '-sFILESYSTEM=1', '-sINCOMING_MODULE_JS_API=["wasmBinary","print","printErr"]', '-sEXPORTED_RUNTIME_METHODS=["FS"]',
                '-sEXPORTED_FUNCTIONS=["_haiku_styles_generate"]', '-o', str(output / 'decoder.mjs')], check=True)
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
inputs = [source / name for name in ('styles.patch', 'styles_host.cpp', 'lab_concepts.h', 'browser_styles.cpp')]
inputs += [shared / 'host.cpp', shared / 'llm.h', *sorted((shared / 'include').glob('*.h')), *sorted((shared / 'generated').glob('*.h'))]
info = {
    'model': model['model'], 'model_path': '../haiku-generator/model.bin',
    'sha256': model['sha256'], 'model_bytes': model['model_bytes'],
    'decoder': 'C5 controlled grammar + one-thing / two-part styles (scripts/haiku-styles-src)',
    'wasm_sha256': sha(output / 'decoder.wasm'), 'module_sha256': sha(output / 'decoder.mjs'),
    'source_sha256': {str(path.relative_to(ROOT / 'scripts')): sha(path) for path in inputs},
}
(output / 'model-info.json').write_text(json.dumps(info, indent=2) + '\n')
print('Built', output / 'decoder.wasm')
