#!/usr/bin/env python3
"""Generate metrics.md with tabular line-count report and colored Mermaid diagrams."""

import os
import time

EXCLUDE_DIRS = {".git", "build", "dist", "node_modules"}

GROUPS = {
    "C++ Source (.cpp/.hpp)": lambda p, e: p.startswith("./src/") and e in {".cpp", ".hpp"},
    "JavaScript / WASM Glue (.js/.mjs)": lambda p, e: not p.startswith("./tests/") and e in {".js", ".mjs"},
    "CSS & HTML (.css/.html/.svg)": lambda p, e: e in {".css", ".html", ".svg"},
    "Scripts & Build (.sh/.py/CMake/Docker/Conf)": lambda p, e: not p.startswith("./tests/") and (
        e in {".sh", ".py", ".yml", ".conf"} or os.path.basename(p) in {"CMakeLists.txt", "Dockerfile"}
    ),
    "JSON Data & Manifests (.json)": lambda p, e: e == ".json",
    "Documentation (.md)": lambda p, e: e == ".md",
    "C++ Unit Tests": lambda p, e: p.startswith("./tests/cpp/"),
    "WASM / Perf / Security / E2E / Deploy Tests": lambda p, e: p.startswith("./tests/") and not p.startswith("./tests/cpp/"),
}


def generate_metrics():
    counts = {k: [0, 0] for k in GROUPS}
    total_files, total_lines = 0, 0
    for root, dirs, files in os.walk("."):
        dirs[:] = [
            d for d in sorted(dirs)
            if d not in EXCLUDE_DIRS
            and not (root.endswith("web") and d == "wasm")
            and not (root.endswith("extension") and d == "wasm")
        ]
        for fname in sorted(files):
            path = os.path.join(root, fname)
            ext = os.path.splitext(fname)[1].lower()
            try:
                with open(path, "r", encoding="utf-8", errors="replace") as f:
                    n = sum(1 for _ in f)
            except OSError:
                continue
            for gname, pred in GROUPS.items():
                if pred(path, ext):
                    counts[gname][0] += 1
                    counts[gname][1] += n
                    total_files += 1
                    total_lines += n
                    break

    rows = "\n".join(f"| {g} | {v[0]} | {v[1]} |" for g, v in counts.items())
    content = f"""# Project Metrics

Updated: {time.strftime("%Y-%m-%d %H:%M:%S UTC", time.gmtime())}

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
{rows}
| **Total Project** | **{total_files}** | **{total_lines}** |
"""
    with open("metrics.md", "w", encoding="utf-8") as f:
        f.write(content)

