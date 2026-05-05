# mod-custom-spells

AzerothCore module for creating custom spell effects via C++ scripts for WoTLK 3.3.5a.

## Features

- Hook on `OnPlayerSpellCast` for specific spell IDs
- Template-based structure for easily adding new custom spells
- Enable/disable via config

### Included custom spells

| Spell ID | Name | Description |
|----------|------|-------------|
| 900106 | Rage Burst + Execute | Gives the caster 100 rage and casts Execute (97471) on the spell target |

## Installation

1. Clone/copy the module into the AzerothCore modules directory:
   ```bash
   cd azerothcore-wotlk/modules
   git clone <repo-url> mod-custom-spells
   ```

2. Rebuild AzerothCore:
   ```bash
   cd build
   cmake .. -DCMAKE_INSTALL_PREFIX=<install-path> -DSCRIPTS=static -DMODULES=static
   make -j$(nproc)
   make install
   ```

3. Copy and adjust the config:
   ```bash
   cp mod_custom_spells.conf.dist mod_custom_spells.conf
   ```

## Adding a new custom spell

1. Define the spell ID as a constant in the `CustomSpellIds` enum in `src/custom_spells.cpp`:
   ```cpp
   enum CustomSpellIds
   {
       SPELL_CUSTOM_MY_SPELL = 123456,
   };
   ```

2. Add a new `case` block in the `switch` inside `OnPlayerSpellCast()`:
   ```cpp
   case SPELL_CUSTOM_MY_SPELL:
   {
       // Custom logic here
       Unit* target = spell->m_targets.GetUnitTarget();
       if (target)
           player->CastSpell(target, SPELL_SOMETHING, true);
       break;
   }
   ```

3. Rebuild the core.

## Configuration

| Option | Default | Description |
|--------|----------|-------------|
| `CustomSpells.Enable` | `1` | Enable (1) or disable (0) the module |

## Requirements

- [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk) WoTLK 3.3.5a

## License

GNU AGPL v3 - see [LICENSE](LICENSE)
