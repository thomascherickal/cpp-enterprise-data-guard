import assert from 'node:assert/strict';
import { createRequire } from 'node:module';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const require = createRequire(import.meta.url);
const createDataGuardModule = require(path.resolve(__dirname, '../../dist/wasm/data_guard.js'));

const wasm = await createDataGuardModule();
const policy = wasm.getDefaultPolicy();
const sample = 'const k = "AKIA1234567890ABCDEF"; email=alice@corp.com; ssn=123-45-6789\n';
const largePayload = sample.repeat(200);

const t0 = performance.now();
for (let i = 0; i < 100; i++) {
  const res = JSON.parse(wasm.scanText(sample, policy));
  assert.equal(res.blocked, true);
}
const batchMs = performance.now() - t0;

const t1 = performance.now();
const bigRes = JSON.parse(wasm.scanText(largePayload, policy));
const bigMs = performance.now() - t1;

assert.equal(bigRes.blocked, true);
assert.ok(batchMs < 5000, `100 scans took too long: ${batchMs.toFixed(1)}ms`);
assert.ok(bigMs < 5000, `Large payload scan took too long: ${bigMs.toFixed(1)}ms`);

console.log(`Performance tests passed: 100x=${batchMs.toFixed(1)}ms, 14KB=${bigMs.toFixed(1)}ms`);

