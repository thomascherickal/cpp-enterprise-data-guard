#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

MODE="local"
PORT=""
LAUNCH_GUI=false
LAUNCH_WEB=true
REBUILD=false
CHECK_ONLY=false
SCAN_TEXT=""

usage() {
  cat <<'EOF'
Usage: ./scripts/run.sh [--mode=local|demo|enterprise] [--port=PORT]
                        [--gui|--gui-only] [--scan="TEXT"] [--test]
                        [--rebuild] [--check] [-h|--help]
EOF
}

for arg in "$@"; do
  case "$arg" in
    --mode=*) MODE="${arg#*=}" ;;
    --port=*) PORT="${arg#*=}" ;;
    --gui) LAUNCH_GUI=true ;;
    --gui-only) LAUNCH_GUI=true; LAUNCH_WEB=false ;;
    --rebuild) REBUILD=true ;;
    --check) CHECK_ONLY=true ;;
    --test) exec ./scripts/run-tests.sh ;;
    --scan=*) SCAN_TEXT="${arg#*=}" ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $arg" >&2; usage >&2; exit 2 ;;
  esac
done

if [[ "$REBUILD" == "true" || ! -x "build/native/dataguard-admin" || ! -f "web/wasm/data_guard.wasm" ]]; then
  ./scripts/build-all.sh
fi

if [[ -n "$SCAN_TEXT" ]]; then
  exec ./build/native/dataguard-admin --headless --scan "$SCAN_TEXT"
fi

if [[ "$CHECK_ONLY" == "true" ]]; then
  echo "[Check] Native binary: build/native/dataguard-admin"
  echo "[Check] WASM module:   web/wasm/data_guard.wasm"
  ./build/native/dataguard-admin --headless --scan "hello world" >/dev/null
  echo "[Check] Ready."
  exit 0
fi

if [[ "$LAUNCH_GUI" == "true" ]]; then
  if [[ "$LAUNCH_WEB" == "false" ]]; then
    exec ./build/native/dataguard-admin
  fi
  ./build/native/dataguard-admin &
fi

case "$MODE" in
  local) exec ./scripts/deploy-local.sh --port="${PORT:-8080}" ;;
  demo) exec ./scripts/deploy-demo.sh --port="${PORT:-8090}" ;;
  enterprise) exec ./scripts/deploy-enterprise.sh --up ;;
  *) echo "Invalid mode: $MODE" >&2; exit 2 ;;
esac
