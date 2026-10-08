"""Sync the custom class spells into the client Spell.dbc carriers.

The client draws a spell's name, tooltip, icon, aura bar entry and cast gating
from ITS OWN Spell.dbc record; the server reads acore_world.spell_dbc. FL ships
the client file twice (both go into the combined patch-9, the overlay shadows
hot_dbc, scripts/30_build_hot_dbc_patch.py refuses when they disagree):

    hot_dbc  C:\\wowstuff\\ForgottenLand2.0\\output\\hot_dbc\\DBFilesClient\\Spell.dbc
    overlay  F:\\wowstuff\\coa-program\\pack\\overlay\\DBFilesClient\\Spell.dbc

Per carrier this tool
  1. copies every NON-STRING field of the server rows 900000-901199 into the
     carrier's record of the same id (string offsets and their locale masks
     stay; floats are written as floats),
  2. appends every server record the carrier lacks except SKIP_IDS (name from
     the server row, every other string empty),
  3. sets Name, Description and SpellIconID of every picker spell from
     data/spellbook_enUS.json (icon_spell = the stock spell whose icon the
     record shows), and the buff tooltip (AuraDescription) of the helper
     buffs listed under its "auras".
Strings are appended to the string block (an identical existing string is
reused, so a rerun adds nothing); the old bytes stay as dead data. It refuses
a carrier with dangling string offsets (an append would turn them into text,
share-public memory "spell-dbc-dangling-string-offsets") or a block that does
not start with NUL (offset 0 must read as "").

    python tools/client_spell_sync.py            --check (default): report, write nothing
    python tools/client_spell_sync.py --write    back up each carrier, write in place, re-read
    python tools/client_spell_sync.py --lua      picker labels in lua/CustomSpells_Server.lua
                                                 := the manifest names

Then build the client patch: scripts\\30_build_hot_dbc_patch.py, then --deploy
with the client closed (share-public forgotten-land MIG entry for the host).
The server rows come from the workbench DB (credentials from worldserver.conf,
passed in MYSQL_PWD, never on the command line); --server-tsv reads a saved
`mysql -N -B` dump instead (not --raw: a newline in a text column would split its row).
"""
import argparse
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
MODULE = os.path.dirname(HERE)
MANIFEST = os.path.join(MODULE, "data", "spellbook_enUS.json")
PICKER_LUA = os.path.join(MODULE, "lua", "CustomSpells_Server.lua")
CORE = os.path.dirname(os.path.dirname(MODULE))
DDL = os.path.join(CORE, "data", "sql", "base", "db_world", "spell_dbc.sql")
CARRIERS = [("hot_dbc", r"C:\wowstuff\ForgottenLand2.0\output\hot_dbc\DBFilesClient\Spell.dbc"),
            ("overlay", r"F:\wowstuff\coa-program\pack\overlay\DBFilesClient\Spell.dbc")]
BACKUP_DIR = r"F:\Backups\fl-restore-backups"
MYSQL = r"C:\Program Files\MySQL\MySQL Server 8.4\bin\mysql.exe"
WORLD_CONF = r"C:\wowstuff\dcore\configs\worldserver.conf"

ID_LO, ID_HI = 900000, 901199
# server rows the client does NOT get: dead helpers no script casts any more
# (every other server-only row is appended, so new picker spells, visible
# buffs and damage helpers get a client record: spellbook, aura bar, combat log)
SKIP_IDS = {900176, 900276, 900305, 900506}
# the record whose locale-mask dwords appended strings copy
MASK_TEMPLATE_ID = 900275
FIELDS, RECSIZE = 234, 936


def ddl_columns():
    text = open(DDL, encoding="utf-8").read()
    cols = re.findall(r"^\s*`(\w+)`\s+(\w+)", text, re.M)
    cols = [(c, t.lower()) for c, t in cols if c != "spell_dbc"]
    if len(cols) != FIELDS or cols[0][0] != "ID":
        raise SystemExit("spell_dbc DDL: expected %d columns, found %d" % (FIELDS, len(cols)))
    return [c for c, _t in cols], {i for i, (_c, t) in enumerate(cols) if t == "float"}


COLS, FLOAT_COLS = ddl_columns()
CI = {c: i for i, c in enumerate(COLS)}
STRING_COLS = {i for i, c in enumerate(COLS) if "_Lang_" in c}           # offsets AND masks
TEXT_COLS = [i for i, c in enumerate(COLS) if "_Lang_" in c and not c.endswith("_Mask")]


def load_auras():
    """Buff tooltips (AuraDescription) of the helpers a player sees in the aura bar."""
    m = json.load(open(MANIFEST, encoding="utf-8"))
    out = {}
    for e in m.get("auras", []):
        sid, text = e["id"], e["aura_description"]
        if not ID_LO <= sid <= ID_HI or sid in out or not text.strip() or "\0" in text:
            raise SystemExit("manifest auras: bad entry %r" % sid)
        out[sid] = text
    return out


def load_manifest():
    m = json.load(open(MANIFEST, encoding="utf-8"))
    if m.get("schema_version") != 2 or m.get("locale") != "enUS":
        raise SystemExit("%s: unsupported manifest" % MANIFEST)
    out = {}
    for e in m["entries"]:
        sid = e["id"]
        if not ID_LO <= sid <= ID_HI or sid in out:
            raise SystemExit("manifest: bad or duplicate id %r" % sid)
        for key in ("name", "description"):
            if not isinstance(e.get(key), str) or not e[key].strip() or "\0" in e[key]:
                raise SystemExit("manifest: %d has no usable %s" % (sid, key))
        out[sid] = e
    return out


def server_rows(tsv=None):
    if tsv:
        text = open(tsv, encoding="utf-8").read()
    else:
        line = [ln for ln in open(WORLD_CONF, encoding="utf-8", errors="replace")
                if ln.startswith("WorldDatabaseInfo")][0]
        host, port, user, pw, db = line.split('"')[1].split(";")
        query = ("SELECT * FROM `%s`.`spell_dbc` WHERE `ID` BETWEEN %d AND %d ORDER BY `ID`;"
                 % (db, ID_LO, ID_HI))
        # batch output WITHOUT --raw: a tab or newline inside a text column comes back escaped,
        # so every row stays one line of FIELDS columns (900105's description has a newline;
        # with --raw the row was split and silently skipped). Only non-string fields are read.
        proc = subprocess.run([MYSQL, "-h" + host, "-P" + port, "-u" + user, "-N", "-B", "-e", query],
                              capture_output=True, text=True, encoding="utf-8",
                              env=dict(os.environ, MYSQL_PWD=pw))
        if proc.returncode != 0:
            raise SystemExit("mysql failed: %s" % proc.stderr.strip())
        text = proc.stdout
    rows = {}
    for line in text.splitlines():
        parts = line.split("\t")
        if len(parts) != FIELDS or not parts[0].isdigit():
            continue
        rows[int(parts[0])] = parts
    if not rows:
        raise SystemExit("no server rows %d-%d" % (ID_LO, ID_HI))
    return rows


def dword(col_index, text):
    """The DBC dword of one server value (float columns as IEEE float)."""
    if col_index in FLOAT_COLS:
        return struct.unpack("<I", struct.pack("<f", float(text)))[0]
    return int(text) & 0xFFFFFFFF


class Carrier(object):
    def __init__(self, label, path):
        self.label, self.path = label, path
        self.raw = open(path, "rb").read()
        magic, n, fields, size, slen = struct.unpack_from("<4sIIII", self.raw, 0)
        end = 20 + n * size
        if magic != b"WDBC" or fields != FIELDS or size != RECSIZE or len(self.raw) != end + slen:
            raise SystemExit("%s: not the 3.3.5a Spell.dbc layout" % path)
        self.count = n
        self.records = bytearray(self.raw[20:end])
        self.strings = bytearray(self.raw[end:])
        if not self.strings or self.strings[0] != 0 or self.strings[-1] != 0:
            raise SystemExit("%s: string block must start and end with NUL" % path)
        self.index = {}
        dangling = 0
        for r in range(n):
            vals = struct.unpack_from("<%dI" % FIELDS, self.records, r * size)
            self.index[vals[0]] = r
            dangling += sum(1 for i in TEXT_COLS if vals[i] >= slen)
        if dangling:
            raise SystemExit("%s: %d dangling string offsets - clear them first" % (path, dangling))
        self.known = {}
        start = 0
        while start < len(self.strings):
            stop = self.strings.index(0, start)
            self.known.setdefault(bytes(self.strings[start:stop]), start)
            start = stop + 1

    def get(self, sid, col):
        return struct.unpack_from("<I", self.records, self.index[sid] * RECSIZE + CI[col] * 4)[0]

    def put(self, sid, col_index, value):
        struct.pack_into("<I", self.records, self.index[sid] * RECSIZE + col_index * 4, value)

    def text(self, sid, col):
        off = self.get(sid, col)
        return bytes(self.strings[off:self.strings.index(0, off)]).decode("utf-8", "replace")

    def offset_of(self, text):
        data = text.encode("utf-8")
        off = self.known.get(data)
        if off is None:
            off = len(self.strings)
            self.strings.extend(data + b"\0")
            self.known[data] = off
        return off

    def build(self):
        return (struct.pack("<4sIIII", b"WDBC", self.count, FIELDS, RECSIZE, len(self.strings))
                + bytes(self.records) + bytes(self.strings))


def sync(carrier, srv, manifest):
    """Apply steps 1-3 to one carrier in memory; returns the change report."""
    report = {"synced": [], "appended": [], "texts": [], "icons": [], "server_only": []}
    mask_tpl = carrier.get(MASK_TEMPLATE_ID, "Name_Lang_Mask")

    icon_col = CI["SpellIconID"]
    for sid, vals in sorted(srv.items()):
        if sid not in carrier.index:
            continue
        changed = []
        for i in range(1, FIELDS):
            if i in STRING_COLS:
                continue
            if i == icon_col and manifest.get(sid, {}).get("icon_spell"):
                continue        # step 3 sets the manifest's icon
            new = dword(i, vals[i])
            if carrier.get(sid, COLS[i]) != new:
                carrier.put(sid, i, new)
                changed.append(COLS[i])
        if changed:
            report["synced"].append((sid, changed))

    for sid in sorted(set(srv) - set(carrier.index)):
        if sid in SKIP_IDS:
            report["server_only"].append(sid)
            continue
        vals = srv[sid]
        rec = bytearray(RECSIZE)
        for i in range(FIELDS):
            if i not in STRING_COLS:
                struct.pack_into("<I", rec, i * 4, dword(i, vals[i]))
        carrier.records.extend(rec)
        carrier.index[sid] = carrier.count
        carrier.count += 1
        carrier.put(sid, CI["Name_Lang_enUS"], carrier.offset_of(vals[CI["Name_Lang_enUS"]]))
        carrier.put(sid, CI["Name_Lang_Mask"], mask_tpl)
        report["appended"].append(sid)

    missing = sorted(set(manifest) - set(carrier.index))
    if missing:
        raise SystemExit("%s: manifest spells without a record: %s" % (carrier.label, missing))
    for sid, e in sorted(manifest.items()):
        for col, key in (("Name_Lang_enUS", "name"), ("Description_Lang_enUS", "description")):
            if carrier.text(sid, col) != e[key]:
                carrier.put(sid, CI[col], carrier.offset_of(e[key]))
                report["texts"].append((sid, col))
            mask_col = col.replace("_enUS", "_Mask")
            if carrier.get(sid, mask_col) == 0:
                carrier.put(sid, CI[mask_col], mask_tpl)
        if e.get("icon_spell"):
            src = e["icon_spell"]
            if src not in carrier.index:
                raise SystemExit("%s: icon_spell %d of %d has no record" % (carrier.label, src, sid))
            icon = carrier.get(src, "SpellIconID")
            if carrier.get(sid, "SpellIconID") != icon:
                carrier.put(sid, CI["SpellIconID"], icon)
                report["icons"].append((sid, icon))
    for sid, text in sorted(load_auras().items()):
        if sid not in carrier.index:
            raise SystemExit("%s: aura text for %d, which has no record" % (carrier.label, sid))
        if carrier.text(sid, "AuraDescription_Lang_enUS") != text:
            carrier.put(sid, CI["AuraDescription_Lang_enUS"], carrier.offset_of(text))
            carrier.put(sid, CI["AuraDescription_Lang_Mask"], mask_tpl)
            report["texts"].append((sid, "AuraDescription_Lang_enUS"))
    return report


def verify(before, after, srv, manifest):
    """Nothing outside the custom block may change; the old string block stays a prefix."""
    b_magic, b_n, _f, _s, b_slen = struct.unpack_from("<4sIIII", before, 0)
    a_magic, a_n, _f, _s, _a_slen = struct.unpack_from("<4sIIII", after, 0)
    b_end, a_end = 20 + b_n * RECSIZE, 20 + a_n * RECSIZE
    if a_n < b_n or not after[a_end:].startswith(before[b_end:]):
        raise SystemExit("verify: record count shrank or the old string block changed")
    allowed_text = {CI["Name_Lang_enUS"], CI["Description_Lang_enUS"],
                    CI["Name_Lang_Mask"], CI["Description_Lang_Mask"]}
    aura_text = {CI["AuraDescription_Lang_enUS"], CI["AuraDescription_Lang_Mask"]}
    auras = load_auras()
    for r in range(b_n):
        old = struct.unpack_from("<%dI" % FIELDS, before, 20 + r * RECSIZE)
        new = struct.unpack_from("<%dI" % FIELDS, after, 20 + r * RECSIZE)
        if old == new:
            continue
        sid = old[0]
        if new[0] != sid or not ID_LO <= sid <= ID_HI:
            raise SystemExit("verify: record %d (id %d) outside the custom block changed" % (r, sid))
        for i in range(FIELDS):
            if old[i] == new[i]:
                continue
            ok = ((i not in STRING_COLS and sid in srv)
                  or (sid in manifest and (i in allowed_text or COLS[i] == "SpellIconID"))
                  or (sid in auras and i in aura_text))
            if not ok:
                raise SystemExit("verify: %d %s changed unexpectedly" % (sid, COLS[i]))


def sha(data):
    return hashlib.sha256(data).hexdigest()


def write_carrier(carrier, before, after, tag):
    bak_dir = os.path.join(BACKUP_DIR, carrier.label)
    os.makedirs(bak_dir, exist_ok=True)
    bak = os.path.join(bak_dir, "Spell.dbc.pre_%s_%s_%s" % (tag, time.strftime("%Y%m%d"), sha(before)[:8]))
    if not os.path.exists(bak):
        shutil.copy2(carrier.path, bak)
    fd, tmp = tempfile.mkstemp(dir=os.path.dirname(carrier.path), suffix=".tmp")
    with os.fdopen(fd, "wb") as f:
        f.write(after)
    os.replace(tmp, carrier.path)
    if open(carrier.path, "rb").read() != after:
        raise SystemExit("%s: read-back differs" % carrier.path)
    return bak


def update_lua(manifest):
    text = open(PICKER_LUA, encoding="utf-8").read()
    changed = 0

    def repl(m):
        nonlocal changed
        sid = int(m.group(2))
        if sid not in manifest:
            return m.group(0)
        name = manifest[sid]["name"].replace("\\", "\\\\").replace('"', '\\"')
        if name != m.group(3):
            changed += 1
        return '%s{ %d, "%s", "%s" }' % (m.group(1), sid, name, m.group(4))

    new = re.sub(r'(\s)\{ (\d+), "((?:[^"\\]|\\.)*)", "([^"]*)" \}', repl, text)
    listed = {int(x) for x in re.findall(r'\{ (\d+), "', new)}
    if listed != set(manifest):
        raise SystemExit("picker list and manifest differ: %s / %s"
                         % (sorted(listed - set(manifest)), sorted(set(manifest) - listed)))
    if new != text:
        with open(PICKER_LUA, "w", encoding="utf-8", newline="\n") as f:
            f.write(new)
    print("picker labels: %d changed" % changed)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--write", action="store_true", help="back up and write the carriers in place")
    ap.add_argument("--lua", action="store_true", help="only rewrite the picker labels")
    ap.add_argument("--carrier", action="append", metavar="LABEL=PATH",
                    help="a carrier instead of the default pair (repeatable)")
    ap.add_argument("--server-tsv", help="server rows from a saved mysql -N --raw dump")
    ap.add_argument("--tag", default="csrework", help="backup name tag (default csrework)")
    args = ap.parse_args()

    manifest = load_manifest()
    if args.lua:
        update_lua(manifest)
        return 0
    carriers = CARRIERS
    if args.carrier:
        carriers = [tuple(c.split("=", 1)) for c in args.carrier]
    srv = server_rows(args.server_tsv)
    print("server rows %d-%d: %d, manifest spells: %d" % (ID_LO, ID_HI, len(srv), len(manifest)))
    outputs = []
    for label, path in carriers:
        carrier = Carrier(label, path)
        before = carrier.raw
        report = sync(carrier, srv, manifest)
        after = carrier.build()
        verify(before, after, srv, manifest)
        print("== %s %s" % (label, path))
        print("   records %d -> %d, string block +%d bytes, sha %s -> %s"
              % (len(carrier.index) - len(report["appended"]), carrier.count,
                 len(after) - len(before) - len(report["appended"]) * RECSIZE,
                 sha(before)[:12], sha(after)[:12]))
        print("   synced %d records: %s" % (len(report["synced"]),
                                             " ".join(str(s) for s, _c in report["synced"])))
        print("   appended %s; server-only, not appended %s" % (report["appended"], report["server_only"]))
        print("   texts set %d (%d spells), icons set %d"
              % (len(report["texts"]), len({s for s, _c in report["texts"]}), len(report["icons"])))
        outputs.append((carrier, before, after))
    if not args.write:
        print("check only - nothing written (--write to apply)")
        return 0
    for carrier, before, after in outputs:
        if before == after:
            print("%s: unchanged" % carrier.label)
            continue
        bak = write_carrier(carrier, before, after, args.tag)
        print("%s: written (backup %s)" % (carrier.label, bak))
    return 0


if __name__ == "__main__":
    sys.exit(main())
