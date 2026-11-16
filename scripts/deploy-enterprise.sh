#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

OUT_DIR="${OUT_DIR:-dist/enterprise}"
START_DOCKER=false
for arg in "$@"; do
  case "$arg" in
    --up) START_DOCKER=true ;;
    --out=*) OUT_DIR="${arg#*=}" ;;
    --help)
      echo "Usage: $0 [--out=DIR] [--up]"
      exit 0
      ;;
    *)
      echo "Unknown option: $arg" >&2
      exit 2
      ;;
  esac
done

./scripts/build-all.sh

echo "[Enterprise] Assembling deployment bundle in $OUT_DIR..."
rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR/bundle/wasm"

cp web/index.html web/styles.css web/app.js web/interceptor.js web/policy.default.json "$OUT_DIR/bundle/"
cp -r web/js "$OUT_DIR/bundle/"
cp dist/wasm/data_guard.js dist/wasm/data_guard.wasm "$OUT_DIR/bundle/wasm/"
cp deploy/enterprise/Dockerfile deploy/enterprise/docker-compose.yml deploy/enterprise/nginx.conf "$OUT_DIR/"

./build/native/dataguard-admin --headless --export-policy "$OUT_DIR/bundle/policy.json"

echo "[Enterprise] Bundle ready at $OUT_DIR"
if [[ "$START_DOCKER" == "true" ]]; then
  if command -v docker >/dev/null 2>&1; then
    (cd "$OUT_DIR" && docker compose up -d --build)
  else
    echo "ERROR: docker is not installed." >&2
    exit 1
  fi
fi

