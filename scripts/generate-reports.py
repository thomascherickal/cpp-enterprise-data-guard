#!/usr/bin/env python3
"""Generate metrics.md, system-profile.md, and next-steps.md."""

import subprocess
import time

from report_metrics import generate_metrics


def run_cmd(cmd):
    try:
        return subprocess.check_output(cmd, stderr=subprocess.DEVNULL, text=True).strip()
    except Exception:
        return "Unavailable"


def generate_system_profile():
    cpu_model = run_cmd(["bash", "-c", "lscpu | grep -m1 'Model name' | cut -d: -f2 | xargs"])
    cpu_cores = run_cmd(["nproc"])
    mem_info = run_cmd(["bash", "-c", "free -h | awk '/Mem:/ {print $3 \" / \" $2}'"])
    gpu_info = run_cmd(["nvidia-smi", "--query-gpu=name,utilization.gpu,memory.used,memory.total", "--format=csv,noheader"])

    content = f"""# System Profile (CPU & GPU)

Updated: {time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime())}

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
| CPU Model | {cpu_model} |
| Logical Cores | {cpu_cores} |
| Memory Used / Total | {mem_info} |
| GPU Telemetry | {gpu_info} |
"""
    with open("system-profile.md", "w", encoding="utf-8") as f:
        f.write(content)


def generate_next_steps():
    content = """# Post-Commit & Post-Push Checklist

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
"""
    with open("next-steps.md", "w", encoding="utf-8") as f:
        f.write(content)


if __name__ == "__main__":
    generate_metrics()
    generate_system_profile()
    generate_next_steps()
    print("Generated metrics.md, system-profile.md, next-steps.md")
