# Chrome & Microsoft Edge Browser Extension Guide

The **Enterprise Data Guard** Manifest V3 browser extension runs `data_guard.wasm` inside the extension service worker and guards AI chat inputs (`chatgpt.com`, `claude.ai`, `gemini.google.com`, `copilot.microsoft.com`, `perplexity.ai`, `poe.com`) before prompts are submitted.

## 1. Build & Package

```bash
./scripts/build-extension.sh
```

This populates `extension/wasm/data_guard.{js,wasm}` and creates `dist/enterprise-data-guard-extension.zip`.

## 2. Load Unpacked in Chrome or Edge

### Google Chrome
1. Open `chrome://extensions`
2. Enable **Developer mode** (top-right toggle)
3. Click **Load unpacked** and select the `extension/` directory

### Microsoft Edge
1. Open `edge://extensions`
2. Enable **Developer mode** (left sidebar toggle)
3. Click **Load unpacked** and select the `extension/` directory

## 3. Features

- **Automatic Prompt Interception**: Intercepts `Enter` submissions on AI chat inputs and shows a blocking dialog with detected secrets/PII and a one-click **Replace with Safe Redacted Prompt** button.
- **Paste Warning Toast**: Alerts immediately when secrets or PII are pasted into an AI prompt box.
- **Extension Popup**: Toggle protection on/off, view scan/block statistics, run quick pre-flight scans, toggle policy categories, or import an admin-exported `policy.json`.
