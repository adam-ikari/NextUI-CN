#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""TR key audit tool for downstream localization.

Scans source files for TR("key") usages, then compares against a .lang file
(key=value per line) and reports:
  - missing keys (used in code but not in lang file)
  - unused keys (present in lang file but not used in code)
  - duplicate keys in lang file

This is intentionally lightweight and doesn't require third-party deps.

Exit codes:
  0: OK (no missing keys)
  2: missing keys exist
  3: usage scan found no keys (likely misconfigured path)
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path

TR_RE = re.compile(r"\bTR\(\s*\"((?:\\\\.|[^\"\\\\])*)\"\s*\)")


@dataclass
class LangParseResult:
    mapping: dict[str, str]
    duplicates: list[str]


def iter_source_files(root: Path, includes: list[str], excludes: list[str]) -> list[Path]:
    inc_exts = {e.lower() if e.startswith('.') else f'.{e.lower()}' for e in includes}
    exclude_parts = [p.strip().replace('\\', '/') for p in excludes if p.strip()]

    out: list[Path] = []
    for p in root.rglob('*'):
        if not p.is_file():
            continue

        # quick exclude by path substring
        sp = p.as_posix()
        if any(ex in sp for ex in exclude_parts):
            continue

        if p.suffix.lower() in inc_exts:
            out.append(p)

    return out


def unescape_c_string(s: str) -> str:
    # we only need to handle sequences used by our i18n loader
    s = s.replace('\\n', '\n').replace('\\t', '\t').replace('\\\\', '\\')
    s = s.replace('\\"', '"')
    return s


def scan_tr_keys(paths: list[Path]) -> tuple[set[str], dict[str, list[str]]]:
    keys: set[str] = set()
    occurrences: dict[str, list[str]] = {}

    for fp in paths:
        try:
            text = fp.read_text(encoding='utf-8', errors='ignore')
        except Exception:
            continue

        for m in TR_RE.finditer(text):
            raw = m.group(1)
            k = unescape_c_string(raw)
            keys.add(k)
            occurrences.setdefault(k, []).append(str(fp))

    return keys, occurrences


def parse_lang_file(lang_path: Path) -> LangParseResult:
    mapping: dict[str, str] = {}
    duplicates: list[str] = []

    if not lang_path.exists():
        return LangParseResult(mapping=mapping, duplicates=duplicates)

    with lang_path.open('r', encoding='utf-8-sig', errors='ignore') as f:
        for idx, line in enumerate(f, start=1):
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if '=' not in line:
                continue
            k, v = line.split('=', 1)
            k = k.strip()
            v = v.strip()
            if not k:
                continue
            if k in mapping:
                duplicates.append(f"{k} (line {idx})")
            mapping[k] = v

    return LangParseResult(mapping=mapping, duplicates=duplicates)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description='Audit TR("key") usage vs .lang file')
    ap.add_argument('--root', default='.', help='Scan root directory (default: .)')
    ap.add_argument('--lang', required=True, help='Language file path, e.g. workspace/i18n/locales/zh_CN.lang')
    ap.add_argument('--include', default='c,h', help='Comma-separated extensions to include (default: c,h)')
    ap.add_argument('--exclude', default='build/,cores/patches/,workspace/_unmaintained/', help='Comma-separated path substrings to exclude')
    ap.add_argument('--write-keys', default='', help='Optional output file path to write sorted keys used in code')

    args = ap.parse_args(argv)

    root = Path(args.root).resolve()
    lang_path = Path(args.lang).resolve()
    includes = [x.strip() for x in args.include.split(',') if x.strip()]
    excludes = [x.strip() for x in args.exclude.split(',') if x.strip()]

    files = iter_source_files(root, includes, excludes)
    used_keys, occurrences = scan_tr_keys(files)

    if not used_keys:
        print('[tr_audit] No TR("...") keys found. Check --root / --exclude settings.')
        return 3

    if args.write_keys:
        outp = Path(args.write_keys).resolve()
        outp.parent.mkdir(parents=True, exist_ok=True)
        outp.write_text('\n'.join(sorted(used_keys)) + '\n', encoding='utf-8')

    lang = parse_lang_file(lang_path)
    lang_keys = set(lang.mapping.keys())

    missing = sorted(used_keys - lang_keys)
    unused = sorted(lang_keys - used_keys)

    print(f"[tr_audit] scanned files: {len(files)}")
    print(f"[tr_audit] TR keys used : {len(used_keys)}")
    print(f"[tr_audit] lang keys    : {len(lang_keys)}")

    if lang.duplicates:
        print('\n[tr_audit] duplicate keys in lang file:')
        for d in lang.duplicates:
            print(f"  - {d}")

    if missing:
        print('\n[tr_audit] missing translations (present in code, absent in lang):')
        for k in missing:
            # show a single example path to speed up fixing
            example = occurrences.get(k, ['?'])[0]
            print(f"  - {k}    (eg: {example})")

    if unused:
        print('\n[tr_audit] unused translations (present in lang, not referenced by TR()):')
        for k in unused:
            print(f"  - {k}")

    return 2 if missing else 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv[1:]))
