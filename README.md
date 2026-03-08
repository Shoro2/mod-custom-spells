# mod-custom-spells

AzerothCore-Modul zum Erstellen von Custom Spell Effekten via C++ Scripts für WoTLK 3.3.5a.

## Features

- Hook auf `OnPlayerSpellCast` für spezifische Spell IDs
- Template-basierte Struktur zum einfachen Hinzufügen neuer Custom Spells
- Aktivierbar/Deaktivierbar über Config

### Enthaltene Custom Spells

| Spell ID | Name | Beschreibung |
|----------|------|-------------|
| 900106 | Rage Burst + Execute | Gibt dem Caster 100 Rage und castet Execute (97471) auf das Spell-Target |

## Installation

1. Modul in das AzerothCore Modules-Verzeichnis klonen/kopieren:
   ```bash
   cd azerothcore-wotlk/modules
   git clone <repo-url> mod-custom-spells
   ```

2. AzerothCore neu bauen:
   ```bash
   cd build
   cmake .. -DCMAKE_INSTALL_PREFIX=<install-pfad> -DSCRIPTS=static -DMODULES=static
   make -j$(nproc)
   make install
   ```

3. Config kopieren und anpassen:
   ```bash
   cp mod_custom_spells.conf.dist mod_custom_spells.conf
   ```

## Neuen Custom Spell hinzufügen

1. Spell ID als Konstante im `CustomSpellIds` Enum in `src/custom_spells.cpp` definieren:
   ```cpp
   enum CustomSpellIds
   {
       SPELL_CUSTOM_MY_SPELL = 123456,
   };
   ```

2. Neuen `case` Block im `switch` in `OnPlayerSpellCast()` hinzufügen:
   ```cpp
   case SPELL_CUSTOM_MY_SPELL:
   {
       // Custom Logik hier
       Unit* target = spell->m_targets.GetUnitTarget();
       if (target)
           player->CastSpell(target, SPELL_SOMETHING, true);
       break;
   }
   ```

3. Core neu bauen.

## Konfiguration

| Option | Standard | Beschreibung |
|--------|----------|-------------|
| `CustomSpells.Enable` | `1` | Modul aktivieren (1) oder deaktivieren (0) |

## Voraussetzungen

- [AzerothCore](https://github.com/azerothcore/azerothcore-wotlk) WoTLK 3.3.5a

## Lizenz

GNU AGPL v3 - siehe [LICENSE](LICENSE)
