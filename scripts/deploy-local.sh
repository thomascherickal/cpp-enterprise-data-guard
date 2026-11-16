#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

OUT_DIR="${OUT_DIR:-dist/local}"
PORT="${PORT:-8080}"
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

echo "[Local] Installing Admin GUI and Web Dashboard to $OUT_DIR..."
rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR/bin" "$OUT_DIR/web/wasm"

cp build/native/dataguard-admin "$OUT_DIR/bin/"
cp web/index.html web/styles.css web/app.js web/interceptor.js web/policy.default.json "$OUT_DIR/web/"
cp -r web/js web/assets "$OUT_DIR/web/"
cp dist/wasm/data_guard.js dist/wasm/data_guard.wasm "$OUT_DIR/web/wasm/"
./build/native/dataguard-admin --headless --export-policy "$OUT_DIR/web/policy.json"

echo "[Local] Admin GUI binary: $OUT_DIR/bin/dataguard-admin"
echo "[Local] Web Dashboard dir: $OUT_DIR/web"

if [[ "$SERVE" == "true" ]]; then
  exec python3 deploy/local/local-server.py --dir "$OUT_DIR/web" --port "$PORT"
fi

