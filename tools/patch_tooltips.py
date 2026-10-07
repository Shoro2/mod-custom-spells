"""Create a Spell.dbc tooltip candidate without changing gameplay records.

This tool has no DB, MPQ, deploy or restart capability. It changes only the
enUS Description string offset and its locale metadata. IDs, record order,
all other string slots and every gameplay dword remain byte-for-byte intact.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct


def read_columns(schema):
    columns = [c for c in re.findall(r"^\s*`(\w+)`", schema, re.M)
               if c != "spell_dbc"]
    if len(columns) != 234 or columns[0] != "ID":
        raise ValueError("Expected the current 234-column spell_dbc schema")
    return columns


def read_manifest(path):
    manifest = json.loads(path.read_text(encoding="utf-8"))
    if manifest["schema_version"] != 1 or manifest["locale"] != "enUS":
        raise ValueError("Unsupported tooltip manifest")
    texts = {}
    for row in manifest["entries"]:
        sid, text = row["id"], row["description"]
        if not isinstance(sid, int) or not 900000 <= sid <= 901199:
            raise ValueError("Manifest ID outside the unchanged custom block")
        if sid in texts or not isinstance(text, str) or not text.strip() or "\0" in text:
            raise ValueError(f"Invalid or duplicate tooltip for {sid}")
        texts[sid] = text
    return texts


def patch_bytes(raw, columns, texts, allow_missing=()):
    if len(raw) < 20:
        raise ValueError("Truncated WDBC header")
    magic, count, fields, size, slen = struct.unpack_from("<4sIIII", raw)
    end = 20 + count * size
    if magic != b"WDBC" or fields != 234 or size != 936 or len(raw) != end + slen:
        raise ValueError("Invalid Spell.dbc shape")
    strings = bytearray(raw[end:])
    if not strings or strings[0] != 0 or strings[-1] != 0:
        raise ValueError("Invalid string block")
    records = bytearray(raw[20:end])
    offsets = [i for i, c in enumerate(columns)
               if "_Lang_" in c and not c.endswith("_Mask")]
    index = {}
    for n in range(count):
        values = struct.unpack_from("<234I", records, n * size)
        sid = values[0]
        if sid in index:
            raise ValueError(f"Duplicate DBC ID {sid}")
        index[sid] = n
        for i in offsets:
            off = values[i]
            if off >= slen or strings.find(0, off) < 0:
                raise ValueError(f"Invalid string offset {sid}:{columns[i]}")
    missing = set(texts) - index.keys()
    allowed = set(allow_missing)
    if missing != allowed:
        raise ValueError(f"Missing IDs differ from explicit allowance: {sorted(missing)} vs {sorted(allowed)}")

    # Reuse complete strings, so repeating the operation does not grow the file.
    string_index = {}
    start = 0
    while start < len(strings):
        stop = strings.index(0, start)
        string_index.setdefault(bytes(strings[start:stop]), start)
        start = stop + 1
    desc = columns.index("Description_Lang_enUS")
    desc_mask = columns.index("Description_Lang_Mask")
    name_mask = columns.index("Name_Lang_Mask")
    changed = []
    for sid, text in sorted(texts.items()):
        if sid in missing:
            continue
        base = index[sid] * size
        encoded = text.encode("utf-8")
        old_off = struct.unpack_from("<I", records, base + desc * 4)[0]
        old_text = bytes(strings[old_off:strings.index(0, old_off)])
        if old_text == encoded:
            continue
        new_off = string_index.get(encoded)
        if new_off is None:
            new_off = len(strings)
            string_index[encoded] = new_off
            strings.extend(encoded + b"\0")
        struct.pack_into("<I", records, base + desc * 4, new_off)
        # Use the existing row's locale flag pattern, as the client sync does.
        mask = struct.unpack_from("<I", records, base + name_mask * 4)[0]
        struct.pack_into("<I", records, base + desc_mask * 4, mask)
        changed.append(sid)
    output = struct.pack("<4sIIII", magic, count, fields, size, len(strings)) + records + strings

    # Validate the preservation contract independently of text rendering.
    allowed_fields = {desc, desc_mask}
    for sid, n in index.items():
        before = struct.unpack_from("<234I", raw, 20 + n * size)
        after = struct.unpack_from("<234I", output, 20 + n * size)
        for i in range(fields):
            if before[i] != after[i] and (sid not in texts or i not in allowed_fields):
                raise ValueError(f"Unexpected gameplay/string change {sid}:{columns[i]}")
    if not output[end:].startswith(raw[end:]):
        raise ValueError("Original string block was changed")
    report = {"input_sha256": hashlib.sha256(raw).hexdigest(),
              "output_sha256": hashlib.sha256(output).hexdigest(),
              "record_count": count, "changed_ids": changed,
              "missing_ids": sorted(missing), "added_string_bytes": len(output) - len(raw),
              "allowed_fields": [columns[i] for i in sorted(allowed_fields)],
              "gameplay_dwords_preserved": True}
    return output, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--schema", type=Path, required=True)
    parser.add_argument("--manifest", type=Path,
                        default=Path(__file__).resolve().parents[1] / "data/tooltips_enUS.json")
    parser.add_argument("--allow-missing", type=int, nargs="*", default=[])
    args = parser.parse_args()
    if args.input.resolve() == args.output.resolve():
        parser.error("Use a separate candidate output; in-place patching is forbidden")
    columns = read_columns(args.schema.read_text(encoding="utf-8"))
    output, report = patch_bytes(args.input.read_bytes(), columns,
                                 read_manifest(args.manifest), args.allow_missing)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output)
    args.output.with_suffix(".report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
