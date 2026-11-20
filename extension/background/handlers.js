async function handleMessage(message, { wasm, enabled, policy, stats }, updateBadge) {
  if (message.type === 'GET_STATE') {
    return { ok: true, enabled, policy, stats, version: wasm.getVersion() };
  }
  if (message.type === 'SET_ENABLED') {
    const nextEnabled = Boolean(message.enabled);
    await chrome.storage.local.set({ enabled: nextEnabled });
    await updateBadge(nextEnabled, stats.blocked);
    return { ok: true, enabled: nextEnabled };
  }
  if (message.type === 'SET_POLICY') {
    const validation = JSON.parse(wasm.validatePolicy(JSON.stringify(message.policy)));
    if (!validation.valid) return { ok: false, errors: validation.errors };
    await chrome.storage.local.set({ policy: message.policy });
    return { ok: true, policy: message.policy };
  }
  if (message.type === 'RESET_POLICY') {
    const defaultPolicy = JSON.parse(wasm.getDefaultPolicy());
    await chrome.storage.local.set({ policy: defaultPolicy });
    return { ok: true, policy: defaultPolicy };
  }
  if (message.type === 'SCAN_TEXT') {
    if (!enabled) {
      return {
        ok: true,
        enabled: false,
        result: { blocked: false, riskScore: 0, findingCount: 0, findings: [], redactedText: message.text || '' }
      };
    }
    const result = JSON.parse(wasm.scanText(message.text || '', JSON.stringify(policy)));
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
    return { ok: true, enabled: true, result, stats };
  }
  return { ok: false, error: 'Unknown message type' };
}

