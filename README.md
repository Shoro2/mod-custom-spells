# mod-custom-spells

Custom spell library for an [AzerothCore](https://www.azerothcore.org/) **WoW 3.3.5a (WotLK)** server.

The module defines new class spell effects and modifiers through `spell_dbc` overrides plus C++ `SpellScript` / `AuraScript` classes, and ships an in-game picker (`/spells`) where players choose which of the custom spells their class learns. Part of the Forgotten Land server project.

## Features

- Custom spells for all 10 player classes, with custom IDs grouped per class:

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

- Effects range from pure DBC modifiers (e.g. *"Mortal Strike +50 % damage"*, *"Bloodthirst cooldown -2 s"*) to custom C++ logic: recursion-guarded procs, multi-spell triggers, single-target to AoE conversions, custom damage formulas, on-block / on-dodge / on-parry effects, summon overrides (*"Ghoul -> Frost Wyrm"*).
- Custom NPCs with their own scripts, e.g. Frost Wyrm (900333), Spirit Wolf (900436), Healing Treant (901066), and a second pet for hunters and warlocks (Beast Companion, 900525) with its own pet bar.
- Per-class C++ source files (`custom_spells_warrior.cpp`, `..._paladin.cpp`, ...).
- In-game spell picker (`/spells` or `/cs`) built on AIO: players toggle which custom spells they know; the server validates every request against a per-class whitelist.
- A weaker recast of your own buff asks first: when a buff of yours on the target (Thorns, a mark, a blessing, a lower rank of the same buff, ...) is stronger than the one you are casting, the cast is held back and a popup asks whether to replace it. Needs the core hook `PlayerScript::OnPlayerCanReplaceStrongerOwnBuff` of the FL AzerothCore fork.

## Requirements

- AzerothCore WotLK, **the [Shoro2/azerothcore-wotlk](https://github.com/Shoro2/azerothcore-wotlk) fork**. Some scripts use script hooks (for example `OnSpellBeforeEffects`, `OnSpellHitResult`) and a spell-modifier type (`SPELLMOD_MAX_AURA_STACKS`) that upstream AzerothCore does not have; on upstream the module does not build or those effects stay inactive.
- For the picker and pet bar: a Lua engine module ([mod-ale](https://github.com/azerothcore/mod-ale), Eluna-compatible) and [AIO](https://github.com/Rochet2/AIO). The C++ spell effects work without them.
- Optional: the GM-only "Learn all Talents" button of the picker calls the `.forgotten learnall` command of mod-forgotten-talents when that module is present.
- Optional client patch: the client draws names, tooltips and icons from its own `Spell.dbc`, so custom spells need records there to show proper text. `data/spellbook_enUS.json` holds name, tooltip and icon per picker spell, and `tools/client_spell_sync.py` writes them into a client `Spell.dbc`. Marker auras and helper spells need no client entry.

## Installation

1. Clone the module into the `modules/` directory of the server source:
   ```bash
   cd azerothcore-wotlk/modules
   git clone https://github.com/Shoro2/mod-custom-spells.git
   ```
2. Re-run CMake and rebuild the server (the module is picked up automatically).
3. SQL: the files in `data/sql/db-world/` are applied by the AzerothCore DB updater on the next world server start. They populate `spell_dbc`, `spell_proc`, `spell_script_names` and `creature_template` for the custom NPCs.
4. Copy `conf/mod_custom_spells.conf.dist` to your server's module config folder as `mod_custom_spells.conf`.
5. Picker, pet bar and the buff popup: copy the files in `lua/` into the server's `lua_scripts/` folder (the `*_Server.lua` files run on the server, the `*_Client.lua` files are sent to clients by AIO).
6. Optional: patch the client `Spell.dbc` as described above.
7. Restart the world server.

## Configuration

`conf/mod_custom_spells.conf.dist`:

- `CustomSpells.Enable` (default `1`) - master toggle. When `0`, no custom spell script does any work.
- `CustomSpells.BuffReplaceConfirm.Enable` (default `1`) - ask before a weaker recast replaces your own stronger buff. When `0`, such a cast always replaces.

## Usage

- `/spells` or `/cs` (client chat) - opens the spell picker.
- `.cspet attack | follow | stay | aggressive | defensive | passive | cast <spell> | autocast | status | sync` - controls the hunter/warlock second pet; normally sent by the pet bar, available to players for their own second pet.
- `.csbuff yes | no` - the buff popup's answers (yes casts the held-back buff again and replaces the old one).

## Adding a new custom spell

Pick the cheapest path that does the job:

| Effect | Path |
|--------|------|
| Damage +-X %, cooldown +-X s, cast time +-X %, unlimited targets, passive modifiers | Pure DBC: a `spell_dbc` row with `SPELL_AURA_ADD_PCT_MODIFIER` / `ADD_FLAT_MODIFIER`, no C++ |
| Conditional procs, multi-spell triggers, single-target to AoE, block/dodge/parry hooks, custom damage formulas | `spell_dbc` row plus a `SpellScript` / `AuraScript`, registered through `spell_script_names` |

Steps: add the `spell_dbc` row (and a `spell_proc` row for procs) to the class SQL file in `data/sql/db-world/`, add the spell ID constant to `src/custom_spells_common.h`, implement and register the script in the class's `custom_spells_<class>.cpp`, add the picker entry in `lua/CustomSpells_Server.lua` and the name/tooltip in `data/spellbook_enUS.json`, then rebuild.

Pitfalls worth knowing:

- `EffectBasePoints` in `spell_dbc` stores `real_value - 1`.
- Set `EquippedItemClass` to `-1` on every inserted `spell_dbc` row; the table default `0` means "needs a consumable" and silently blocks casts and procs.
- A spell has at most three effects; `EffectSpellClassMaskA/B/C` are per-effect masks (`A_1..A_3` belong to effect 1).
- Check `SpellFamilyFlags` against your own `Spell.dbc`, not against online databases.

## License

MIT (see `LICENSE`).
