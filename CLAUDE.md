# CLAUDE.md - mod-custom-spells

## Project Overview

AzerothCore module for defining custom spell effects via C++ SpellScripts. Each custom spell gets its own SpellScript class that hooks into the spell's DBC effects (e.g. School Damage) and overrides the damage/behavior.

## Module Structure

```
mod-custom-spells/
├── src/
│   ├── mod_custom_spells_loader.cpp  # Module entry point, registers scripts
│   └── custom_spells.cpp            # Custom spell logic (main file to edit)
├── conf/
│   └── mod_custom_spells.conf.dist  # Config: CustomSpells.Enable
├── data/sql/db-world/
│   └── mod_custom_spells.sql         # spell_dbc + spell_script_names + spell_proc
└── tools/
    └── extract_custom_spells.py      # Extract spells >= 900000 from Spell.dbc
```

## How to Add a New Custom Spell

1. Add spell ID constant to the `CustomSpellIds` enum in `src/custom_spells.cpp`
2. Create a new SpellScript class (see `spell_custom_paragon_strike` as example)
3. Hook into the DBC effect (e.g. `OnEffectHitTarget` with `SPELL_EFFECT_SCHOOL_DAMAGE`)
4. Use `SetHitDamage()` to override damage, `GetCaster()`, `GetHitUnit()` for units
5. Register with `RegisterSpellScript(YourClassName)` in `AddCustomSpellsScripts()`
6. Add `spell_dbc` entry in `mod_custom_spells.sql` (defines the spell in the server's spell store)
7. Add `spell_script_names` entry in `mod_custom_spells.sql` (links spell ID to C++ script)
8. Add `spell_proc` entry if the spell uses proc mechanics

**IMPORTANT**: The binary `Spell.dbc` in `azerothcore-wotlk/share/dbc/` has corrupted record alignment for custom spells. Always define custom spells via the `spell_dbc` SQL table instead.

## Custom Spell Reference

### 900106 - Paragon Strike
- **Type**: Castable spell (SCHOOL_DAMAGE on enemy)
- **C++ Script**: `spell_custom_paragon_strike`
- **DBC**: Effect_1=SCHOOL_DAMAGE, TargetA=ENEMY, DmgClass=MELEE, School=Physical
- **Logic**: Base 666 + 66% AP + 1% per Paragon level (aura 100000 stacks)

### 900107 - Bladestorm Cooldown Reduction
- **Type**: Passive proc (DUMMY on self, triggers on melee hit)
- **C++ Script**: `spell_custom_bladestorm_cd_reduce`
- **DBC**: Attr=PASSIVE, Effect_1=DUMMY, TargetA=CASTER, ProcMask=0x14 (auto+melee spell), Chance=20%
- **Logic**: Each proc reduces Bladestorm (46927) CD by 500ms

### 900114 - Whirlwind Proc Aura
- **Type**: Passive proc aura (triggers 900113 on melee auto-attack)
- **DBC**: Attr=PASSIVE, Effect_1=APPLY_AURA(42=PROC_TRIGGER_SPELL), TriggerSpell=900113, ProcMask=0x4 (auto-attack), Chance=20%, RecoveryTime=500ms
- **SQL**: spell_proc entry with 500ms ICD to prevent recursive proc loops

### 900115 - Bloody Whirlwind (Buff)
- **Type**: Stackable buff (+50% Whirlwind damage per stack)
- **DBC**: Effect_1=APPLY_AURA(108=ADD_PCT_MODIFIER), BasePoints=50, Stacks=5, Duration=15s (idx 8)
- **DBC**: SpellClassSet=4 (Warrior), EffectClassMaskA[1]=0x4 (matches Whirlwind family flags)
- **Consumed by**: Whirlwind (1680) via `spell_custom_bloody_whirlwind_consume`

### 900116 - Bloody Whirlwind Passive
- **Type**: Passive proc aura (triggers 900115 on Bloodthirst melee spell hit)
- **C++ Script**: `spell_custom_bloody_whirlwind_passive` (AuraScript)
- **DBC**: Attr=0x1d0 (PASSIVE+HIDDEN), **Effect_0**=APPLY_AURA(42=PROC_TRIGGER_SPELL), TriggerSpell=900115
- **DBC**: ProcMask=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS), Chance=100%
- **C++ Hook**: EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL → CheckProc filters to Bloodthirst only
- **SQL**: spell_proc entry overrides ProcFlags=0x10, SpellTypeMask=DAMAGE, Phase=HIT

### 1680 - Whirlwind (vanilla spell, script hook only)
- **C++ Script**: `spell_custom_bloody_whirlwind_consume`
- **Logic**: AfterCast removes all Bloody Whirlwind (900115) stacks

## Proc System Notes

- `spell_dbc.ProcTypeMask` defines DBC-level proc flags (loaded at startup)
- `spell_proc` table overrides proc data at SQL level (loaded after DBC)
- C++ `DoCheckProc` further filters procs in the script
- **PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS = 0x10** → melee spells like Bloodthirst
- **PROC_FLAG_DONE_MELEE_AUTO_ATTACK = 0x4** → white melee hits
- If `SpellFamilyName=0` in spell_proc, any spell can trigger (C++ script narrows it)
- The C++ `OnEffectProc` index must match the DBC/spell_dbc effect index!

## Key APIs (SpellScript)

- `GetCaster()` / `GetHitUnit()` - Get caster and target units
- `SetHitDamage(amount)` - Override the spell effect's damage
- `GetHitDamage()` - Get current calculated damage
- `GetSpellInfo()` - Get the SpellInfo of the spell being cast
- `GetCaster()->ToPlayer()` - Cast to Player for player-specific APIs
- `player->GetTotalAttackPowerValue(BASE_ATTACK)` - Get melee AP
- `player->GetAuraCount(auraId)` - Get stack count of an aura
- `player->ModifySpellCooldown(spellId, deltaMs)` - Modify a spell's cooldown (negative = reduce)
- `LOG_INFO("module", "format {}", args)` - Logging
- `RegisterSpellScript(ClassName)` - Register in AddCustomSpellsScripts()

## Key APIs (AuraScript)

- `DoCheckProc` - Filter which events trigger the proc (return true to allow)
- `OnEffectProc` - Execute when proc fires (EFFECT_INDEX must match DBC effect!)
- `PreventDefaultAction()` - Stop default proc behavior, handle manually
- `GetTarget()` - Get the aura holder
- `eventInfo.GetSpellInfo()` - Get the triggering spell's SpellInfo
- `spellInfo->SpellFamilyName` / `spellInfo->SpellFamilyFlags[N]` - Family matching

## Tools

- `tools/extract_custom_spells.py` - Extract spells >= 900000 from Spell.dbc
  ```
  python3 tools/extract_custom_spells.py <Spell.dbc> [output.dbc]
  ```
  Handles corrupted DBC files by scanning at all 4-byte boundaries.

## Build

This module is built automatically when placed in `azerothcore-wotlk/modules/mod-custom-spells/`. No separate build step needed - it compiles with the core.

## Code Style

Follow AzerothCore conventions:
- 4-space indentation, no tabs
- `Type const*` (not `const Type*`)
- `UPPER_SNAKE_CASE` for spell/NPC constants with prefix: `SPELL_CUSTOM_*`
- `UpperCamelCase` for class/method names
- No braces around single-line if/else/for/while

## Config

`CustomSpells.Enable` (default: 1) in `mod_custom_spells.conf.dist` controls whether the module processes spell casts.

## Loader Convention

The loader function in `mod_custom_spells_loader.cpp` must be named `Addmod_custom_spellsScripts()` - module folder name with `-` replaced by `_`.
