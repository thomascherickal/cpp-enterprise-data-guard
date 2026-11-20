(() => {
  if (window.__dgExtensionActive) return;
  window.__dgExtensionActive = true;

  const { isPromptInput, readInputText, showToast, showBlockedModal } = window.__dgDomUi;
  let bypassTarget = null;
  let scanningInFlight = false;

  function triggerEnter(target) {
    bypassTarget = target;
    target.dispatchEvent(
      new KeyboardEvent('keydown', { key: 'Enter', code: 'Enter', bubbles: true, cancelable: true })
    );
  }

  document.addEventListener(
    'keydown',
    async (event) => {
      if (event.key !== 'Enter' || event.shiftKey || event.isComposing) return;
      const target = event.target;
      if (!isPromptInput(target)) return;
      if (bypassTarget === target) { bypassTarget = null; return; }

      const text = readInputText(target);
      if (!text.trim() || scanningInFlight) return;

      event.preventDefault();
      event.stopImmediatePropagation();
      scanningInFlight = true;

      try {
        const resp = await chrome.runtime.sendMessage({ type: 'SCAN_TEXT', text, recordStats: true });
        if (!resp?.ok || !resp.enabled || !resp.result?.blocked) {
          triggerEnter(target);
          return;
        }
        showBlockedModal(target, resp.result, () => triggerEnter(target));
      } catch {
        triggerEnter(target);
      } finally {
        scanningInFlight = false;
      }
    },
    true
  );

  document.addEventListener(
    'paste',
    (event) => {
      const target = event.target;
      if (!isPromptInput(target)) return;
      const pasted = event.clipboardData?.getData('text') || '';
      if (!pasted.trim()) return;
      setTimeout(async () => {
        try {
          const resp = await chrome.runtime.sendMessage({
            type: 'SCAN_TEXT',
            text: pasted,
            recordStats: false
          });
          if (resp?.ok && resp.enabled && resp.result?.blocked) {
            showToast(`Data Guard Warning: ${resp.result.findingCount} secret/PII item(s) pasted.`);
          }
        } catch {
          // ignore runtime errors
        }
      }, 0);
    },
    true
  );
})();
