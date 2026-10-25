// Haiku lab worker: the lab decoder (one-thing and two-part styles) with the V8 model from
// ../haiku-generator/. Runs entirely on the visitor's device. The V8.9 baseline on the lab page
// uses the unchanged ../haiku-generator/worker.js instead.
let runtime, info, output, loading;
const styles = {'one-thing': 1, 'two-part': 2};
const progress = text => postMessage({type: 'progress', text});
const sha256 = async bytes => [...new Uint8Array(await crypto.subtle.digest('SHA-256', bytes))].map(n => n.toString(16).padStart(2, '0')).join('');
async function checkedFetch(url) {
  const response = await fetch(url, url === './model-info.json' ? {cache: 'no-cache'} : {});
  if (!response.ok) throw Error(`Could not download ${url} (${response.status}). Please reload to retry.`);
  return response;
}
async function initialize() {
  progress('Loading the lab generator…');
  info = await (await checkedFetch('./model-info.json')).json();
  // Same URL as the main generator's worker, so the browser cache serves the model once.
  const response = await checkedFetch(`${info.model_path}?v=${info.sha256.slice(0,12)}`);
  const reader = response.body.getReader(), chunks = [];
  let total = 0;
  while (true) {
    const {done, value} = await reader.read();
    if (done) break;
    chunks.push(value); total += value.length;
    progress(`Downloading model · ${(total / 1e6).toFixed(1)} / ${(info.model_bytes / 1e6).toFixed(1)} MB`);
  }
  const bytes = new Uint8Array(total); let offset = 0;
  for (const chunk of chunks) {bytes.set(chunk, offset); offset += chunk.length;}
  progress('Checking the model…');
  if (total !== info.model_bytes || await sha256(bytes) !== info.sha256) throw Error('Model verification failed. Please reload to retry.');
  const wasm = new Uint8Array(await (await checkedFetch(`./decoder.wasm?v=${info.wasm_sha256.slice(0,12)}`)).arrayBuffer());
  if (await sha256(wasm) !== info.wasm_sha256) throw Error('Decoder verification failed. Please reload to retry.');
  const {default: createDecoder} = await import(`./decoder.mjs?v=${info.module_sha256.slice(0,12)}`);
  runtime = await createDecoder({wasmBinary: wasm, print: line => {output = line;}, printErr: line => console.error(line)});
  runtime.FS.writeFile('/model.bin', bytes);
  postMessage({type: 'ready', info});
}
self.onmessage = async ({data}) => {
  if (data.type === 'init') {
    loading ??= initialize();
    try {await loading;} catch (error) {postMessage({type: 'fatal', error: error.message});}
    return;
  }
  if (data.type !== 'generate') return;
  try {
    const style = styles[data.style];
    if (!runtime || !style || !Number.isInteger(data.concept) || data.concept < -1 || data.concept > 9 ||
        !Number.isInteger(data.case) || data.case < 0 || data.case > 1e9) throw Error('Invalid generation request.');
    output = null;
    const started = performance.now();
    const code = runtime._haiku_lab_generate(style, data.concept, data.case);
    if (code || !output) throw Error('The decoder could not finish this poem.');
    const result = JSON.parse(output);
    result.browser_seconds = (performance.now() - started) / 1000;
    postMessage({type: 'result', id: data.id, result});
  } catch (error) {postMessage({type: 'error', id: data.id, error: error.message});}
};
