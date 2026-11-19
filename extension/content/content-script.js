(() => {
  if (window.__dgExtensionActive) return;
  window.__dgExtensionActive = true;

  let bypassTarget = null;
  let scanningInFlight = false;

  function isPromptInput(el) {
    if (!el || el.nodeType !== Node.ELEMENT_NODE) return false;
    if (el.tagName === 'TEXTAREA') return true;
    if (el.tagName === 'INPUT' && (el.type === 'text' || !el.type)) return true;
    if (el.isContentEditable) return true;
    return false;
  }

  function readInputText(el) {
    if (el.tagName === 'TEXTAREA' || el.tagName === 'INPUT') {
      return el.value || '';
    }
    return el.innerText || el.textContent || '';
  }

  function writeInputText(el, text) {
    if (el.tagName === 'TEXTAREA' || el.tagName === 'INPUT') {
      el.value = text;
    } else {
      el.innerText = text;
    }
    el.dispatchEvent(new Event('input', { bubbles: true }));
  }

  function showToast(message) {
    const existing = document.getElementById('dg-ext-toast');
    if (existing) existing.remove();
    const toast = document.createElement('div');
    toast.id = 'dg-ext-toast';
    toast.textContent = message;
    document.body.appendChild(toast);
    setTimeout(() => toast.remove(), 4000);
  }

  function showBlockedModal(target, result) {
    const existing = document.getElementById('dg-ext-overlay');
    if (existing) existing.remove();

    const overlay = document.createElement('div');
    overlay.id = 'dg-ext-overlay';

    const modal = document.createElement('div');
    modal.id = 'dg-ext-modal';
    modal.setAttribute('role', 'dialog');
    modal.setAttribute('aria-modal', 'true');

    const title = document.createElement('h2');
    title.textContent = 'Enterprise Data Guard Blocked This Prompt';

    const desc = document.createElement('p');
    desc.textContent = `${result.findingCount} sensitive item(s) detected (Risk Score: ${result.riskScore}/100).`;

    const list = document.createElement('ul');
    list.id = 'dg-ext-findings';
    for (const f of result.findings || []) {
      const li = document.createElement('li');
      li.textContent = `[${f.severity}] ${f.ruleId}: ${f.maskedMatch} (${f.description})`;
      list.appendChild(li);
    }

    const actions = document.createElement('div');
    actions.className = 'dg-ext-actions';

    const cancelBtn = document.createElement('button');
    cancelBtn.type = 'button';
    cancelBtn.className = 'dg-ext-btn';
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', () => overlay.remove());

    const sendAnywayBtn = document.createElement('button');
    sendAnywayBtn.type = 'button';
    sendAnywayBtn.className = 'dg-ext-btn';
    sendAnywayBtn.textContent = 'Send Anyway';
    sendAnywayBtn.addEventListener('click', () => {
      overlay.remove();
      bypassTarget = target;
      target.dispatchEvent(
        new KeyboardEvent('keydown', {
          key: 'Enter',
          code: 'Enter',
          bubbles: true,
          cancelable: true
        })
      );
    });

    const redactBtn = document.createElement('button');
    redactBtn.type = 'button';
    redactBtn.className = 'dg-ext-btn dg-ext-btn-primary';
    redactBtn.textContent = 'Replace with Safe Redacted Prompt';
    redactBtn.addEventListener('click', () => {
      writeInputText(target, result.redactedText || '');
      overlay.remove();
      target.focus();
      showToast('Prompt replaced with safe redacted version.');
    });

    actions.append(cancelBtn, sendAnywayBtn, redactBtn);
    modal.append(title, desc, list, actions);
    overlay.appendChild(modal);
    document.body.appendChild(overlay);
    redactBtn.focus();
  }

  document.addEventListener(
    'keydown',
    async (event) => {
      if (event.key !== 'Enter' || event.shiftKey || event.isComposing) return;
      const target = event.target;
      if (!isPromptInput(target)) return;

      if (bypassTarget === target) {
        bypassTarget = null;
        return;
      }

      const text = readInputText(target);
      if (!text.trim() || scanningInFlight) return;

      event.preventDefault();
      event.stopImmediatePropagation();
      scanningInFlight = true;

      try {
        const resp = await chrome.runtime.sendMessage({
          type: 'SCAN_TEXT',
          text,
          recordStats: true
        });
        if (!resp?.ok || !resp.enabled || !resp.result?.blocked) {
          bypassTarget = target;
          target.dispatchEvent(
            new KeyboardEvent('keydown', {
              key: 'Enter',
              code: 'Enter',
              bubbles: true,
              cancelable: true
            })
          );
          return;
        }
        showBlockedModal(target, resp.result);
      } catch {
        bypassTarget = target;
        target.dispatchEvent(
          new KeyboardEvent('keydown', {
            key: 'Enter',
            code: 'Enter',
            bubbles: true,
            cancelable: true
          })
        );
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
            showToast(
              `Data Guard Warning: ${resp.result.findingCount} secret/PII item(s) pasted.`
            );
          }
        } catch {
          // ignore runtime errors
        }
      }, 0);
    },
    true
  );
})();

