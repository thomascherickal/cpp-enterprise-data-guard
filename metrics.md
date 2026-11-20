# Project Metrics

Updated: 2026-11-20 22:48:51 UTC

```mermaid
flowchart LR
    A["100% Test Pass"] --> B["100% C++ gcov Coverage"]
    B --> C["<=100 Lines/File"]
    C --> D["CI & Git Hook Gate"]
    style A fill:#004e8c,stroke:#38bdf8,color:#ffffff
    style B fill:#065f46,stroke:#34d399,color:#ffffff
    style C fill:#1e293b,stroke:#94a3b8,color:#ffffff
    style D fill:#0063b1,stroke:#60a5fa,color:#ffffff
```

## Line-Count Report

| Group | Files | Lines |
| :--- | ---: | ---: |
| C++ Source (.cpp/.hpp) | 37 | 2059 |
| JavaScript / WASM Glue (.js/.mjs) | 12 | 680 |
| CSS & HTML (.css/.html/.svg) | 12 | 659 |
| Scripts & Build (.sh/.py/CMake/Docker/Conf) | 17 | 825 |
| JSON Data & Manifests (.json) | 3 | 91 |
| Documentation (.md) | 19 | 1167 |
| C++ Unit Tests | 10 | 552 |
| WASM / Perf / Security / E2E / Deploy Tests | 5 | 152 |
| **Total Project** | **115** | **6185** |
