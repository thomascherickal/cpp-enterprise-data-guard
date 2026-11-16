#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "=== [1/3] C++ Unit Tests + 100% Coverage Check ==="
rm -rf build/coverage
cmake -B build/coverage -DENABLE_COVERAGE=ON -DBUILD_ADMIN_GUI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build/coverage -j"$(nproc)"
ctest --test-dir build/coverage --output-on-failure
python3 scripts/check-coverage.py

echo "=== [2/3] WebAssembly Build & Node.js Integration Tests ==="
./scripts/build-all.sh
node tests/wasm/test_wasm.mjs

echo "=== [3/3] Deployment Scripts Smoke Tests ==="
./tests/deploy/test_deploy_scripts.sh

echo "ALL TESTS AND COVERAGE CHECKS PASSED!"

