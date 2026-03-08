# CLAUDE.md - mod-custom-spells

## Project Overview

AzerothCore module for defining custom spell effects via C++ scripts. Hooks into `OnPlayerSpellCast` (PlayerScript) to intercept specific spell IDs and execute custom logic.

## Module Structure

```
mod-custom-spells/
├── src/
│   ├── mod_custom_spells_loader.cpp  # Module entry point, registers scripts
│   └── custom_spells.cpp            # Custom spell logic (main file to edit)
├── conf/
│   └── mod_custom_spells.conf.dist  # Config: CustomSpells.Enable
└── data/sql/                        # SQL directories (currently unused)
```

## How to Add a New Custom Spell

1. Add spell ID constant to the `CustomSpellIds` enum in `src/custom_spells.cpp`
2. Add a new `case` block in the `switch (spellId)` inside `OnPlayerSpellCast()`
3. Implement custom logic using the `player` and `spell` objects

## Key APIs

- `player->CastSpell(target, spellId, true)` - Cast a spell (true = triggered)
- `player->EnergizeBySpell(target, spellId, amount, powerType)` - Add power (rage/mana/etc). Rage is stored *10 internally
- `spell->m_targets.GetUnitTarget()` - Get the spell's target unit
- `spell->GetSpellInfo()->Id` - Get the cast spell's ID
- `player->GetSelectedUnit()` - Get player's currently selected unit
- `LOG_INFO("module", "format {}", args)` - Logging

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
