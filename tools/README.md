# Offline tooltip candidates

`data/tooltips_enUS.json` is a **review draft**, not a release declaration.
It covers the empty picker descriptions and records concept/source conflicts
in each entry's `review_note`. Do not silently turn a source discrepancy into
the accepted design by publishing its current-behavior text.

`patch_tooltips.py` reads the current core `spell_dbc.sql` schema and a complete
Spell.dbc carrier, then writes a separate candidate plus JSON report. It has
no database access, SQL execution, MPQ writer or deployment capability.

```powershell
python -B tools/patch_tooltips.py --input <Spell.dbc> --output <candidate.dbc> --schema <core/data/sql/base/db_world/spell_dbc.sql>
```

For the observed server carrier alone, `--allow-missing 900902` declares the
existing DB-only Weakened Soul marker. Allowances must match missing IDs
exactly. Client carriers must contain every manifest ID. No sparse DB rows
are created for DBC-only Warrior spells.

The patch changes only `Description_Lang_enUS` string offsets and corresponding
locale flags. It appends/reuses UTF-8 strings; IDs, record order, gameplay
dwords, other strings/locales and the original string block are preserved.
Repeated application is byte-idempotent. The dated vault review links the
actual Windows carrier verification reports and pending native tests.

To make the descriptions durable in the DB, add reviewed strings to the
existing per-class module INSERTs, preserving every old value. A separate
hash-tracked UPDATE can be lost when a later changed base INSERT is reapplied.
The CoA overlay shadows hot staging in the combined client patch, so prepare
both complete candidates and use the established script30 deployment path
after the coordinator grants the exact data window. Never patch MPQ manually.
