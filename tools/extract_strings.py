#!/usr/bin/env python3
"""
Extract Pascal string literals from Wizardry.Code source for i18n catalogue.

Scans WRITE/WRITELN calls and := assignments, pulls every 'string literal',
emits JSON with file/line/content + a suggested catalogue key.

Usage:
    python3 extract_strings.py <source_root> <output_json>
"""

import json
import re
import sys
import unicodedata
from collections import Counter
from pathlib import Path


PASCAL_STRING_RE = re.compile(r"'((?:[^']|'')*)'")
LINE_COMMENT_RE = re.compile(r"\(\*.*?\*\)|\{[^}]*\}", re.DOTALL)


def strip_comments(text: str) -> str:
    return LINE_COMMENT_RE.sub(" ", text)


def slug_key(text: str, max_len: int = 40) -> str:
    """Derive a stable catalogue key from the string content."""
    s = unicodedata.normalize("NFKD", text).encode("ascii", "ignore").decode()
    s = re.sub(r"[^A-Za-z0-9]+", "_", s).strip("_").lower()
    if not s:
        s = "blank"
    return s[:max_len]


CATEGORY_RULES = [
    ("border",     lambda s: set(s) <= set("+-|=*. ")),
    ("blank",      lambda s: not s.strip()),
    ("punct_only", lambda s: not re.search(r"[A-Za-z]", s)),
    ("error",      lambda s: any(t in s for t in ("** ", "!! ", "ERROR", "FAIL"))),
    ("prompt",     lambda s: s.rstrip().endswith(("?", "?>", ">", ":"))),
    ("menu_opt",   lambda s: bool(re.match(r"^[A-Z]\)", s.lstrip()))),
    ("title",      lambda s: s.isupper() and len(s.split()) <= 3 and len(s) <= 20),
    ("spell_word", lambda s: s.strip().rstrip(" -") in {"MURMUR", "CHANT", "PRAY", "INVOKE"}),
]


def categorize(s: str) -> str:
    for name, rule in CATEGORY_RULES:
        if rule(s):
            return name
    return "message"


def extract_file(path: Path) -> list[dict]:
    text = path.read_text(encoding="latin-1", errors="replace")
    text = strip_comments(text)
    results = []
    for lineno, line in enumerate(text.splitlines(), start=1):
        upper = line.upper()
        if not any(tag in upper for tag in ("WRITE", "WRITELN", ":=", "EXITADDP", "DSPTITLE", "STREQ", "CONCAT")):
            continue
        for m in PASCAL_STRING_RE.finditer(line):
            raw = m.group(1).replace("''", "'")
            if not raw:
                continue
            if len(raw) == 1 and raw in "0123456789":
                continue
            results.append({
                "file": path.name,
                "line": lineno,
                "content": raw,
                "len": len(raw),
                "category": categorize(raw),
            })
    return results


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__, file=sys.stderr)
        return 2
    root = Path(sys.argv[1]).expanduser().resolve()
    output = Path(sys.argv[2]).expanduser().resolve()

    files = sorted(p for p in root.rglob("*.TEXT.txt"))
    if not files:
        print(f"no Pascal sources under {root}", file=sys.stderr)
        return 1

    all_strings = []
    per_file = Counter()
    for f in files:
        items = extract_file(f)
        for item in items:
            item["disk"] = f.parent.name
        all_strings.extend(items)
        per_file[f.name] = len(items)

    # Build dedup catalogue: same content -> single canonical entry
    by_content = {}
    for item in all_strings:
        c = item["content"]
        if c not in by_content:
            by_content[c] = {
                "key": slug_key(c),
                "source": c,
                "category": item["category"],
                "occurrences": [],
                "translation": "",
            }
        by_content[c]["occurrences"].append({
            "disk": item["disk"], "file": item["file"], "line": item["line"]
        })

    # Resolve duplicate slugs by appending suffix
    seen_keys = Counter()
    for entry in by_content.values():
        base = entry["key"]
        seen_keys[base] += 1
        if seen_keys[base] > 1:
            entry["key"] = f"{base}_{seen_keys[base]}"

    catalogue = sorted(by_content.values(), key=lambda e: (e["category"], e["key"]))

    category_counts = Counter(e["category"] for e in catalogue)
    total_chars = sum(len(e["source"]) for e in catalogue)
    total_occurrences = sum(len(e["occurrences"]) for e in catalogue)

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps({
            "summary": {
                "files_scanned": len(files),
                "total_string_occurrences": total_occurrences,
                "unique_strings": len(catalogue),
                "total_source_chars": total_chars,
                "by_category": dict(category_counts),
                "by_file": dict(per_file),
            },
            "catalogue": catalogue,
        }, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    print(f"scanned {len(files)} files")
    print(f"  total occurrences: {total_occurrences}")
    print(f"  unique strings:    {len(catalogue)}")
    print(f"  total chars:       {total_chars}")
    print(f"  categories: {dict(category_counts)}")
    print(f"output: {output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
