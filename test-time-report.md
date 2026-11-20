# Test Time Report

Updated: 2026-11-20 22:48:51 UTC | Total Runtime: **53s** (Limit: 1800s)

```mermaid
flowchart LR
    U["Unit + 100% gcov"] --> W["WASM Integration"]
    W --> S["Security & Perf"]
    S --> E["E2E & Deploy Smoke"]
    style U fill:#004e8c,stroke:#38bdf8,color:#ffffff
    style W fill:#0063b1,stroke:#60a5fa,color:#ffffff
    style S fill:#1e2430,stroke:#94a3b8,color:#ffffff
    style E fill:#065f46,stroke:#34d399,color:#ffffff
```

| Suite | Duration | Status |
| :--- | ---: | :--- |
| C++ Unit Tests & 100% Coverage | 7s | PASS |
| Native + WASM Build & Integration | 10s | PASS |
| Security Tests | 0s | PASS |
| Performance & Load Tests | 0s | PASS |
| End-to-End (E2E) Tests | 11s | PASS |
| Deployment Smoke Tests | 25s | PASS |
| **Total** | **53s** | **PASS** |
