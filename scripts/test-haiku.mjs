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
assert.equal(hash(readFileSync(new URL('decoder.mjs', dir))), info.module_sha256);
// Ensure the module executes supplied, verified bytes rather than fetching another copy.
await assert.rejects(createDecoder({wasmBinary: new Uint8Array([0]), printErr: () => {}}));
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
// Exact-line repeat guard: a fresh decoder writing 20 Space poems in a row must match the
// native host, reject at least one recent line, and never reuse a line from recent poems.
const fresh = await createDecoder({wasmBinary: wasm, print: text => {line = text;}});
fresh.FS.writeFile('/model.bin', model);
const recent = []; let guarded = 0;
for (const [caseNumber, expected] of fixture.space_sequence.entries()) {
  line = null;
  assert.equal(fresh._haiku_generate(4, caseNumber), 0);
  const poem = JSON.parse(line);
  assert.deepEqual(poem.lines, expected.lines);
  guarded += poem.line_repeat_attempts;
  for (const text of poem.lines) {
    const key = text.toLowerCase().replace(/[ .,]+$/, '');
    assert.ok(!recent.includes(key), `repeated line: ${text}`);
    recent.push(key);
  }
}
assert.ok(guarded > 0, 'the repeat guard never fired');
console.log(`space sequence: ${fixture.space_sequence.length} poems, ${guarded} repeated lines rejected`);
console.log('Model hashes, invalid inputs, all themes, history, theme switching, and the line repeat guard passed.');
