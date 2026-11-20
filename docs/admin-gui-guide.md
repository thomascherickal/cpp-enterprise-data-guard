# Admin GUI Control Centre Guide

The **Admin Control Centre** (`dataguard-admin`) is a native C++ GTK+ 3 desktop application for security administrators.

## Launching the GUI

```bashbash 
./build/native/dataguard-admin
```

## Tabs

1. **Policy Configuration**: Toggle Secrets, PII, High-Entropy Bearer detection, and IPv4 scanning; set minimum severity (`LOW`..`CRITICAL`), entropy threshold (`2.0`..`6.0`), redaction mode (`PLACEHOLDER`, `MASK`, `REMOVE`), and blocking thresholds.
2. **Allowlist & Custom Rules**: Add safe test tokens (e.g., `AKIAIOSFODNN7EXAMPLE`) to the allowlist, or define custom organization regex rules with severity levels.
3. **Live Scanner Test Bench**: Paste sample prompts or code and run the exact C++ engine to verify policy behavior before exporting.

## Exporting & Headless CLI

Click **Export Policy JSON...** in the GUI, or use the headless CLI for CI/CD automation:

```bash
./build/native/dataguard-admin --headless --export-policy policy.json
./build/native/dataguard-admin --headless --policy policy.json --scan "text to check"
```

## Visual Architecture

```mermaid
flowchart LR
    Tab1["Policy Tab"] --> Ctrl["AdminController"]
    Tab2["Rules Tab"] --> Ctrl
    Tab3["Test Bench"] --> Ctrl
    Ctrl --> Out["policy.json"]
    style Tab1 fill:#004e8c,stroke:#38bdf8,color:#ffffff
    style Tab2 fill:#0063b1,stroke:#60a5fa,color:#ffffff
    style Tab3 fill:#1e2430,stroke:#94a3b8,color:#ffffff
    style Ctrl fill:#004e8c,stroke:#38bdf8,color:#ffffff
    style Out fill:#065f46,stroke:#34d399,color:#ffffff
```
