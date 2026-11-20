export const SAMPLES = {
  safe: `Explain how WebAssembly linear memory works in C++ Emscripten builds.\nKeep the summary under 5 bullet points.`,
  secrets: `// Accidentally pasted production config!\nconst AWS_ACCESS_KEY_ID = "AKIA9876543210ZYXWVU";\nconst DATABASE_URL = "postgres://admin:SuperSecretPass99@db.prod.internal:5432/customers";\nconst OPENAI_KEY = "sk-proj-abcdefghijklmnopqrstuvwxyz123456";`,
  pii: `Please draft a polite follow-up email for this customer:\nName: Jordan Vance\nEmail: jordan.vance@acme-corp.com\nSSN: 123-45-6789\nCard: 4532-0151-1283-0366`
};

export function wireInterceptorDialog() {
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

