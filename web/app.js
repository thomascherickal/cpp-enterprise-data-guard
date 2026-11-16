const SAMPLES = {
  safe: `Explain how WebAssembly linear memory works in C++ Emscripten builds.\nKeep the summary under 5 bullet points.`,
  secrets: `// Accidentally pasted production config!\nconst AWS_ACCESS_KEY_ID = "AKIA9876543210ZYXWVU";\nconst DATABASE_URL = "postgres://admin:SuperSecretPass99@db.prod.internal:5432/customers";\nconst OPENAI_KEY = "sk-proj-abcdefghijklmnopqrstuvwxyz123456";`,
  pii: `Please draft a polite follow-up email for this customer:\nName: Jordan Vance\nEmail: jordan.vance@acme-corp.com\nSSN: 123-45-6789\nCard: 4532-0151-1283-0366`
};

const STORAGE_KEY = 'dataguard_local_stats_v1';

const state = {
  wasm: null,
  policy: null,
  stats: loadStats(),
  debounceTimer: null
};

function loadStats() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (raw) return JSON.parse(raw);
  } catch {
    // ignore storage errors
  }
  return { scans: 0, secrets: 0, pii: 0 };
}

function saveStats() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(state.stats));
  } catch {
    // ignore storage errors
  }
  document.getElementById('statScans').textContent = String(state.stats.scans);
  document.getElementById('statSecrets').textContent = String(state.stats.secrets);
  document.getElementById('statPii').textContent = String(state.stats.pii);
}

async function loadInitialPolicy() {
  for (const url of ['./policy.json', './policy.default.json']) {
    try {
      const resp = await fetch(url);
      if (resp.ok) {
        return await resp.json();
      }
    } catch {
      // try next fallback
    }
  }
  return JSON.parse(state.wasm.getDefaultPolicy());
}

function syncPolicyControls() {
  if (!state.policy) return;
  document.getElementById('policyOrgValue').textContent =
    state.policy.organization || 'Enterprise Default';
  document.getElementById('polSecrets').checked = Boolean(state.policy.enableSecrets);
  document.getElementById('polPii').checked = Boolean(state.policy.enablePii);
  document.getElementById('polEntropy').checked = Boolean(state.policy.enableEntropy);
  document.getElementById('polIp').checked = Boolean(state.policy.enableIpAddresses);
  document.getElementById('polRedactionMode').value =
    state.policy.redactionMode || 'PLACEHOLDER';
}

function readPolicyControls() {
  state.policy.enableSecrets = document.getElementById('polSecrets').checked;
  state.policy.enablePii = document.getElementById('polPii').checked;
  state.policy.enableEntropy = document.getElementById('polEntropy').checked;
  state.policy.enableIpAddresses = document.getElementById('polIp').checked;
  state.policy.redactionMode = document.getElementById('polRedactionMode').value;
}

function renderFindingsTable(findings) {
  const tbody = document.getElementById('findingsBody');
  tbody.replaceChildren();

  if (!findings || findings.length === 0) {
    const tr = document.createElement('tr');
    const td = document.createElement('td');
    td.colSpan = 6;
    td.className = 'empty-cell';
    td.textContent = 'No secrets or personal data detected.';
    tr.appendChild(td);
    tbody.appendChild(tr);
    return;
  }

  for (const f of findings) {
    const tr = document.createElement('tr');

    const sevTd = document.createElement('td');
    const badge = document.createElement('span');
    badge.className =
      f.severity === 'CRITICAL' || f.severity === 'HIGH'
        ? 'badge badge-danger'
        : f.severity === 'MEDIUM'
          ? 'badge badge-warn'
          : 'badge badge-neutral';
    badge.textContent = f.severity;
    sevTd.appendChild(badge);

    const ruleTd = document.createElement('td');
    const code = document.createElement('code');
    code.textContent = f.ruleId;
    ruleTd.appendChild(code);

    const catTd = document.createElement('td');
    catTd.textContent = f.category;

    const locTd = document.createElement('td');
    locTd.textContent = `L${f.line}:C${f.column}`;

    const maskTd = document.createElement('td');
    const maskCode = document.createElement('code');
    maskCode.textContent = f.maskedMatch;
    maskTd.appendChild(maskCode);

    const descTd = document.createElement('td');
    descTd.textContent = f.description;

    tr.append(sevTd, ruleTd, catTd, locTd, maskTd, descTd);
    tbody.appendChild(tr);
  }
}

function runScan(countInStats = true) {
  if (!state.wasm || !state.policy) return;
  const input = document.getElementById('promptInput').value;
  const result = JSON.parse(state.wasm.scanText(input, JSON.stringify(state.policy)));

  document.getElementById('riskScoreText').textContent = `${result.riskScore} / 100`;
  document.getElementById('riskProgress').value = result.riskScore;
  document.getElementById('redactedOutput').value = result.redactedText || '';
  document.getElementById('copyRedactedBtn').disabled = !input.length;

  const verdict = document.getElementById('verdictBadge');
  if (!input.trim()) {
    verdict.textContent = 'READY';
    verdict.className = 'badge badge-safe';
    document.getElementById('findingSummary').textContent = 'No findings yet.';
  } else if (result.blocked) {
    verdict.textContent = 'BLOCKED BY POLICY';
    verdict.className = 'badge badge-danger';
    document.getElementById('findingSummary').textContent =
      `${result.findingCount} sensitive item(s) detected. Use redacted output before sending.`;
  } else if (result.findingCount > 0) {
    verdict.textContent = 'WARNING';
    verdict.className = 'badge badge-warn';
    document.getElementById('findingSummary').textContent =
      `${result.findingCount} low-severity item(s) detected.`;
  } else {
    verdict.textContent = 'SAFE TO SEND';
    verdict.className = 'badge badge-safe';
    document.getElementById('findingSummary').textContent =
      'Zero secrets or personal data detected.';
  }

  renderFindingsTable(result.findings);

  if (countInStats && input.trim()) {
    state.stats.scans += 1;
    for (const f of result.findings) {
      if (f.category === 'SECRET') state.stats.secrets += 1;
      else if (f.category === 'PII') state.stats.pii += 1;
    }
    saveStats();
  }
}

async function init() {
  saveStats();
  const statusBadge = document.getElementById('wasmStatusBadge');

  try {
    state.wasm = await createDataGuardModule();
    state.policy = await loadInitialPolicy();
    syncPolicyControls();
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
    const out = document.getElementById('redactedOutput').value;
    await navigator.clipboard.writeText(out);
    const btn = document.getElementById('copyRedactedBtn');
    const prev = btn.textContent;
    btn.textContent = 'Copied!';
    setTimeout(() => {
      btn.textContent = prev;
    }, 1500);
  });

  for (const id of ['polSecrets', 'polPii', 'polEntropy', 'polIp', 'polRedactionMode']) {
    document.getElementById(id).addEventListener('change', () => {
      readPolicyControls();
      runScan(false);
    });
  }

  document.getElementById('importPolicyInput').addEventListener('change', async (e) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const text = await file.text();
    const validation = JSON.parse(state.wasm.validatePolicy(text));
    if (!validation.valid) {
      alert(`Invalid policy.json:\n${validation.errors.join('\n')}`);
      return;
    }
    state.policy = JSON.parse(text);
    syncPolicyControls();
    runScan(false);
  });

  document.getElementById('resetPolicyBtn').addEventListener('click', async () => {
    state.policy = JSON.parse(state.wasm.getDefaultPolicy());
    syncPolicyControls();
    runScan(false);
  });

  const dialog = document.getElementById('interceptorDialog');
  document.getElementById('openInterceptorBtn').addEventListener('click', async () => {
    try {
      const resp = await fetch('./interceptor.js');
      document.getElementById('interceptorSnippet').textContent = await resp.text();
    } catch {
      document.getElementById('interceptorSnippet').textContent = '// See web/interceptor.js';
    }
    dialog.showModal();
  });
  document.getElementById('closeDialogBtn').addEventListener('click', () => dialog.close());
  document.getElementById('copyInterceptorBtn').addEventListener('click', async () => {
    const code = document.getElementById('interceptorSnippet').textContent;
    await navigator.clipboard.writeText(code);
  });
}

init();

