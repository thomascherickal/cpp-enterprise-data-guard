let currentPolicy = null;
let lastRedacted = '';

function renderStats(stats) {
  document.getElementById('statScans').textContent = String(stats.scans || 0);
  document.getElementById('statBlocked').textContent = String(stats.blocked || 0);
  document.getElementById('statSecrets').textContent = String(stats.secrets || 0);
  document.getElementById('statPii').textContent = String(stats.pii || 0);
}

function renderPolicy(policy) {
  currentPolicy = policy;
  document.getElementById('polSecrets').checked = Boolean(policy.enableSecrets);
  document.getElementById('polPii').checked = Boolean(policy.enablePii);
  document.getElementById('polEntropy').checked = Boolean(policy.enableEntropy);
  document.getElementById('polIp').checked = Boolean(policy.enableIpAddresses);
}

async function loadState() {
  const resp = await chrome.runtime.sendMessage({ type: 'GET_STATE' });
  if (!resp?.ok) {
    document.getElementById('versionLabel').textContent = resp?.error || 'WASM Error';
    return;
  }
  document.getElementById('versionLabel').textContent = `WASM v${resp.version} • Zero-Trust`;
  document.getElementById('enabledToggle').checked = resp.enabled;
  renderStats(resp.stats);
  renderPolicy(resp.policy);
}

async function savePolicy() {
  if (!currentPolicy) return;
  currentPolicy.enableSecrets = document.getElementById('polSecrets').checked;
  currentPolicy.enablePii = document.getElementById('polPii').checked;
  currentPolicy.enableEntropy = document.getElementById('polEntropy').checked;
  currentPolicy.enableIpAddresses = document.getElementById('polIp').checked;
  await chrome.runtime.sendMessage({ type: 'SET_POLICY', policy: currentPolicy });
}

document.getElementById('enabledToggle').addEventListener('change', async (e) => {
  await chrome.runtime.sendMessage({ type: 'SET_ENABLED', enabled: e.target.checked });
});

for (const id of ['polSecrets', 'polPii', 'polEntropy', 'polIp']) {
  document.getElementById(id).addEventListener('change', savePolicy);
}

document.getElementById('scanBtn').addEventListener('click', async () => {
  const text = document.getElementById('quickInput').value;
  const resp = await chrome.runtime.sendMessage({
    type: 'SCAN_TEXT',
    text,
    recordStats: true
  });
  if (!resp?.ok) return;
  const r = resp.result;
  lastRedacted = r.redactedText || '';
  document.getElementById('copyBtn').disabled = !text.length;
  document.getElementById('scanVerdict').textContent = r.blocked
    ? `Blocked: ${r.findingCount} finding(s), Risk ${r.riskScore}/100`
    : `Safe: ${r.findingCount} finding(s), Risk ${r.riskScore}/100`;
  if (resp.stats) renderStats(resp.stats);
});

document.getElementById('copyBtn').addEventListener('click', async () => {
  await navigator.clipboard.writeText(lastRedacted);
  document.getElementById('scanVerdict').textContent = 'Copied safe redacted prompt.';
});

document.getElementById('importPolicy').addEventListener('change', async (e) => {
  const file = e.target.files?.[0];
  if (!file) return;
  try {
    const policy = JSON.parse(await file.text());
    const resp = await chrome.runtime.sendMessage({ type: 'SET_POLICY', policy });
    if (resp?.ok) {
      renderPolicy(resp.policy);
      document.getElementById('scanVerdict').textContent = 'Policy imported.';
    } else {
      document.getElementById('scanVerdict').textContent =
        `Invalid policy: ${(resp?.errors || []).join(', ')}`;
    }
  } catch {
    document.getElementById('scanVerdict').textContent = 'Invalid JSON file.';
  }
});

document.getElementById('resetPolicyBtn').addEventListener('click', async () => {
  const resp = await chrome.runtime.sendMessage({ type: 'RESET_POLICY' });
  if (resp?.ok) {
    renderPolicy(resp.policy);
    document.getElementById('scanVerdict').textContent = 'Policy reset to default.';
  }
});

loadState();

