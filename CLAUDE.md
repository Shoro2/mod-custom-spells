# CLAUDE.md - mod-custom-spells

> **Signpost only.** Cross-cutting documentation, code patterns, the proc system, the `spell_dbc` column reference and the step-by-step recipe for new spells live in [`share-public/docs/custom-spells/`](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells) and [`share-public/docs/03-spell-system.md`](https://github.com/Shoro2/share-public/blob/main/docs/03-spell-system.md). Read only what you need for the task.

## What is the module?

AzerothCore module that defines custom spell effects via C++ SpellScripts. Each custom spell has its own SpellScript class that hooks into the spell's DBC effects (e.g. `SCHOOL_DAMAGE`) and overrides damage / behavior. Pure DBC modifiers (e.g. `+50% damage`) skip C++ entirely.

## Files of this repo

[`INDEX.md`](./INDEX.md) · [`functions.md`](./functions.md) (how it works, pitfalls, APIs) ·
[`data_structure.md`](./data_structure.md) (file map) · [`log.md`](./log.md) · [`todo.md`](./todo.md) ·
[`CustomSpells.md`](./CustomSpells.md) (ID catalog) · [`tools/README.md`](./tools/README.md).

## Doc cross-refs (read these first)

| Looking for… | Read |
|----------------|----------------|
| Module overview, ID block scheme | [`share-public/docs/custom-spells/00-overview.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/00-overview.md) |
| Architecture (DBC + C++ + AIO + DB), three hook strategies | [`share-public/docs/custom-spells/01-architecture.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/01-architecture.md) |
| ID range of a class / next free ID | [`share-public/docs/custom-spells/02-id-blocks.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/02-id-blocks.md) |
| `spell_proc` setup, ProcFlags, `PROC_HIT_*` masks, off-by-one BasePoints | [`share-public/docs/custom-spells/03-procs-and-flags.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/03-procs-and-flags.md) |
| **Step-by-step recipe** for a new spell (Path A pure-DBC vs. Path B DBC + C++, all 4 patterns, `spell_dbc` insert example, registration, build, test, checklist) | [`share-public/docs/custom-spells/04-adding-a-spell.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/04-adding-a-spell.md) |
| Tricky patterns (recursion guards, target caps, ICDs, custom NPCs, `OnRemove` detection, channel/cast, owner→pet) | [`share-public/docs/custom-spells/05-complex-spells.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/05-complex-spells.md) |
| What does spell X do? Status? Implementation notes? | `share-public/docs/custom-spells/specs/<class>-<spec>.md` (e.g. [`warrior-arms`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warrior-arms.md)) |
| SpellScript / AuraScript hook lifecycle, proc full chain, **`spell_dbc` column reference** (257 cols), `EffectAura` values, `EffectSpellClassMask`, DBC override pipeline | [`share-public/docs/03-spell-system.md`](https://github.com/Shoro2/share-public/blob/main/docs/03-spell-system.md) |
| Cross-repo change log, plans, TODOs | [`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md) |
| ID schema + current allocation summary in this repo | [`./CustomSpells.md`](./CustomSpells.md) |
| Corrected ProcFlags reference (verified against `SpellMgr.h`) | [`./PROCFLAGS_REFERENCE.md`](./PROCFLAGS_REFERENCE.md) |

## Delivery: picker, spellbook, cursed items

The `/spells` picker whitelist (`lua/CustomSpells_Server.lua`) decides what a class may learn; every
spell in it has a spellbook name, tooltip and icon in `data/spellbook_enUS.json`
(`tools/client_spell_sync.py`), and every passive in it is also a cursed-item passive (mod-paragon-itemgen,
`tools/gen_class_passives.py`). Adding a spell = row(s) in the class SQL file + binding in
`mod_custom_spells.sql` + constant in `custom_spells_common.h` + picker entry + manifest entry + a bot
check in `tests/` + the two generators. The hunter's/warlock's second pet has its own AIO bar
(`lua/CustomSpells_PetBar_*.lua`, `src/custom_spells_second_pet.cpp`). Details: [`functions.md`](./functions.md).

## DBC status (quick overview)

> **Curated per-spec view** with status, source links, implementation notes for every spell ID: [`share-public/docs/custom-spells/specs/`](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells/specs).

Class blocks contain spells in `Spell.dbc` (including manual Warrior entries)
or the `spell_dbc` override table. Existence and catalog "implemented" labels do
not establish functional acceptance. The dated tooltip review records gaps
between the original concept, actual effects and picker delivery. Custom IDs
remain `900xxx/901xxx`; the CoA FL800xxx remap does not apply to this module.

Custom NPCs:
- `900333` — Frost Wyrm (DK Frost), AI script `npc_custom_frost_wyrm`, DisplayID 26752, 2× Gargoyle HP, casts Frost Breath
- `900436` — Spirit Wolf (Shaman Enhance proc summon, DisplayID 27074)
- `901066` — Healing Treant (Druid Resto HoT proc)

Next free IDs within each block are tracked in [`./CustomSpells.md`](./CustomSpells.md) (current allocation table).

## Build

The module is built automatically when placed in `azerothcore-wotlk/modules/mod-custom-spells/`. No separate build step:

```bash
cd azerothcore-wotlk/build
make -j$(nproc) && make install
```

## Config

`CustomSpells.Enable` (default: 1) in `mod_custom_spells.conf.dist` controls whether the module processes spell casts.

## Spec file status (post-implementation)

After a spell is implemented, update both:
- the row in this repo's [`CustomSpells.md`](./CustomSpells.md) "current allocation" table (set the spec status if it changed)
- the row + per-spell `Status` column in `share-public/docs/custom-spells/specs/<class>-<spec>.md`

After in-game testing, change the per-spell status from `implemented` to `tested`.

## Logging duty

Every change to this module (or any of the related repos) must be logged in [`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md). Format: ISO 8601 timestamp, repo, files, commit hash.
