#!/usr/bin/env python3
"""Convert extracted_strings.json into a flat translation catalogue stub."""
import json
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: build_translation_template.py <extracted.json> <out.json>", file=sys.stderr)
        return 2
    src = json.load(open(sys.argv[1], encoding="utf-8"))
    catalogue = {"_meta": {
        "language": "zh_TW",
        "source_unique": src["summary"]["unique_strings"],
        "source_total_chars": src["summary"]["total_source_chars"],
    }}
    for entry in src["catalogue"]:
        catalogue[entry["key"]] = {
            "en": entry["source"],
            "zh_TW": "",
            "category": entry["category"],
            "refs": [f"{o['disk']}/{o['file']}:{o['line']}" for o in entry["occurrences"][:3]],
        }
    Path(sys.argv[2]).write_text(
        json.dumps(catalogue, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    print(f"wrote {len(catalogue)-1} entries to {sys.argv[2]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
