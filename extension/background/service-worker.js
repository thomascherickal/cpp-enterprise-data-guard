importScripts('../wasm/data_guard.js', 'handlers.js');

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
      const ctx = await getStoredState();
      sendResponse(await handleMessage(message, ctx, updateBadge));
    } catch (err) {
      sendResponse({ ok: false, error: err?.message || String(err) });
    }
  })();
  return true;
});
