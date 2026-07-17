# CLAUDE.md - mod-custom-spells

> **Signpost only.** Cross-cutting documentation, code patterns, the proc system, the `spell_dbc` column reference and the step-by-step recipe for new spells live in [`share-public/docs/custom-spells/`](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells) and [`share-public/docs/03-spell-system.md`](https://github.com/Shoro2/share-public/blob/main/docs/03-spell-system.md). Read only what you need for the task.

## What is the module?

AzerothCore module that defines custom spell effects via C++ SpellScripts. Each custom spell has its own SpellScript class that hooks into the spell's DBC effects (e.g. `SCHOOL_DAMAGE`) and overrides damage / behavior. Pure DBC modifiers (e.g. `+50% damage`) skip C++ entirely.

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

## Spell editor convention (numeric values)

When working on custom spells, keep in mind:
- Numeric values (damage, healing, absorption) are **real in-game values**, not internal DBC encodings. The editor converts to DBC format: `EffectBasePoints = real_value − 1` (off-by-one). Detail: [03-procs-and-flags.md#off-by-one-basepoints](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/03-procs-and-flags.md#off-by-one-basepoints).
- WotLK balancing: low-level 30–150, mid 200–600, high 800–2500, boss 3000–10000+.
- Spells have at most 3 effects (Effect1/2/3 in `Spell.dbc`).
- Periodic effects: `AmplitudeSeconds` for the tick interval, `DurationSeconds` for the total duration.
- Summon spells must have `DurationSeconds` set (determines summon lifetime).
- Tooltip tokens: `$d` (duration), `$s1` (BasePoints effect 1), etc.
- Icon: passed as a semantic hint (e.g. "frost", "fiery melee strike") — the editor resolves via fuzzy match against `SpellIcon.dbc`.

## Module structure

```
mod-custom-spells/
├── src/
│   ├── mod_custom_spells_loader.cpp  # Module entry point, registers scripts
│   ├── custom_spells_common.h        # Shared header: enum, constants, includes
│   ├── custom_spells.cpp             # Main: calls per-class registration functions
│   ├── custom_spells_warrior.cpp     # Warrior (Arms / Fury / Prot)
│   ├── custom_spells_paladin.cpp     # Paladin (Holy / Prot / Ret)
│   ├── custom_spells_dk.cpp          # Death Knight (Blood / Frost / Unholy)
│   ├── custom_spells_shaman.cpp      # Shaman (Ele / Enhance / Resto)
│   ├── custom_spells_hunter.cpp      # Hunter (BM / MM / Surv)
│   ├── custom_spells_rogue.cpp       # Rogue (Assa / Combat / Sub)
│   └── custom_spells_druid.cpp       # Druid (Balance / Feral / Resto)
├── conf/
│   └── mod_custom_spells.conf.dist   # Config: CustomSpells.Enable
├── lua/
│   ├── CustomSpells_Server.lua       # AIO spell picker (server): per-class whitelist + learn/forget
│   └── CustomSpells_Client.lua       # AIO spell picker (client): /spells toggle UI
└── data/sql/db-world/
    └── mod_custom_spells.sql          # spell_script_names, spell_dbc, spell_proc
```

The Lua pair deploys to `dcore/lua_scripts/CustomSpells/` (Eluna/ALE + AIO required). The
server file's `CLASS_SPELLS` whitelist is the delivery source of truth — update it when
spells are added/retired, and keep helpers/orphaned markers out of it.

## DBC status (quick overview)

> **Curated per-spec view** with status, source links, implementation notes for every spell ID: [`share-public/docs/custom-spells/specs/`](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells/specs).

All class blocks are populated. Spell IDs `900100-901108` exist in `Spell.dbc` (manual entries for Warrior Fury) or in the `spell_dbc` override table; all are implemented.

Custom NPCs:
- `900333` — Frost Wyrm (DK Frost), AI script `npc_custom_frost_wyrm`, DisplayID 26752, 2× Gargoyle HP, casts Frost Breath
- `900436` — Spirit Wolf (Shaman Enhance proc summon, DisplayID 27074)
- `901066` — Healing Treant (Druid Resto HoT proc)

Next free IDs within each block are tracked in [`./CustomSpells.md`](./CustomSpells.md) (current allocation table).

## Two paths for custom spells (decision aid)

```
                 ┌──────────────────────────┐
                 │  Plan a new custom spell  │
                 └────────────┬─────────────┘
                              │
                 ┌────────────▼─────────────┐
                 │  Does the spell need C++? │
                 └──┬───────────────────┬───┘
                    │                   │
               No   │                   │ Yes
                    │                   │
        ┌───────────▼──────┐  ┌─────────▼──────────┐
        │  Path A: DBC only│  │  Path B: DBC + C++ │
        │  (spell_dbc SQL) │  │  (DBC + SpellScript)│
        └───────────┬──────┘  └─────────┬──────────┘
                    │                   │
                    └─────────┬─────────┘
                              │
                 ┌────────────▼─────────────┐
                 │  Patch the client DBC    │
                 │  (Spell.dbc for tooltips) │
                 └────────────┬─────────────┘
                              │
                 ┌────────────▼─────────────┐
                 │  Build the server + test │
                 └──────────────────────────┘
```

| Effect type | Path |
|------------|------|
| Damage ±X %, Cooldown ±X s, Cast time ±X %, unlimited targets, passive stat modifiers, SpellFamilyMask-based buffs | **Path A** (pure DBC) |
| Conditional procs, multi-spell triggers, single→AoE conversion, block/dodge/parry procs, custom damage formulas, runtime cooldown manipulation | **Path B** (DBC + C++ SpellScript / AuraScript) |

Full step-by-step recipe (with the 4 SpellScript patterns A/B/C/D, the `spell_dbc` insert example, `spell_script_names` registration, build & test, and a 12-item checklist) is in [`share-public/docs/custom-spells/04-adding-a-spell.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/04-adding-a-spell.md).

## Key APIs (SpellScript)

- `GetCaster()` / `GetHitUnit()` — caster and target units
- `SetHitDamage(amount)` / `GetHitDamage()` — override / read effect damage
- `GetSpellInfo()` — `SpellInfo` of the spell being cast
- `GetCaster()->ToPlayer()` — cast to `Player` for player-specific APIs
- `player->GetTotalAttackPowerValue(BASE_ATTACK)` — melee AP
- `player->GetAuraCount(auraId)` — aura stack count
- `player->ModifySpellCooldown(spellId, deltaMs)` — adjust cooldown (negative = reduce)
- `player->RemoveSpellCooldown(spellId, true)` — clear cooldown (with client update)
- `LOG_INFO("module", "format {}", args)` — logging
- `RegisterSpellScript(ClassName)` — register in `AddCustomSpellsScripts()`

When hooking on existing Blizzard spells via `spell_script_names`, the C++ class runs on **every** cast of that spell. Always check `HasAura(<marker_aura>)` and `sConfigMgr->GetOption<bool>("CustomSpells.Enable", true)` so the effect is only active when the player has the passive and the module is enabled.

## SpellFamilyName values

| Value | Class | Value | Class |
|------:|-------|------:|-------|
| 0 | Generic | 8 | Rogue |
| 3 | Mage | 9 | Hunter |
| 4 | Warrior | 10 | Paladin |
| 5 | Warlock | 11 | Shaman |
| 6 | Priest | 15 | Death Knight |
| 7 | Druid | | |

## Common pitfalls

1. **SpellFamilyFlags wrong**: ALWAYS verify against the project's own `Spell.dbc` (LOG_INFO debug pattern in [`03-spell-system.md`](https://github.com/Shoro2/share-public/blob/main/docs/03-spell-system.md#critical-always-verify-spellfamilyflags-via-debug-log)), never against online DBs (wowhead, wowdb).
2. **`MaxAffectedTargets=0` set globally**: this affects ALL players, not only those with the passive — for conditional targets use C++.
3. **Proc loop**: helper spells can re-proc → set ICD in `spell_proc` (`Cooldown` field) and / or check `SPELL_ATTR3_CAN_PROC_FROM_PROCS`.
4. **`spell_script_names` missing**: the C++ class is not loaded → spell has no effect.
5. **`DurationIndex` forgotten**: a passive aura needs `DurationIndex=21` (permanent).
6. **Attributes missing PASSIVE**: without `0x40` the spell is castable instead of permanently active.
7. **Off-by-one BasePoints**: writing `BasePoints=50` for "+50 %" yields **+51 %** in-game. Store `49`, not `50`. Detail: [`03-procs-and-flags.md#off-by-one-basepoints`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/03-procs-and-flags.md#off-by-one-basepoints). (Only applies with `EffectDieSides=1`; with `EffectDieSides=0` the value is used as-is.)
8. **`EffectSpellClassMaskA/B/C` are PER-EFFECT flag96 masks**: `A_1..A_3` = the three flag words of **effect 1**, `B_*` = effect 2, `C_*` = effect 3. A spellmod whose target spell has its family bit in `SpellFamilyFlags[1]` needs the mask in **`EffectSpellClassMaskA_2`** — writing it into `B_1` gives effect 1 an empty mask and the modifier silently affects nothing. Always verify the mask against the server `Spell.dbc` `SpellClassMask_1..3` of the target spell (2026-07-18 repair wave fixed 25+ rows with this bug).
9. **`EffectMiscValue` on aura 107/108 is the SpellModOp**: 0=DAMAGE, 1=DURATION, 4=CHARGES, 10=CASTING_TIME, 11=COOLDOWN, 14=COST, 17=JUMP_TARGETS, 22=DOT. "Instant"/"-X% cast" is op 10 (not 14), "duration" is op 1 and "double HoTs/DoTs" is op 22 (not 17).
10. **Extra-target damage must be dealt directly, not cast via helper spells**: `CastCustomSpell(target, HELPER, &damage, ...)` with a server-only helper produced no visible damage in-game. Use the T2-proven pattern instead: build `SpellNonMeleeDamage` with the ORIGINAL spell's `SpellInfo`, then `DealSpellDamage` + `SendSpellNonMeleeDamageLog` (heals: `HealInfo` + `HealBySpell`). The client renders the known spell id; no client DBC entry needed.
11. **`Attributes` 0x10000000 = SPELL_ATTR0_NOT_IN_COMBAT_ONLY_PEACEFUL** ("cannot be used in combat") — never put it on castable actives or helpers. Triggered casts bypass it, player casts do not.

## Build

The module is built automatically when placed in `azerothcore-wotlk/modules/mod-custom-spells/`. No separate build step:

```bash
cd azerothcore-wotlk/build
make -j$(nproc) && make install
```

## Code style

AzerothCore conventions:
- 4-space indentation, no tabs
- `Type const*` (not `const Type*`)
- `UPPER_SNAKE_CASE` for spell/NPC constants with prefix `SPELL_CUSTOM_*`
- `UpperCamelCase` for class/method names
- No braces around single-line if/else/for/while

CI runs `apps/ci/ci-codestyle.sh` which rejects: trailing whitespace, tabs, multiple consecutive blank lines, and `LOG_*` calls that use `ObjectGuid::GetCounter()` (use `ObjectGuid::ToString().c_str()` instead).

## Config

`CustomSpells.Enable` (default: 1) in `mod_custom_spells.conf.dist` controls whether the module processes spell casts.

## Loader convention

The loader function in `mod_custom_spells_loader.cpp` must be named `Addmod_custom_spellsScripts()` — module folder name with `-` replaced by `_`.

## Spec file status (post-implementation)

After a spell is implemented, update both:
- the row in this repo's [`CustomSpells.md`](./CustomSpells.md) "current allocation" table (set the spec status if it changed)
- the row + per-spell `Status` column in `share-public/docs/custom-spells/specs/<class>-<spec>.md`

After in-game testing, change the per-spell status from `implemented` to `tested`.

## Logging duty

Every change to this module (or any of the related repos) must be logged in [`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md). Format: ISO 8601 timestamp, repo, files, commit hash.
