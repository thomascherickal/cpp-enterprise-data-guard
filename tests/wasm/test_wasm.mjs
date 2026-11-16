import assert from 'node:assert/strict';
import { createRequire } from 'node:module';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const require = createRequire(import.meta.url);
const createDataGuardModule = require(path.resolve(__dirname, '../../dist/wasm/data_guard.js'));

const wasm = await createDataGuardModule();

assert.equal(wasm.getVersion(), '1.0.0');

const defaultPolicy = wasm.getDefaultPolicy();
const valOk = JSON.parse(wasm.validatePolicy(defaultPolicy));
assert.equal(valOk.valid, true);
assert.equal(valOk.errors.length, 0);

const valBadJson = JSON.parse(wasm.validatePolicy('{broken'));
assert.equal(valBadJson.valid, false);

const valBadPolicy = JSON.parse(wasm.validatePolicy('{"minSeverity":"BOGUS"}'));
assert.equal(valBadPolicy.valid, false);

const secretScan = JSON.parse(
  wasm.scanText('const k = "AKIA1234567890ABCDEF";', defaultPolicy)
);
assert.equal(secretScan.blocked, true);
assert.equal(secretScan.findingCount, 1);
assert.equal(secretScan.findings[0].ruleId, 'SEC-AWS-KEY');
assert.ok(!secretScan.redactedText.includes('AKIA1234567890ABCDEF'));

const safeScan = JSON.parse(
  wasm.scanText('Explain C++ RAII in two sentences.', defaultPolicy)
);
assert.equal(safeScan.blocked, false);
assert.equal(safeScan.findingCount, 0);

const errScan = JSON.parse(wasm.scanText('hello', '{bad json'));
assert.equal(errScan.blocked, true);
assert.ok(errScan.error);

const redacted = wasm.redactText('SSN 123-45-6789', defaultPolicy);
assert.equal(redacted, 'SSN [REDACTED:PII-SSN]');

console.log('All WASM integration tests passed!');

