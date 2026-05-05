# mod-custom-spells

Custom spell library for an [AzerothCore](https://www.azerothcore.org/) **WoW 3.3.5a (WotLK)** server.

## What it does

Defines new spell effects and modifiers via DBC overrides plus C++ `SpellScript` / `AuraScript` classes. The catalog covers all 10 player classes, with custom IDs grouped per class and per spec:

| Class | ID block |
|-------|----------|
| Warrior | 900100-900199 |
| Paladin | 900200-900299 |
| Death Knight | 900300-900399 |
| Shaman | 900400-900499 |
| Hunter | 900500-900599 |
| Rogue | 900600-900699 |
| Mage | 900700-900799 |
| Warlock | 900800-900899 |
| Priest | 900900-900999 |
| Druid | 901000-901099 |
| Non-class (global) | 901100-901199 |

Effects range from simple DBC modifiers (e.g. *"Mortal Strike +50 % damage"*, *"Bloodthirst cooldown −2 s"*) to custom C++ logic (recursion-guarded procs, multi-spell triggers, single-target → AoE conversions, custom damage formulas, on-block / on-dodge / on-parry effects, summon overrides such as *"Ghoul → Frost Wyrm"*). Three custom NPCs ship with the module (Frost Wyrm, Spirit Wolf, Healing Treant).

Full ID-by-ID catalog with effect descriptions, status, and implementation notes: [`CustomSpells.md`](./CustomSpells.md) and per-spec files in [`share-public/docs/custom-spells/specs/`](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells/specs).

## Key features

- **Two implementation paths**:
  - *Pure DBC* — passive modifiers via `SPELL_AURA_ADD_PCT_MODIFIER` / `ADD_FLAT_MODIFIER`, no C++ needed
  - *DBC + C++* — `SpellScript` / `AuraScript` for conditional procs, AoE conversions, runtime cooldown manipulation, custom damage formulas
- **Proc system** integration via `spell_proc` with corrected `ProcFlags` reference (see [`PROCFLAGS_REFERENCE.md`](./PROCFLAGS_REFERENCE.md))
- **Custom NPCs** with their own AI scripts (`npc_custom_frost_wyrm`)
- **Off-by-one BasePoints** documented (`Spell.dbc` stores `real_value − 1`)
- Per-class C++ source files (`custom_spells_warrior.cpp`, `…_paladin.cpp`, …) for clean separation
- Comprehensive cross-cutting docs in [share-public](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells) covering architecture, ID blocks, proc flags, the step-by-step recipe for new spells, and complex-spell patterns (recursion guards, target caps, ICDs, owner→pet aura transfer, …)

## Installation

1. Place this module inside the AzerothCore `modules/` directory:
   ```bash
   cd azerothcore-wotlk/modules
   git clone https://github.com/Shoro2/mod-custom-spells.git
   ```
2. Re-run CMake and build the server:
   ```bash
   cd ../build
   cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/azeroth-server \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DSCRIPTS=static -DMODULES=static
   make -j$(nproc) && make install
   ```
3. Apply the SQL files shipped under `data/sql/db-world/` (the AzerothCore SQL updater picks them up automatically). The world DB import populates `spell_dbc`, `spell_proc`, `spell_script_names`, and `creature_template` for the custom NPCs.
4. Copy the config and adjust if needed:
   ```bash
   cp $HOME/azeroth-server/etc/mod_custom_spells.conf.dist $HOME/azeroth-server/etc/mod_custom_spells.conf
   ```
5. (Optional) Patch the client `Spell.dbc` so tooltips show the custom spell names. Use `share-public/python_scripts/copy_spells_dbc.py` and `patch_dbc.py`. Marker auras and helper spells need no client patch.
6. Restart the world server.

## Configuration

`conf/mod_custom_spells.conf.dist`:

- `CustomSpells.Enable` (default `1`) — master toggle. When `0`, no SpellScript / AuraScript hook does any work.

## Adding a new custom spell

The full step-by-step recipe (DBC entry → SQL → C++ → build → test) is in [`share-public/docs/custom-spells/04-adding-a-spell.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/04-adding-a-spell.md). Quick decision aid:

| Effect | Path |
|--------|------|
| Damage ±X %, Cooldown ±X s, Cast time ±X %, unlimited targets, passive modifiers | Pure DBC |
| Conditional procs, multi-spell triggers, single→AoE conversion, block/dodge/parry, custom damage formulas | DBC + C++ |

After implementation, log the change in [`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md).

## Requirements

- [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk) (WoW 3.3.5a / WotLK)

## Project context

Part of a multi-repo project. Cross-cutting documentation, the proc flags reference (verified against `SpellMgr.h`), and the curated per-spec catalog live in [share-public](https://github.com/Shoro2/share-public).

## License

GPL v2 (see `LICENSE`).
