# Installation Guide

This guide explains how to install dependencies, build, and verify **Enterprise Data Guard** from source.

## 1. Prerequisites

### Linux (Ubuntu / Debian)
Install the C++17 toolchain, CMake, GTK+ 3 development headers, Python 3, and Node.js:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libgtk-3-dev python3 nodejs git
```

### Emscripten SDK (for WebAssembly compilation)
Install and activate `emsdk` so `em++` is available:

```bash
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
source ~/emsdk/emsdk_env.sh
```

> **Note:** `scripts/build-all.sh` automatically checks `PATH` and `~/emsdk/upstream/emscripten/em++`, or you can set `EMXX=/path/to/em++`.

### Optional (Enterprise Container Deployment)
- **Docker** and **Docker Compose** (`docker compose`) if deploying via `./scripts/run.sh --mode=enterprise`.

---

## 2. Clone the Repository

```bash
git clone https://github.com/thomascherickal/cpp-enterprise-data-guard.git
cd cpp-enterprise-data-guard
```

---

## 3. Build Native Binaries & WebAssembly

Run the build script to compile:
1. `dataguard_core` (C++17 scanning engine)
2. `build/native/dataguard-admin` (native C++ GTK+ 3 Admin Control Centre)
3. `dist/wasm/data_guard.{js,wasm}` and `web/wasm/data_guard.{js,wasm}` (WebAssembly browser module)

```bash
./scripts/build-all.sh
```

---

## 4. Verify Installation

Run a quick readiness check:
```bash
./scripts/run.sh --check
```

Run the full verification suite (C++ unit tests, 100% `gcov` line coverage check, Node.js WASM integration tests, and deployment smoke tests):
```bash
./scripts/run.sh --test
```

---

## 5. Install Locally (Optional)

To package the Admin GUI binary and static web bundle into `dist/local/` without starting the HTTP server:

```bash
./scripts/deploy-local.sh --no-serve
```

You can optionally symlink or copy the Admin CLI/GUI binary into your user `PATH`:
```bash
mkdir -p ~/.local/bin
cp dist/local/bin/dataguard-admin ~/.local/bin/
```

---

## 6. Next Steps

- **Run the project**: See [scripts/run.sh](scripts/run.sh) (`./scripts/run.sh` for Local Web Dashboard, `--gui` for Admin Control Centre, `--mode=demo` for Demo).
- **Deploy to Enterprise / Local / Demo**: See [deployment.md](deployment.md).
- **Operational Manual**: See [manual.md](manual.md).

