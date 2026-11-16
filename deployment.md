# Deployment Guide

This guide covers deploying **Enterprise Data Guard** in three modes: **Local**, **Demo**, and **Enterprise**.

## Prerequisites

- **C++17 compiler** (`g++` or `clang++`), `cmake`, and `pkg-config`
- **GTK+ 3 development libraries** (`libgtk-3-dev` / `gtk+-3.0`) for the native Admin Control Centre GUI
- **Emscripten SDK** (`em++`) for compiling the C++ scanner to WebAssembly
- **Python 3** (for local/demo HTTP serving) or **Docker & Docker Compose** (for enterprise deployment)

---

## 1. Local Deployment

Builds the native C++ Admin Control Centre (`dataguard-admin`), compiles the WebAssembly scanner (`data_guard.wasm`), exports `policy.json`, and serves the user web dashboard locally on `http://127.0.0.1:8080`.

```bash
# Build, install to dist/local/, and start the local web dashboard
./scripts/deploy-local.sh
```

Options:
- `--port=PORT`: Serve on a custom port (default: `8080`).
- `--out=DIR`: Install to a custom directory (default: `dist/local`).
- `--no-serve`: Build and package without starting the HTTP server.

To launch the native C++ Admin Control Centre GUI:
```bash
./dist/local/bin/dataguard-admin
```

---

## 2. Demo Deployment

Prepares an interactive demo environment in `dist/demo/` with pre-configured scenarios (`demo-scenarios.json`), runs a live CLI demonstration scan, and serves the web dashboard on `http://127.0.0.1:8090`.

```bash
# Build, run sample CLI scan, and start the demo web dashboard
./scripts/deploy-demo.sh
```

Options:
- `--port=PORT`: Serve on a custom port (default: `8090`).
- `--out=DIR`: Output to a custom directory (default: `dist/demo`).
- `--no-serve`: Build, package, and run the CLI demo scan without starting the HTTP server.

---

## 3. Enterprise Deployment

Packages a production-ready, read-only Nginx + Docker Compose bundle in `dist/enterprise/` containing the static web dashboard, `data_guard.wasm`, and an enterprise-exported `policy.json` with strict Content Security Policy headers.

```bash
# Assemble the enterprise bundle in dist/enterprise/
./scripts/deploy-enterprise.sh

# Assemble and immediately start the containerized service on port 8080
./scripts/deploy-enterprise.sh --up
```

Options:
- `--out=DIR`: Output to a custom directory (default: `dist/enterprise`).
- `--up`: Run `docker compose up -d --build` inside the output directory after packaging.

### Enterprise Bundle Contents
- `Dockerfile`: `nginx:1.27-alpine` with healthcheck on port `8080`.
- `docker-compose.yml`: Read-only root filesystem with `tmpfs` mounts for Nginx cache/runtime.
- `nginx.conf`: Configures `application/wasm` MIME type, `no-store` caching for `policy.json`, and strict CSP (`default-src 'self'; script-src 'self' 'wasm-unsafe-eval';`).
- `bundle/`: Static web dashboard, `wasm/data_guard.{js,wasm}`, and `policy.json`.

### Updating Enterprise Policy
1. Open the C++ Admin Control Centre (`./build/native/dataguard-admin`), configure rules/allowlists, and export `policy.json`.
2. Or use headless CLI export:
   ```bash
   ./build/native/dataguard-admin --headless --export-policy dist/enterprise/bundle/policy.json
   ```
