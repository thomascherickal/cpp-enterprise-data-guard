import { renderScanVerdict } from './js/findings-view.js';
import { loadInitialPolicy, readPolicyControls, syncPolicyControls } from './js/policy-ui.js';
import { SAMPLES, wireInterceptorDialog } from './js/samples-dialog.js';
import { loadStats, recordScanStats, saveStats } from './js/stats-store.js';
import { initWasmClient } from './js/wasm-client.js';

const state = { wasm: null, policy: null, stats: loadStats(), debounceTimer: null };

function runScan(countInStats = true) {
  if (!state.wasm || !state.policy) return;
  const input = document.getElementById('promptInput').value;
  const result = state.wasm.scanText(input, state.policy);
  renderScanVerdict(input, result);
  if (countInStats && input.trim()) recordScanStats(state.stats, result.findings);
}

async function init() {
  saveStats(state.stats);
  const statusBadge = document.getElementById('wasmStatusBadge');
  try {
    state.wasm = await initWasmClient();
    state.policy = await loadInitialPolicy(state.wasm);
    syncPolicyControls(state.policy);
    statusBadge.textContent = `WASM v${state.wasm.getVersion()} Active`;
    statusBadge.className = 'badge badge-safe';
  } catch (err) {
    statusBadge.textContent = 'WASM Load Failed';
    statusBadge.className = 'badge badge-danger';
    console.error(err);
    return;
  }

  const promptEl = document.getElementById('promptInput');
  promptEl.addEventListener('input', () => {
    if (!document.getElementById('liveScanToggle').checked) return;
    clearTimeout(state.debounceTimer);
    state.debounceTimer = setTimeout(() => runScan(false), 120);
  });

  document.getElementById('scanNowBtn').addEventListener('click', () => runScan(true));
  document.getElementById('clearBtn').addEventListener('click', () => {
    promptEl.value = '';
    runScan(false);
  });

  document.querySelectorAll('[data-sample]').forEach((btn) => {
    btn.addEventListener('click', () => {
      promptEl.value = SAMPLES[btn.dataset.sample] || '';
      runScan(true);
    });
  });

  document.getElementById('copyRedactedBtn').addEventListener('click', async () => {
    await navigator.clipboard.writeText(document.getElementById('redactedOutput').value);
    const btn = document.getElementById('copyRedactedBtn');
    const prev = btn.textContent;
    btn.textContent = 'Copied!';
    setTimeout(() => { btn.textContent = prev; }, 1500);
  });

  for (const id of ['polSecrets', 'polPii', 'polEntropy', 'polIp', 'polRedactionMode']) {
    document.getElementById(id).addEventListener('change', () => {
      readPolicyControls(state.policy);
      runScan(false);
    });
  }

  document.getElementById('importPolicyInput').addEventListener('change', async (e) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const text = await file.text();
    const validation = state.wasm.validatePolicy(text);
    if (!validation.valid) {
      alert(`Invalid policy.json:\n${validation.errors.join('\n')}`);
      return;
    }
    state.policy = JSON.parse(text);
    syncPolicyControls(state.policy);
    runScan(false);
  });

  document.getElementById('resetPolicyBtn').addEventListener('click', () => {
    state.policy = state.wasm.getDefaultPolicy();
    syncPolicyControls(state.policy);
    runScan(false);
  });

  wireInterceptorDialog();
}

init();
