# CLAUDE.md - mod-custom-spells

> **Zentrales Projekt-Wiki**: Dieses Modul ist Teil eines Multi-Repo WoW-Server-Projekts. Die übergreifende Dokumentation, Zusatzinfos und Python-Tools befinden sich im [share-public](https://github.com/Shoro2/share-public) Repository:
> - [`CLAUDE.md`](https://github.com/Shoro2/share-public/blob/main/CLAUDE.md) — Gesamtarchitektur, SpellScript/DBC-Referenz, alle Custom-IDs, Modul-Übersicht, **komplette DB-Struktur (304 Tabellen)**, **DBC-Inventar (246 Dateien)**
> - [`claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md) — Änderungshistorie, Projektpläne, priorisierte TODOs
> - [`python_scripts/`](https://github.com/Shoro2/share-public/tree/main/python_scripts) — DBC-Patching-Tools (`patch_dbc.py`, `copy_spells_dbc.py`), Paragon-Spell-Generator (`add_paragon_spell.py`)
> - [`dbc/`](https://github.com/Shoro2/share-public/tree/main/dbc) — Alle 246 WoW-Client DBC-Dateien (Spell.dbc, SpellItemEnchantment.dbc, etc.)
> - [`mysqldbextracts/`](https://github.com/Shoro2/share-public/tree/main/mysqldbextracts) — Komplette DB-Spaltenstruktur (`mysql_column_list_all.txt`), CSV-Exporte (`creature_template.csv`, `item_template.csv`)
>
> **Alle Änderungen an diesem oder den anderen Repos müssen dort geloggt werden.**
>
> **Beim Arbeiten an Custom Spells beachten:**
> - Numerische Werte (Damage, Healing, Absorption) sind immer **reale In-Game-Werte**, nicht interne DBC-Encodings (BasePoints = Spielerwert, Editor konvertiert zu DBC-Format: `EffectBasePoints = BasePoints - 1`)
> - WotLK-Balancing beachten: Low-Level 30–150, Mid 200–600, High 800–2500, Boss 3000–10000+
> - Spells haben max. 3 Effects (Effect1/2/3 in spell.dbc)
> - Periodic Effects: AmplitudeSeconds für Tick-Intervall, DurationSeconds für Gesamtdauer
> - Bei Summon-Spells muss DurationSeconds gesetzt sein (bestimmt Summon-Dauer)
> - Spell-Tooltips können Tokens verwenden: `$d` (Duration), `$s1` (BasePoints Effect 1), etc.
> - Icon wird als semantischer Hint angegeben (z.B. "frost", "fiery melee strike") — der Editor resolved per Fuzzy-Match gegen SpellIcon.dbc

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
└── data/sql/db-world/
    └── mod_custom_spells.sql         # spell_script_names registration
```

## How to Add a New Custom Spell

1. Add spell ID constant to the `CustomSpellIds` enum in `src/custom_spells.cpp`
2. Create a new SpellScript class (see `spell_custom_paragon_strike` as example)
3. Hook into the DBC effect (e.g. `OnEffectHitTarget` with `SPELL_EFFECT_SCHOOL_DAMAGE`)
4. Use `SetHitDamage()` to override damage, `GetCaster()`, `GetHitUnit()` for units
5. Register with `RegisterSpellScript(YourClassName)` in `AddCustomSpellsScripts()`
6. Add the spell to `data/sql/db-world/mod_custom_spells.sql` in `spell_script_names`

## Current Custom Spells (Implemented)

| Spell ID | Script Name | Type | Effect |
|----------|-------------|------|--------|
| 900106 | `spell_custom_paragon_strike` | SpellScript | SCHOOL_DAMAGE: Base 666 + 66% AP, +1%/Paragon level |
| 900107 | `spell_custom_bladestorm_cd_reduce` | AuraScript | PROC: On melee damage dealt → reduce Bladestorm (46927) CD by 0.5s |
| 900115 | (DBC-only buff) | — | +50% Whirlwind damage per stack, max 5, 10s duration |
| 900116 | `spell_custom_bloody_whirlwind_passive` | AuraScript | PROC: On Bloodthirst hit → apply buff 900115 |
| 900117 | `spell_custom_speedy_bloodthirst` | AuraScript | PROC: On Whirlwind cast → reset Bloodthirst (23881) cooldown |
| 900118 | (DBC-only passive marker) | — | WW→Overpower passive: checked via HasAura in 1680 script |
| 900119 | (DBC-only passive marker) | — | WW→Slam passive: checked via HasAura in 1680 script |
| 900120 | `spell_custom_ww_overpower` | SpellScript | OnHit: Multiplies damage by Paragon bonus |
| 900121 | `spell_custom_ww_slam` | SpellScript | OnHit: Multiplies damage by Paragon bonus |
| 1680 | `spell_custom_bloody_whirlwind_consume` | SpellScript | AFTER_CAST: Removes 900115 stacks + single-target autocast |

### Bloody Whirlwind System (900115/900116)

- **900116** (passive aura, EFFECT_1 = `SPELL_AURA_PROC_TRIGGER_SPELL`): Sits on the player, procs on Bloodthirst hits
- **900115** (buff): +50% Whirlwind damage per stack, max 5 stacks, 10s duration
- **1680** (Whirlwind override): Consumes all 900115 stacks after cast
- **Bloodthirst detection**: `SpellFamilyName=4, SpellFamilyFlags[1]=0x00000400` (Bit 42)
- **Important**: The DBC `EffectSpellClassMask` (Class Mask Target Spells) is NOT used by the proc system — filtering is done entirely in the C++ `CheckProc` handler

## DBC Status

Spell IDs 900100-900121 already exist in `Spell.dbc` (22 entries). Many are placeholders with template data. Next free ID for new spells: **900122+**.

IDs with existing C++ SpellScript: 900106, 900107, 900116, 900117, 900120, 900121, 1680
IDs used as DBC-only buffs/markers: 900114, 900115, 900118, 900119
IDs available for reassignment/new use: 900100-900105, 900108-900113

---

## Custom Spell Master Plan (All Classes)

### ID-Block-Schema

| Klasse | Spec | ID-Block | Anzahl |
|--------|------|----------|--------|
| Warrior | Arms | 900100-900109 | 10 |
| Warrior | Fury | 900110-900119 | 10 |
| Warrior | Prot | 900120-900125 | 6 |
| (weitere Klassen) | — | ab 900126+ | — |

> **Hinweis**: Einige IDs in diesen Blöcken sind bereits belegt (900106, 900107, 900114-900121). Die Zuordnung in den Tabellen unten berücksichtigt bestehende Implementierungen. Helper-Spells (Buffs, Trigger-Spells) können IDs außerhalb des Blocks verwenden oder freie Slots nutzen.

---

### Warrior — Arms (900100-900109)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900100 | Mortal Strike damage +50% | **DBC** | NEU | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyName=4, SpellFamilyMask für MS. Kein C++ nötig. |
| 2 | 900101 | Mortal Strike cd -2sec | **DBC** | NEU | Passive Aura: `SPELL_AURA_ADD_FLAT_MODIFIER` + `SPELLMOD_COOLDOWN`, Wert=-2000. SpellFamilyMask für MS. |
| 3 | 900102 | Overpower damage +50% | **DBC** | NEU | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für Overpower. |
| 4 | 900103 | Overpower +9 targets (AoE) | **C++** | NEU | Kein DBC-Modifier für Target-Count. Braucht SpellScript mit `OnObjectAreaTargetSelect` oder Override der Target-Selektion. Overpower muss als AoE umgebaut werden (DBC: TargetA ändern auf AREA_ENEMY). |
| 5 | 900104 | Mortal Strike +9 targets (AoE) | **C++** | NEU | Gleicher Ansatz wie #4. MS wird AoE-fähig gemacht. |
| 6 | 900105 | 20% Crit-Chance → Enhanced Execute (5 stacks) | **C++** | NEU | Passive Proc-Aura: `DoCheckProc` prüft auf Crit (`HitMask & PROC_HIT_CRITICAL`), 20% Chance. Triggered Spell = 900106 (Paragon Strike). Braucht `spell_proc` Eintrag. |
| 7 | 900106 | Enhanced Execute (Damage Spell) | **C++** | ✓ DONE | = Paragon Strike. 666 base + 0.66×AP + 1%/PL. Bereits implementiert. |
| 8 | 900107 | Phys. Damage → Bladestorm CD -0.5s | **C++** | ✓ DONE | Bereits implementiert als `spell_custom_bladestorm_cd_reduce`. |
| 9 | 900108 | Whirlwind unlimited targets | **DBC** | NEU | DBC-Feld `MaxAffectedTargets` auf 0 setzen (= unlimited). Kann via `spell_dbc` Tabelle oder WW-Clone mit unbegrenzten Targets. Alternative: Passive Aura die WW modifiziert. |
| 10a | 900109 | WW 1 target → autocast Overpower | **C++** | ✓ DONE | Implementiert via 900118 (passive marker) + 1680 Script (FilterTargets + autocast 900120). |
| 10b | *neu* | Thunderclap → Rend + 5× Sunder Armor | **C++** | NEU | SpellScript auf Thunderclap (6343/8198/...). `AfterHit`: Auf jedes getroffene Ziel Rend casten + 5× Sunder Armor applyen. Braucht Loop über Targets. ID: freier Slot oder Hook auf original TC. |

### Warrior — Fury (900110-900119)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900110 | Bloodthirst +50% damage | **DBC** | NEU | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für BT (23881). |
| 2 | 900111 | Bloodthirst +9 targets (AoE) | **C++** | NEU | BT ist Single-Target. Muss via SpellScript auf multi-target umgebaut werden. Neue AoE-Variante mit `TARGET_UNIT_SRC_AREA_ENEMY`. |
| 3 | 900112 | Whirlwind damage +50% | **DBC** | NEU | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für WW. Oder: Ist das der Bloody WW Buff (900115)? Falls separate Erhöhung gewünscht → eigene passive Aura. |
| 4 | 900113 | Whirlwind unlimited targets | **DBC** | NEU | Wie Arms #9 (900108). `MaxAffectedTargets=0`. |
| 5 | 900114 | Cleave unlimited targets | **DBC** | NEU | DBC: `MaxAffectedTargets=0` auf Cleave. **Achtung**: 900114 ist aktuell als WW Proc ICD verwendet → muss umgewidmet oder neuer ID. |
| 6 | *neu* | 20% Auto-Attack → free Whirlwind | **C++** | NEU | Passive Proc-Aura: ProcFlags=`DONE_MELEE_AUTO_ATTACK` (0x04), Chance=20. `HandleProc`: CastSpell(WW, triggered=true). Braucht ICD um Spam zu verhindern. |
| 7 | 900115+900116 | BT → next WW +50% damage | **C++** | ✓ DONE | Bloody Whirlwind System. 900116 = passive proc, 900115 = stacking buff. |
| 8 | 900117 | WW resets Bloodthirst CD | **C++** | ✓ DONE | `spell_custom_speedy_bloodthirst`. |
| 9 | *neu* | WW 1 target → autocast Bloodthirst | **C++** | NEU | Analog zu Arms (900118/900120). Passive Marker-Aura + Erweiterung des 1680-Scripts um BT-Autocast. |
| 10 | *neu* | Remove WW stance requirement | **DBC** | NEU | DBC-Feld `StancesNot` / `Stances` auf WW (1680) auf 0 setzen. Entfernt Stance-Requirement. Kann via `spell_dbc` Tabelle. |

### Warrior — Prot (900120-900125)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | *neu* | Revenge +50% damage | **DBC** | NEU | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für Revenge. |
| 2 | *neu* | Revenge unlimited targets | **C++** | NEU | Revenge ist Single-Target. AoE-Umbau via SpellScript. |
| 3 | *reuse* | TC → Rend + 5× Sunder Armor | **C++** | NEU | Gleiche Implementierung wie Arms #10b. Gemeinsame passive Aura, die den TC-Hook aktiviert. |
| 4 | *neu* | Thunderclap +50% damage | **DBC** | NEU | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für TC. |
| 5 | *neu* | AoE damage on Block | **C++** | NEU | `PlayerScript::OnBlock` oder Proc-Aura mit `PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK` + HitMask=BLOCK. HandleProc: AoE-Damage-Spell auf alle Feinde im Radius casten. |
| 6 | *neu* | 10% Block-Chance → Enhanced Thunderclap | **C++** | NEU | Passive Proc-Aura: auf Block procen, 10% Chance. Triggered: Enhanced TC Spell (eigene ID mit höherem Damage). |

> **Hinweis**: Prot-IDs kollidieren mit 900120/900121 (WW Overpower/Slam). Diese Spells gehören logisch zu Arms/Fury. Prot-Spells sollten ab 900122+ oder einem eigenen Block beginnen.

---

### Implementierungs-Kategorien

#### Rein DBC (kein C++ nötig) — via `spell_dbc` Tabelle oder WoW Spell Editor

Diese Spells sind passive Auren, die über DBC-Felder andere Spells modifizieren:

| Effekt-Typ | DBC Aura | DBC MiscValue | Beispiel |
|------------|----------|---------------|----------|
| Damage +X% | `SPELL_AURA_ADD_PCT_MODIFIER` (108) | `SPELLMOD_DAMAGE` (0) | MS +50%, Overpower +50%, BT +50%, TC +50%, Revenge +50% |
| Cooldown -Xs | `SPELL_AURA_ADD_FLAT_MODIFIER` (107) | `SPELLMOD_COOLDOWN` (11) | MS cd -2s (BasePoints=-2000) |
| Unlimited targets | Direkte DBC-Änderung | `MaxAffectedTargets=0` | WW, Cleave |
| Remove stance req | Direkte DBC-Änderung | `Stances=0` | WW stance removal |

**Voraussetzungen für DBC-Ansatz:**
- SpellFamilyName und SpellFamilyFlags des Ziel-Spells müssen in unserer DBC verifiziert werden
- Passive Aura muss `Attributes` = `SPELL_ATTR0_PASSIVE` (0x40) haben
- `EffectSpellClassMask` der passiven Aura muss auf die SpellFamilyFlags des Ziels matchen

#### C++ erforderlich — SpellScript/AuraScript

| Mechanik | Grund für C++ | Beispiele |
|----------|---------------|-----------|
| Conditional Procs | Komplexe Bedingungen (Crit-Check, Target-Count, Cooldown-Check) | 20% Crit → Execute, WW 1-target autocast |
| Multi-Spell Triggers | Ein Proc löst mehrere verschiedene Spells aus | TC → Rend + 5× Sunder |
| Single→AoE Umbau | Adds target selection to single-target spells | Overpower +9 targets, MS +9 targets, BT +9 targets, Revenge unlimited |
| Block-basierte Procs | Block-Events brauchen UnitScript/PlayerScript Hooks | AoE on block, 10% block → TC |
| Custom Damage Formeln | Nicht-Standard Schadensberechnung | Paragon Strike (666 + 0.66×AP + PL%) |

#### Wichtige Hinweise

1. **SpellFamilyFlags verifizieren**: Vor jeder DBC-Implementierung müssen die SpellFamilyFlags des Ziel-Spells in unserer Spell.dbc geprüft werden (nicht aus Online-DBs übernehmen!)
2. **MaxAffectedTargets**: Änderung auf 0 betrifft ALLE Spieler, nicht nur die mit dem Passive. Für bedingte unlimited targets → C++ `OnObjectAreaTargetSelect`
3. **ID-Konflikte**: 900114 (aktuell WW Proc ICD) und 900120/900121 (WW Boosted Spells) müssen bei der Prot-Planung berücksichtigt werden
4. **Helper-Spells**: Viele Effekte brauchen 2+ Spell-IDs (Passive + Trigger + evtl. Buff). Genügend ID-Raum pro Spec einplanen
5. **spell_proc Tabelle**: Jeder proc-basierte Spell braucht einen Eintrag in `spell_proc` mit korrekten ProcFlags
6. **Testen**: Neue DBC-Modifier immer in-game testen — manche Aura-Typen funktionieren nicht für alle Spell-Kombinationen

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
