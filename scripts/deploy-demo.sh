#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

OUT_DIR="${OUT_DIR:-dist/demo}"
PORT="${PORT:-8090}"
SERVE=true

for arg in "$@"; do
  case "$arg" in
    --no-serve) SERVE=false ;;
    --port=*) PORT="${arg#*=}" ;;
    --out=*) OUT_DIR="${arg#*=}" ;;
    --help)
      echo "Usage: $0 [--out=DIR] [--port=PORT] [--no-serve]"
      exit 0
      ;;
    *)
      echo "Unknown option: $arg" >&2
      exit 2
      ;;
  esac
done

./scripts/build-all.sh

echo "[Demo] Preparing demo environment in $OUT_DIR..."
rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR/web/wasm"

cp web/index.html web/styles.css web/app.js web/interceptor.js web/policy.default.json "$OUT_DIR/web/"
cp -r web/js web/assets "$OUT_DIR/web/"
cp dist/wasm/data_guard.js dist/wasm/data_guard.wasm "$OUT_DIR/web/wasm/"
cp deploy/demo/demo-scenarios.json "$OUT_DIR/web/demo-scenarios.json"
./build/native/dataguard-admin --headless --export-policy "$OUT_DIR/web/policy.json"

echo "[Demo] Running sample scan via C++ Admin CLI..."
./build/native/dataguard-admin --headless --scan "const key = 'AKIA1234567890ABCDEF'; email: alice@corp.com" || true

if [[ "$SERVE" == "true" ]]; then
  exec python3 deploy/local/local-server.py --dir "$OUT_DIR/web" --port "$PORT"
fi

