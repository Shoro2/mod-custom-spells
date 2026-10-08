# Tools

## `client_spell_sync.py` - spellbook names, tooltips and icons

The client draws a custom spell's name, tooltip, icon and aura from **its own**
`Spell.dbc` record; the server reads `acore_world.spell_dbc`. FL ships the client
file in two carriers, and both go into the combined `patch-9.MPQ`:

| Carrier | Path |
|---|---|
| `hot_dbc` | `C:\wowstuff\ForgottenLand2.0\output\hot_dbc\DBFilesClient\Spell.dbc` |
| `overlay` | `F:\wowstuff\coa-program\pack\overlay\DBFilesClient\Spell.dbc` (shadows `hot_dbc`) |

`data/spellbook_enUS.json` is the source of the texts: one entry per picker spell
(`id`, `class`, `spec`, `name`, `description`, optional `icon_spell` = the stock spell
whose icon the record shows). Each description states the behaviour after the
2026-10-08 rework. It replaces the 2026-10-04 review draft `data/tooltips_enUS.json`
and its tool `patch_tooltips.py` (both in git history).

```powershell
python -B tools/client_spell_sync.py            # check: what would change, writes nothing
python -B tools/client_spell_sync.py --write    # back up both carriers, write them in place
python -B tools/client_spell_sync.py --lua      # picker labels := manifest names
```

Per carrier, `--write`:

1. copies every non-string field of the server rows 900000-901199 into the record of
   the same id (floats as floats, string offsets and locale masks untouched);
2. appends every server-only record except the dead helpers in `SKIP_IDS`;
3. sets `Name`, `Description` and `SpellIconID` of every manifest spell.

It refuses a carrier with dangling string offsets or a string block that does not
start with NUL, reuses identical strings (a rerun adds nothing), proves that no
record outside the custom block changed and that the old string block is a prefix of
the new one, and backs each carrier up to
`F:\Backups\fl-restore-backups\{hot_dbc,overlay}\Spell.dbc.pre_<tag>_<date>_<sha8>`.

Then build the client patch with the workspace's
`scripts\30_build_hot_dbc_patch.py` (FL-only archive), then `--deploy` with the client
closed. The host gets the new `patch-9.MPQ` through the launcher (a `client-patch`
entry in share-public `forgotten-land/15-host-migration-log.md`).

The server's own `spell_dbc` names are not shown to players; they stay as they are.
