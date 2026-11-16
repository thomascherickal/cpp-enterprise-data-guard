#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

echo "Testing Enterprise deployment script..."
./scripts/deploy-enterprise.sh --out=dist/test-enterprise
test -f dist/test-enterprise/Dockerfile
test -f dist/test-enterprise/docker-compose.yml
test -f dist/test-enterprise/nginx.conf
test -f dist/test-enterprise/bundle/index.html
test -f dist/test-enterprise/bundle/js/wasm-client.js
test -f dist/test-enterprise/bundle/policy.json
test -f dist/test-enterprise/bundle/wasm/data_guard.wasm

echo "Testing Local deployment script..."
./scripts/deploy-local.sh --out=dist/test-local --no-serve
test -x dist/test-local/bin/dataguard-admin
test -f dist/test-local/web/index.html
test -f dist/test-local/web/policy.json
test -f dist/test-local/web/wasm/data_guard.wasm

echo "Testing Demo deployment script..."
./scripts/deploy-demo.sh --out=dist/test-demo --no-serve
test -f dist/test-demo/web/index.html
test -f dist/test-demo/web/demo-scenarios.json
test -f dist/test-demo/web/wasm/data_guard.wasm

rm -rf dist/test-enterprise dist/test-local dist/test-demo
echo "All deployment script smoke tests passed!"

