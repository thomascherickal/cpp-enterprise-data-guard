import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');

const wasmJs = fs.readFileSync(path.join(root, 'dist/wasm/data_guard.js'), 'utf8');
assert.ok(!wasmJs.includes('new Function'), 'WASM JS must not use new Function');
assert.ok(!wasmJs.includes('eval('), 'WASM JS must not use eval()');

const html = fs.readFileSync(path.join(root, 'web/index.html'), 'utf8');
assert.ok(html.includes('Content-Security-Policy'), 'index.html must define CSP');
assert.ok(!html.includes("'unsafe-eval'"), 'index.html must not allow unsafe-eval');

for (const jsFile of ['web/js/findings-view.js', 'extension/content/dom-ui.js']) {
  const src = fs.readFileSync(path.join(root, jsFile), 'utf8');
  assert.ok(!src.includes('innerHTML'), `${jsFile} must not use innerHTML`);
}

console.log('All security tests passed!');

