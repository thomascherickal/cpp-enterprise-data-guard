#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

START_ALL=$(date +%s)
REPORT="test-time-report.md"

run_stage() {
  local label="$1"; shift
  local t0 t1 dt
  t0=$(date +%s)
  "$@"
  t1=$(date +%s)
  dt=$((t1 - t0))
  printf "| %s | %ds | PASS |\n" "$label" "$dt" >> "$REPORT.tmp"
}

rm -f "$REPORT.tmp"

run_stage "C++ Unit Tests & 100% Coverage" bash -c '
  rm -rf build/coverage
  cmake -B build/coverage -DENABLE_COVERAGE=ON -DBUILD_ADMIN_GUI=OFF -DCMAKE_BUILD_TYPE=Debug
  cmake --build build/coverage -j"$(nproc)"
  ctest --test-dir build/coverage --output-on-failure
  python3 scripts/check-coverage.py
'
run_stage "Native + WASM Build & Integration" bash -c './scripts/build-all.sh && node tests/wasm/test_wasm.mjs'
run_stage "Security Tests" node tests/security/test_security.mjs
run_stage "Performance & Load Tests" node tests/perf/test_performance.mjs
run_stage "End-to-End (E2E) Tests" ./tests/e2e/test_e2e.sh
run_stage "Deployment Smoke Tests" ./tests/deploy/test_deploy_scripts.sh

TOTAL_SEC=$(($(date +%s) - START_ALL))
if [[ "$TOTAL_SEC" -gt 1800 ]]; then
  echo "ERROR: Test suite exceeded 30-minute limit (${TOTAL_SEC}s)" >&2
  exit 1
fi

cat > "$REPORT" <<EOF
# Test Time Report

Updated: $(date -u +"%Y-%m-%d %H:%M:%S UTC") | Total Runtime: **${TOTAL_SEC}s** (Limit: 1800s)

\`\`\`mermaid
flowchart LR
    U["Unit + 100% gcov"] --> W["WASM Integration"]
    W --> S["Security & Perf"]
    S --> E["E2E & Deploy Smoke"]
    style U fill:#004e8c,stroke:#38bdf8,color:#ffffff
    style W fill:#0063b1,stroke:#60a5fa,color:#ffffff
    style S fill:#1e2430,stroke:#94a3b8,color:#ffffff
    style E fill:#065f46,stroke:#34d399,color:#ffffff
\`\`\`

| Suite | Duration | Status |
| :--- | ---: | :--- |
$(cat "$REPORT.tmp")
| **Total** | **${TOTAL_SEC}s** | **PASS** |
EOF
rm -f "$REPORT.tmp"

python3 scripts/generate-reports.py
python3 scripts/enforce-rules.py
echo "ALL TESTS, 100% COVERAGE, AND AGENTS.MD RULES PASSED!"
