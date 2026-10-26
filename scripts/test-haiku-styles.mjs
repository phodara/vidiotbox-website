// Run with node scripts/test-haiku-styles.mjs; executes the style decoder's shipped WebAssembly.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {createHash} from 'node:crypto';
import createDecoder from '../haiku-styles/decoder.mjs';
const dir = new URL('../haiku-styles/', import.meta.url);
const info = JSON.parse(readFileSync(new URL('model-info.json', dir)));
const model = readFileSync(new URL(info.model_path, dir));
const wasm = readFileSync(new URL('decoder.wasm', dir));
const hash = value => createHash('sha256').update(value).digest('hex');
assert.equal(hash(model), info.sha256);
assert.equal(hash(wasm), info.wasm_sha256);
assert.equal(hash(readFileSync(new URL('decoder.mjs', dir))), info.module_sha256);
for (const [name, digest] of Object.entries(info.source_sha256))
  assert.equal(hash(readFileSync(new URL(`./${name}`, import.meta.url))), digest, `${name} changed since the build: run python3 scripts/build-haiku-styles.py`);
let line;
const module = await createDecoder({wasmBinary: wasm, print: text => {line = text;}});
module.FS.writeFile('/model.bin', model);
assert.equal(module._haiku_styles_generate(0, 0), 2);
assert.equal(module._haiku_styles_generate(3, 0), 2);
const concepts = new Set();
for (const style of [1, 2]) {
  for (let caseNumber = 0; caseNumber < 10; caseNumber++) {
    line = null;
    assert.equal(module._haiku_styles_generate(style, caseNumber), 0);
    const poem = JSON.parse(line);
    assert.equal(poem.complete, true);
    assert.deepEqual(poem.syllables, [5, 7, 5]);
    assert.equal(poem.style, style);
    assert.ok(poem.concept, 'every styled poem names its concept');
    concepts.add(poem.concept);
  }
}
console.log(`styles: 20 complete poems, ${concepts.size} concepts`);
