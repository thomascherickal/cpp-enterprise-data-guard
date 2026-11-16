#!/usr/bin/env python3
"""Verify 100% gcov line coverage for C++ core and admin controller modules."""

import glob
import os
import subprocess
import sys

TARGET_SUFFIXES = (
    "src/core/json.cpp",
    "src/core/validators.cpp",
    "src/core/policy.cpp",
    "src/core/rule_registry.cpp",
    "src/core/text_index.cpp",
    "src/core/redactor.cpp",
    "src/core/scanner.cpp",
    "src/admin_gui/admin_controller.cpp",
)


def main():
    build_dir = os.path.abspath("build/coverage")
    gcda_files = glob.glob(os.path.join(build_dir, "**", "*.gcda"), recursive=True)
    if not gcda_files:
        print("ERROR: No .gcda coverage files found in build/coverage", file=sys.stderr)
        return 1

    gcov_dir = os.path.join(build_dir, "gcov_reports")
    os.makedirs(gcov_dir, exist_ok=True)

    subprocess.run(
        ["gcov", "-p", *gcda_files],
        cwd=gcov_dir,
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    gcov_files = glob.glob(os.path.join(gcov_dir, "*.gcov"))
    all_ok = True
    found_targets = set()

    for gcov_path in sorted(gcov_files):
        source_line = None
        total = 0
        covered = 0
        uncovered_lines = []

        with open(gcov_path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                if line.startswith("        -:    0:Source:"):
                    source_line = line.split("Source:", 1)[1].strip()
                    continue
                parts = line.split(":", 2)
                if len(parts) < 3:
                    continue
                count_str = parts[0].strip()
                lineno_str = parts[1].strip()
                if not lineno_str.isdigit() or int(lineno_str) == 0:
                    continue
                if count_str in ("-", ""):
                    continue
                total += 1
                if count_str == "#####":
                    uncovered_lines.append(int(lineno_str))
                else:
                    covered += 1

        if not source_line:
            continue
        matched_suffix = next((s for s in TARGET_SUFFIXES if source_line.endswith(s)), None)
        if not matched_suffix:
            continue

        found_targets.add(matched_suffix)
        pct = (covered / total * 100.0) if total else 100.0
        print(f"{matched_suffix}: {covered}/{total} lines ({pct:.2f}%)")
        if uncovered_lines:
            all_ok = False
            print(f"  Uncovered lines in {matched_suffix}: {uncovered_lines}", file=sys.stderr)

    missing = set(TARGET_SUFFIXES) - found_targets
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

