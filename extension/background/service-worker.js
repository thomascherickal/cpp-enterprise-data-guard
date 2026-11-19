importScripts('../wasm/data_guard.js');

let wasmPromise = null;

async function getWasm() {
  if (!wasmPromise) {
    wasmPromise = createDataGuardModule({
      locateFile: (path) => chrome.runtime.getURL(`wasm/${path}`)
    });
  }
  return await wasmPromise;
}

async function getStoredState() {
  const wasm = await getWasm();
  const stored = await chrome.storage.local.get(['enabled', 'policy', 'stats']);
  const enabled = stored.enabled !== undefined ? Boolean(stored.enabled) : true;
  const policy = stored.policy || JSON.parse(wasm.getDefaultPolicy());
  const stats = stored.stats || { scans: 0, blocked: 0, secrets: 0, pii: 0 };
  return { wasm, enabled, policy, stats };
}

async function updateBadge(enabled, blockedCount) {
  if (!enabled) {
    await chrome.action.setBadgeText({ text: 'OFF' });
    await chrome.action.setBadgeBackgroundColor({ color: '#4a5568' });
    return;
  }
  await chrome.action.setBadgeText({ text: blockedCount > 0 ? String(blockedCount) : '' });
  await chrome.action.setBadgeBackgroundColor({ color: '#004e8c' });
}

chrome.runtime.onInstalled.addListener(async () => {
  const { enabled, policy, stats } = await getStoredState();
  await chrome.storage.local.set({ enabled, policy, stats });
  await updateBadge(enabled, stats.blocked);
});

chrome.runtime.onMessage.addListener((message, _sender, sendResponse) => {
  (async () => {
    try {
      const { wasm, enabled, policy, stats } = await getStoredState();

      if (message.type === 'GET_STATE') {
        sendResponse({
          ok: true,
          enabled,
          policy,
          stats,
          version: wasm.getVersion()
        });
        return;
      }

      if (message.type === 'SET_ENABLED') {
        const nextEnabled = Boolean(message.enabled);
        await chrome.storage.local.set({ enabled: nextEnabled });
        await updateBadge(nextEnabled, stats.blocked);
        sendResponse({ ok: true, enabled: nextEnabled });
        return;
      }

      if (message.type === 'SET_POLICY') {
        const validation = JSON.parse(wasm.validatePolicy(JSON.stringify(message.policy)));
        if (!validation.valid) {
          sendResponse({ ok: false, errors: validation.errors });
          return;
        }
        await chrome.storage.local.set({ policy: message.policy });
        sendResponse({ ok: true, policy: message.policy });
        return;
      }

      if (message.type === 'RESET_POLICY') {
        const defaultPolicy = JSON.parse(wasm.getDefaultPolicy());
        await chrome.storage.local.set({ policy: defaultPolicy });
        sendResponse({ ok: true, policy: defaultPolicy });
        return;
      }

      if (message.type === 'SCAN_TEXT') {
        if (!enabled) {
          sendResponse({
            ok: true,
            enabled: false,
            result: {
              blocked: false,
              riskScore: 0,
              findingCount: 0,
              findings: [],
              redactedText: message.text || ''
            }
          });
          return;
        }

        const result = JSON.parse(
          wasm.scanText(message.text || '', JSON.stringify(policy))
        );

        if (message.recordStats && (message.text || '').trim()) {
          stats.scans += 1;
          if (result.blocked) stats.blocked += 1;
          for (const f of result.findings || []) {
            if (f.category === 'SECRET') stats.secrets += 1;
            else if (f.category === 'PII') stats.pii += 1;
          }
          await chrome.storage.local.set({ stats });
          await updateBadge(enabled, stats.blocked);
        }

        sendResponse({ ok: true, enabled: true, result, stats });
        return;
      }

      sendResponse({ ok: false, error: 'Unknown message type' });
    } catch (err) {
      sendResponse({ ok: false, error: err?.message || String(err) });
    }
  })();
  return true;
});

