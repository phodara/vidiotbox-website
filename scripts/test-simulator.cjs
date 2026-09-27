// Run with node scripts/test-simulator.cjs [optional pre-refactor HTML].
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

function load(file, mobile) {
  const calls = [];
  const context = new Proxy({}, {
    get: (target, key) => target[key] ?? ((...args) => calls.push([key, ...args])),
    set: (target, key, value) => { target[key] = value; calls.push([key, value]); return true; }
  });
  function element() {
    const classes = new Set();
    return {
      style: { setProperty() {} }, dataset: {}, children: [], events: {}, value: '',
      width: 340, height: 650,
      classList: { contains: key => classes.has(key), toggle(key, on) { on ? classes.add(key) : classes.delete(key); } },
      append(...items) { this.children.push(...items); },
      replaceChildren() { this.children = []; },
      setAttribute() {}, addEventListener(name, handler) { this.events[name] = handler; },
      getBoundingClientRect: () => ({ width: 340, height: 650, left: 0, top: 0 }),
      getContext: () => context
    };
  }
  const elements = new Map();
  const document = {
    body: element(), documentElement: {clientWidth: mobile ? 390 : 1200},
    getElementById(id) {
      if (!elements.has(id)) elements.set(id, element());
      return elements.get(id);
    },
    querySelectorAll: () => [], createElement: element, createTextNode: text => text,
    addEventListener() {}
  };
  document.getElementById('gifBackgroundColor').value = '#ffffff';
  const sandbox = vm.createContext({ document, console, URLSearchParams,
    performance: { now: () => 1000 }, requestAnimationFrame() {},
    navigator: {userAgent: mobile ? 'iPhone' : 'Desktop'},
    window: { location: {search: ''}, innerWidth: mobile ? 390 : 1200,
      matchMedia: () => ({matches:false}), addEventListener() {}, setTimeout() {} }
  });
  const html = fs.readFileSync(file, 'utf8');
  const script = html.match(/<script>([\s\S]*?)<\/script>/)[1];
  vm.runInContext(script, sandbox);
  return { run: code => vm.runInContext(code, sandbox), calls };
}
const current = path.join(__dirname, '..', 'facimator.html');
for (const mobile of [false, true]) {
  const app = load(current, mobile);
  app.run(`
    for (const mode of Object.keys(mobileFeatureDefs)) {
      activeMoodPadMode = mode;
      syncMoodPadModeButtons();
      if (mobileFeatureInputs.length !== mobileFeatureDefs[mode].length) throw Error('Missing sliders: ' + mode);
      for (const input of mobileFeatureInputs) {
        const key = input.dataset.featureKey;
        input.value = String((Number(input.min) + Number(input.max)) / 2);
        input.events.input();
        if (!Number.isFinite(currentExpression[key])) throw Error('Invalid value: ' + key);
      }
    }
    activeMoodPadMode = 'mood'; syncMoodPadModeButtons();
    if (mobileFeatureInputs.length) throw Error('Mood should use disk');
  `);
  if (mobile) app.run(`
    const before = { ...currentExpression };
    activeMoodPadMode = 'eyes'; syncMoodPadModeButtons();
    const input = mobileFeatureInputs.find(input => input.dataset.featureKey === 'eyelidOpen');
    input.value = '0.05'; input.events.input();
    for (const key of Object.keys(before)) {
      if (key !== 'eyelidOpen' && before[key] !== currentExpression[key]) throw Error('Coupled eye setting: ' + key);
    }
  `);
  if (process.argv[2]) {
    const baseline = load(process.argv[2], mobile);
    for (const preset of ['neutral','happy','angry','sad','loving']) {
      const scenario = `currentExpression = getExpressionByName('${preset}'); render(); render();`;
      app.run(scenario); baseline.run(scenario);
      app.calls.length = baseline.calls.length = 0;
      app.run('render()'); baseline.run('render()');
      assert.equal(JSON.stringify(app.calls), JSON.stringify(baseline.calls), `${mobile ? 'mobile' : 'desktop'} ${preset} drawing changed`);
      assert.equal(app.run('JSON.stringify(currentExpression)'), baseline.run('JSON.stringify(currentExpression)'));
    }
    const definitions = JSON.parse(app.run('JSON.stringify(sliderDefs)'));
    for (const [key, min, max] of definitions) {
      for (const value of [min, max]) {
        const scenario = `currentExpression = getExpressionByName('neutral'); currentExpression[${JSON.stringify(key)}] = ${value}; normalizeDynamicExpressionConstraints(currentExpression); render(); render();`;
        app.run(scenario); baseline.run(scenario);
        app.calls.length = baseline.calls.length = 0;
        app.run('render()'); baseline.run('render()');
        assert.equal(JSON.stringify(app.calls), JSON.stringify(baseline.calls), `${mobile ? 'mobile' : 'desktop'} ${key}=${value} drawing changed`);
      }
    }
    // Check animation interpolation and GIF encoding against the original implementation.
    const samples = `JSON.stringify([0,0.25,0.5,1].map(t => mixExpressions(getExpressionByName('neutral'),getExpressionByName('happy'),t)))`;
    assert.equal(app.run(samples), baseline.run(samples));
    const gif = `JSON.stringify(Array.from(lzwEncode([0,1,1,0,1,0,0,1], 2)))`;
    assert.equal(app.run(gif), baseline.run(gif));
  }
}
console.log('Desktop/mobile startup, feature sliders, eye independence, and optional baseline comparisons passed.');
