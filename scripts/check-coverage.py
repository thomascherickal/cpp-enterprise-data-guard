#!/usr/bin/env python3
"""Verify 100% gcov line coverage for C++ core and admin controller modules."""

import glob
import os
import subprocess
import sys

TARGET_SUFFIXES = (
    "src/core/json_value.cpp", "src/core/json_dump.cpp",
    "src/core/json_parser.cpp", "src/core/json_parser_tokens.cpp",
    "src/core/validators_checksum.cpp", "src/core/validators_entropy.cpp",
    "src/core/policy_io.cpp", "src/core/policy_validate.cpp",
    "src/core/rule_registry.cpp", "src/core/text_index.cpp",
    "src/core/redactor.cpp", "src/core/scanner_rules.cpp",
    "src/core/scanner_scoring.cpp", "src/core/scanner.cpp",
    "src/admin_gui/admin_controller.cpp", "src/admin_gui/admin_cli.cpp",
)


def parse_gcov(gcov_path):
    source_line, total, covered, uncovered = None, 0, 0, []
    with open(gcov_path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            if line.startswith("        -:    0:Source:"):
                source_line = line.split("Source:", 1)[1].strip()
                continue
            parts = line.split(":", 2)
            if len(parts) < 3:
                continue
            count_str, lineno_str = parts[0].strip(), parts[1].strip()
            if not lineno_str.isdigit() or int(lineno_str) == 0 or count_str in ("-", ""):
                continue
            total += 1
            if count_str == "#####":
                uncovered.append(int(lineno_str))
            else:
                covered += 1
    return source_line, total, covered, uncovered


def main():
    build_dir = os.path.abspath("build/coverage")
    gcda_files = glob.glob(os.path.join(build_dir, "**", "*.gcda"), recursive=True)
    if not gcda_files:
        print("ERROR: No .gcda files found in build/coverage", file=sys.stderr)
        return 1

    gcov_dir = os.path.join(build_dir, "gcov_reports")
    os.makedirs(gcov_dir, exist_ok=True)
    subprocess.run(["gcov", "-p", *gcda_files], cwd=gcov_dir, check=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    all_ok, found = True, set()
    for gcov_path in sorted(glob.glob(os.path.join(gcov_dir, "*.gcov"))):
        src, total, covered, uncovered = parse_gcov(gcov_path)
        if not src:
            continue
        suffix = next((s for s in TARGET_SUFFIXES if src.endswith(s)), None)
        if not suffix:
            continue
        found.add(suffix)
        pct = (covered / total * 100.0) if total else 100.0
        print(f"{suffix}: {covered}/{total} lines ({pct:.2f}%)")
        if uncovered:
            all_ok = False
            print(f"  Uncovered lines in {suffix}: {uncovered}", file=sys.stderr)

    missing = set(TARGET_SUFFIXES) - found
    if missing:
        print(f"ERROR: Missing coverage reports for: {sorted(missing)}", file=sys.stderr)
        return 1
    if not all_ok:
        print("FAIL: Coverage is below 100%.", file=sys.stderr)
        return 1
    print("SUCCESS: 100.00% line coverage achieved across all target C++ modules!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
