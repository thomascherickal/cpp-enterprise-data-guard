#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

./scripts/build-all.sh

echo "[Extension] Packaging dist/enterprise-data-guard-extension.zip..."
python3 - <<'PY'
import os, zipfile

ext_dir = "extension"
out_zip = "dist/enterprise-data-guard-extension.zip"
os.makedirs("dist", exist_ok=True)

with zipfile.ZipFile(out_zip, "w", zipfile.ZIP_DEFLATED) as zf:
    for root, _, files in os.walk(ext_dir):
        for fname in sorted(files):
            full = os.path.join(root, fname)
            rel = os.path.relpath(full, ext_dir)
            zf.write(full, rel)

print(f"Created {out_zip}")
PY
