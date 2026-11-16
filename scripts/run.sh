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
Enterprise Data Guard — Unified Runner

Usage: ./scripts/run.sh [OPTIONS]

Options:
  --mode=local        Build and run Local Web Dashboard (default, port 8080)
  --mode=demo         Build and run Demo Web Dashboard + CLI demo (port 8090)
  --mode=enterprise   Build Enterprise Docker bundle and start via docker compose
  --gui               Launch the C++ GTK+ 3 Admin Control Centre GUI
  --gui-only          Launch only the Admin Control Centre GUI (skip web server)
  --port=PORT         Override web dashboard port
  --scan="TEXT"       Run a one-shot CLI scan on TEXT and exit
  --test              Run full C++ unit tests, 100% coverage, WASM & deploy tests
  --rebuild           Force a fresh build before running
  --check             Build and verify artifacts without starting a blocking server
  -h, --help          Show this help message

Examples:
  ./scripts/run.sh                      # Start Local Web Dashboard on :8080
  ./scripts/run.sh --gui                # Start Web Dashboard + Admin GUI
  ./scripts/run.sh --gui-only           # Start only the C++ Admin GUI
  ./scripts/run.sh --mode=demo          # Start Demo Web Dashboard on :8090
  ./scripts/run.sh --scan="AKIA1234567890ABCDEF"
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
    *)
      echo "Unknown option: $arg" >&2
      usage >&2
      exit 2
      ;;
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
  echo "[Run] Launching C++ Admin Control Centre GUI in background..."
  ./build/native/dataguard-admin &
fi

case "$MODE" in
  local)
    PORT="${PORT:-8080}"
    exec ./scripts/deploy-local.sh --port="$PORT"
    ;;
  demo)
    PORT="${PORT:-8090}"
    exec ./scripts/deploy-demo.sh --port="$PORT"
    ;;
  enterprise)
    exec ./scripts/deploy-enterprise.sh --up
    ;;
  *)
    echo "Invalid mode: $MODE (expected local, demo, or enterprise)" >&2
    exit 2
    ;;
esac
