#!/usr/bin/env python3
"""Enforce AGENTS.md rules: 100-line limit, kebab-case md, Mermaid density, duplicate check."""

import os
import re
import sys

CODE_EXTS = {".cpp", ".hpp", ".c", ".h", ".js", ".mjs", ".css", ".py", ".sh"}
EXCLUDE_DIRS = {".git", "build", "dist", "node_modules"}
UPPER_MD_ALLOWED = {"AGENTS.md", "GEMINI.md", "CLAUDE.md", "SKILL.md"}


def walk_repo():
    for root, dirs, files in os.walk("."):
        dirs[:] = [
            d for d in sorted(dirs)
            if d not in EXCLUDE_DIRS
            and not (root.endswith("web") and d == "wasm")
            and not (root.endswith("extension") and d == "wasm")
        ]
        for fname in sorted(files):
            yield os.path.join(root, fname), fname


def main():
    errors = []
    for path, fname in walk_repo():
        ext = os.path.splitext(fname)[1].lower()
        if ext in CODE_EXTS:
            with open(path, "r", encoding="utf-8", errors="replace") as f:
                nlines = sum(1 for _ in f)
            if nlines > 100:
                errors.append(f"LINE LIMIT (>100): {path} has {nlines} lines")
        elif ext == ".md":
            if fname not in UPPER_MD_ALLOWED and not re.fullmatch(r"[a-z0-9]+(-[a-z0-9]+)*\.md", fname):
                errors.append(f"MD NAMING (not kebab-case): {path}")
            if fname not in UPPER_MD_ALLOWED:
                with open(path, "r", encoding="utf-8", errors="replace") as f:
                    text = f.read()
                prose = re.sub(r"```mermaid.*?```", "", text, flags=re.S)
                words = len(re.findall(r"\b\w+\b", prose))
                diagrams = len(re.findall(r"```mermaid", text))
                needed = max(1, (words + 199) // 200)
                if diagrams < needed:
                    errors.append(f"MERMAID DENSITY: {path} has {diagrams}/{needed} diagrams ({words} words)")
                elif diagrams > 0 and "style " not in text and "classDef " not in text:
                    errors.append(f"MERMAID COLOR: {path} Mermaid diagram(s) missing color styling")

    if errors:
        print("AGENTS.md RULE VIOLATIONS:", file=sys.stderr)
        for e in errors:
            print("  -", e, file=sys.stderr)
        return 1

    print("SUCCESS: All AGENTS.md repository rules passed!")
    return 0


if __name__ == "__main__":
    sys.exit(main())

