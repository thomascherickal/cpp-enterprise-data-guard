(() => {
  function isPromptInput(el) {
    if (!el || el.nodeType !== Node.ELEMENT_NODE) return false;
    if (el.tagName === 'TEXTAREA') return true;
    if (el.tagName === 'INPUT' && (el.type === 'text' || !el.type)) return true;
    return Boolean(el.isContentEditable);
  }

  function readInputText(el) {
    if (el.tagName === 'TEXTAREA' || el.tagName === 'INPUT') return el.value || '';
    return el.innerText || el.textContent || '';
  }

  function writeInputText(el, text) {
    if (el.tagName === 'TEXTAREA' || el.tagName === 'INPUT') el.value = text;
    else el.innerText = text;
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

  function showBlockedModal(target, result, onSendAnyway) {
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

    const sendBtn = document.createElement('button');
    sendBtn.type = 'button';
    sendBtn.className = 'dg-ext-btn';
    sendBtn.textContent = 'Send Anyway';
    sendBtn.addEventListener('click', () => { overlay.remove(); onSendAnyway(); });

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

    actions.append(cancelBtn, sendBtn, redactBtn);
    modal.append(title, desc, list, actions);
    overlay.appendChild(modal);
    document.body.appendChild(overlay);
    redactBtn.focus();
  }

  window.__dgDomUi = { isPromptInput, readInputText, showToast, showBlockedModal };
})();

