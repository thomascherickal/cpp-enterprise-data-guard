#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "[1/3] Building native C++ core, Admin GUI, and tests..."
cmake -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native -j"$(nproc)"

echo "[2/3] Locating Emscripten compiler (em++)..."
EMXX_BIN="${EMXX:-$(command -v em++ || true)}"
if [[ -z "$EMXX_BIN" && -x "/home/thomas/emsdk/upstream/emscripten/em++" ]]; then
  EMXX_BIN="/home/thomas/emsdk/upstream/emscripten/em++"
fi
if [[ -z "$EMXX_BIN" ]]; then
  echo "ERROR: em++ not found. Activate Emscripten SDK or set EMXX." >&2
  exit 1
fi

echo "[3/3] Compiling C++ scanner to WebAssembly ($EMXX_BIN)..."
mkdir -p dist/wasm web/wasm
"$EMXX_BIN" -std=c++17 -O2 -Isrc \
  src/core/json.cpp \
  src/core/validators.cpp \
  src/core/policy.cpp \
  src/core/rule_registry.cpp \
  src/core/text_index.cpp \
  src/core/redactor.cpp \
  src/core/scanner.cpp \
  src/wasm/wasm_bridge.cpp \
  -lembind \
  -s MODULARIZE=1 \
  -s EXPORT_NAME="createDataGuardModule" \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s DYNAMIC_EXECUTION=0 \
  -s ENVIRONMENT="web,worker,node" \
  -o dist/wasm/data_guard.js

cp dist/wasm/data_guard.js dist/wasm/data_guard.wasm web/wasm/
echo "Build complete: build/native/dataguard-admin, dist/wasm/data_guard.{js,wasm}"

