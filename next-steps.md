# Post-Commit & Post-Push Checklist

```mermaid
flowchart LR
    T["Tests & 100% Coverage"] --> L["<=100 Lines & Mermaid"]
    L --> P["CPU/GPU Profile & Metrics"]
    P --> G["Commit & Push"]
    style T fill:#004e8c,stroke:#38bdf8,color:#ffffff
    style L fill:#0063b1,stroke:#60a5fa,color:#ffffff
    style P fill:#1e2430,stroke:#94a3b8,color:#ffffff
    style G fill:#065f46,stroke:#34d399,color:#ffffff
```

1. Run `./scripts/run-tests.sh` to verify unit, integration, smoke, E2E, performance, and security tests pass.
2. Confirm 100% C++ `gcov` line coverage and `< 30m` total runtime in `test-time-report.md`.
3. Confirm `scripts/enforce-rules.py` passes (all source/test files <= 100 lines, colored Mermaid diagrams).
4. Review updated `system-profile.md`, `metrics.md`, and `readme.md`, then commit and push.
