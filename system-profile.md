# System Profile (CPU & GPU)

Updated: 2026-11-20 22:48:51 UTC

```mermaid
flowchart LR
    Build["CMake + Emscripten Build"] --> CPU["CPU Profiling"]
    Test["Unit / WASM / E2E Tests"] --> CPU
    CPU --> GPU["GPU Telemetry"]
    style Build fill:#004e8c,stroke:#38bdf8,color:#ffffff
    style Test fill:#0063b1,stroke:#60a5fa,color:#ffffff
    style CPU fill:#1e2430,stroke:#94a3b8,color:#ffffff
    style GPU fill:#065f46,stroke:#34d399,color:#ffffff
```

| Resource | Snapshot |
| :--- | :--- |
| CPU Model | 11th Gen Intel(R) Core(TM) i5-1135G7 @ 2.40GHz |
| Logical Cores | 8 |
| Memory Used / Total | 9.0Gi / 15Gi |
| GPU Telemetry | NVIDIA GeForce MX330, 0 %, 5 MiB, 2048 MiB |
