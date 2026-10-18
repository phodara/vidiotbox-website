// Run with node scripts/test-haiku.mjs; executes the actual shipped WebAssembly.
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {createHash} from 'node:crypto';
import createDecoder from '../haiku-generator/decoder.mjs';
const dir = new URL('../haiku-generator/', import.meta.url);
const info = JSON.parse(readFileSync(new URL('model-info.json', dir)));
const model = readFileSync(new URL('model.bin', dir));
const wasm = readFileSync(new URL('decoder.wasm', dir));
const hash = value => createHash('sha256').update(value).digest('hex');
assert.equal(hash(model), info.sha256);
assert.equal(hash(wasm), info.wasm_sha256);
let line;
const module = await createDecoder({wasmBinary: wasm, print: text => {line = text;}});
module.FS.writeFile('/model.bin', model);
assert.equal(module._haiku_generate(-1, 0), 2);
assert.equal(module._haiku_generate(0, -1), 2);
const fixture = JSON.parse(readFileSync(new URL('./haiku-src/parity.json', import.meta.url)));
// Interleave themes, including Space -> Spring, to catch leaked sampling defaults.
for (const caseNumber of [0,1]) {
  for (const theme of [4,0,1,2,3]) {
    line = null;
    assert.equal(module._haiku_generate(theme, caseNumber), 0);
    const poem = JSON.parse(line);
    assert.equal(poem.complete, true);
    assert.deepEqual(poem.syllables, [5,7,5]);
    assert.deepEqual(poem.lines, fixture[info.themes[theme]][caseNumber].lines);
    console.log(`${info.themes[theme]} case ${caseNumber}: native / WASM poem parity`);
  }
}
console.log('Model hashes, invalid inputs, all themes, history, and theme switching passed.');
