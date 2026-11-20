#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

./scripts/deploy-local.sh --out=dist/e2e-local --no-serve >/dev/null
python3 deploy/local/local-server.py --dir dist/e2e-local/web --port 18099 >/tmp/dg_e2e.log 2>&1 &
SRV_PID=$!
trap 'kill $SRV_PID 2>/dev/null || true; rm -rf dist/e2e-local' EXIT
sleep 0.5

curl -sf http://127.0.0.1:18099/index.html | grep -q "Enterprise Data Guard"
curl -sfI http://127.0.0.1:18099/wasm/data_guard.wasm | grep -iq "application/wasm"
curl -sf http://127.0.0.1:18099/assets/favicon.svg | grep -q "<svg"
curl -sf http://127.0.0.1:18099/policy.json | grep -q '"redactionMode": "REMOVE"'

./dist/e2e-local/bin/dataguard-admin --headless --policy dist/e2e-local/web/policy.json --scan "safe prompt" >/dev/null
echo "All E2E tests passed!"

