"""Check the icon badges of data/spellbook_enUS.json.

Every custom class spell shows the icon of the stock spell it modifies ("icon_spell") with a short
text drawn on it ("badge"), so one base icon is reused and only the pair (icon, badge) is unique.
This tool fails (exit 1) on

  - an entry without a badge, or a badge that is not 1-5 printable ASCII characters
    (no quote, backslash or "|": the client treats "|" as an escape code);
  - an entry without an icon_spell, or an icon_spell that is not a stock spell (below 900000);
  - two entries with the same (SpellIconID, badge) pair. Two different icon_spell ids can share one
    icon, so the pair is compared by SpellIconID (field 133 of the server Spell.dbc), not by spell id;
  - a SPELL_BADGES block in lua/CustomSpells_Server.lua that differs from the manifest
    (tools/client_spell_sync.py --lua writes it).

    python tools/check_badges.py                 # reads C:\\wowstuff\\dcore\\Data\\dbc\\Spell.dbc
    python tools/check_badges.py --dbc PATH      # another Spell.dbc (or env FL_SPELL_DBC)
    python tools/check_badges.py --no-dbc        # compare icon_spell ids instead of icon ids (weaker)
"""
import argparse
import collections
import json
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
MODULE = os.path.dirname(HERE)
MANIFEST = os.path.join(MODULE, "data", "spellbook_enUS.json")
SERVER_LUA = os.path.join(MODULE, "lua", "CustomSpells_Server.lua")
DEFAULT_DBC = r"C:\wowstuff\dcore\Data\dbc\Spell.dbc"
ICON_FIELD = 133            # SpellIconID in the 3.3.5a Spell.dbc
MAX_LEN = 5
CUSTOM_LO = 900000
BLOCK_RE = re.compile(r"-- BEGIN GENERATED SPELL_BADGES.*?\n(.*?)-- END GENERATED SPELL_BADGES", re.S)
ROW_RE = re.compile(r'^\s*\[(\d+)\] = "((?:[^"\\]|\\.)*)",', re.M)


def read_icons(path, wanted):
    """{spell id: SpellIconID} of the wanted ids from a Spell.dbc."""
    data = open(path, "rb").read()
    magic, count, fields, size, _slen = struct.unpack_from("<4sIIII", data, 0)
    if magic != b"WDBC" or fields <= ICON_FIELD or size < (ICON_FIELD + 1) * 4:
        raise SystemExit("%s: not a 3.3.5a Spell.dbc" % path)
    out = {}
    for r in range(count):
        base = 20 + r * size
        sid = struct.unpack_from("<I", data, base)[0]
        if sid in wanted:
            out[sid] = struct.unpack_from("<I", data, base + ICON_FIELD * 4)[0]
    return out


def lua_block(path):
    """{id: badge} of the generated SPELL_BADGES block, or None when the file has none."""
    text = open(path, encoding="utf-8").read()
    m = BLOCK_RE.search(text)
    if not m:
        return None
    return {int(i): b.replace('\\"', '"').replace("\\\\", "\\") for i, b in ROW_RE.findall(m.group(1))}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dbc", default=os.environ.get("FL_SPELL_DBC", DEFAULT_DBC))
    ap.add_argument("--no-dbc", action="store_true", help="compare icon_spell ids, not SpellIconIDs")
    ap.add_argument("--no-lua", action="store_true", help="skip the SPELL_BADGES block comparison")
    args = ap.parse_args()

    entries = json.load(open(MANIFEST, encoding="utf-8"))["entries"]
    problems = []
    for e in entries:
        sid, badge = e["id"], e.get("badge")
        tag = "%d (%s)" % (sid, e["name"])
        if not isinstance(badge, str) or not badge:
            problems.append("%s: no badge" % tag)
        elif len(badge) > MAX_LEN:
            problems.append("%s: badge %r is longer than %d characters" % (tag, badge, MAX_LEN))
        elif not all(0x20 <= ord(c) <= 0x7E for c in badge) or any(c in '"\\|' for c in badge) \
                or badge != badge.strip():
            problems.append("%s: badge %r is not plain printable ASCII" % (tag, badge))
        icon = e.get("icon_spell")
        if not isinstance(icon, int) or icon <= 0:
            problems.append("%s: no icon_spell" % tag)
        elif icon >= CUSTOM_LO:
            problems.append("%s: icon_spell %d is a custom spell, not the stock spell it modifies" % (tag, icon))

    icons = {}
    if not args.no_dbc:
        if not os.path.exists(args.dbc):
            raise SystemExit("Spell.dbc not found: %s (--dbc PATH, or --no-dbc)" % args.dbc)
        wanted = {e["icon_spell"] for e in entries if isinstance(e.get("icon_spell"), int)}
        icons = read_icons(args.dbc, wanted)
        for sid in sorted(wanted - set(icons)):
            problems.append("icon_spell %d has no record in %s" % (sid, args.dbc))

    groups = collections.defaultdict(list)
    for e in entries:
        icon, badge = e.get("icon_spell"), e.get("badge")
        if not isinstance(icon, int) or not isinstance(badge, str) or not badge:
            continue
        key = (icons.get(icon) if not args.no_dbc else icon, badge)
        groups[key].append(e)
    for (icon, badge), group in sorted(groups.items(), key=lambda kv: str(kv[0])):
        if len(group) > 1:
            problems.append("duplicate (icon %s, badge %r): %s" % (
                icon, badge, "; ".join("%d %s" % (g["id"], g["name"]) for g in group)))

    if not args.no_lua:
        block = lua_block(SERVER_LUA)
        want = {e["id"]: e["badge"] for e in entries if isinstance(e.get("badge"), str)}
        if block is None:
            problems.append("lua/CustomSpells_Server.lua has no SPELL_BADGES block (run client_spell_sync.py --lua)")
        elif block != want:
            diff = sorted(k for k in set(block) | set(want) if block.get(k) != want.get(k))
            problems.append("SPELL_BADGES differs from the manifest for %s (run client_spell_sync.py --lua)"
                            % diff[:10])

    if problems:
        for p in problems:
            print("FAIL " + p)
        print("%d problem(s) in %d entries" % (len(problems), len(entries)))
        return 1
    print("OK: %d entries, %d distinct (icon, badge) pairs%s" % (
        len(entries), len(groups), "" if not args.no_dbc else " (by spell id, no DBC)"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
