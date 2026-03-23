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

## Workflow: Custom Spell erstellen (Schritt für Schritt)

### Übersicht: Zwei Wege für Custom Spells

```
                    ┌──────────────────────────┐
                    │  Neuen Custom Spell planen │
                    └────────────┬─────────────┘
                                 │
                    ┌────────────▼─────────────┐
                    │  Braucht der Spell C++?   │
                    └──┬───────────────────┬───┘
                       │                   │
                  Nein │                   │ Ja
                       │                   │
           ┌───────────▼──────┐  ┌─────────▼──────────┐
           │  Weg A: Nur DBC  │  │  Weg B: DBC + C++  │
           │  (spell_dbc SQL) │  │  (DBC + SpellScript)│
           └───────────┬──────┘  └─────────┬──────────┘
                       │                   │
                       └─────────┬─────────┘
                                 │
                    ┌────────────▼─────────────┐
                    │  Client-DBC patchen       │
                    │  (Spell.dbc für Tooltips) │
                    └────────────┬─────────────┘
                                 │
                    ┌────────────▼─────────────┐
                    │  Server bauen + testen    │
                    └──────────────────────────┘
```

### Weg A: Rein DBC-basierter Spell (kein C++)

Geeignet für: Damage +X%, Cooldown -Xs, passive Stat-Modifier, SpellFamilyMask-basierte Buffs.

**Schritt 1: Spell-ID reservieren**
- Nächste freie ID aus dem Master Plan wählen (siehe ID-Block-Schema unten)
- ID in CLAUDE.md dokumentieren

**Schritt 2: spell_dbc INSERT in SQL erstellen**

Die `spell_dbc` Tabelle überschreibt beim Server-Start die Werte aus `Spell.dbc`. Neue Spell-IDs werden komplett aus der DB geladen. Relevante Spalten für passive Auren:

```sql
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`,
    `CastingTimeIndex`, `DurationIndex`, `RangeIndex`,
    `Effect_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`,
    `EffectAura_1`, `EffectMiscValue_1`,
    `EffectSpellClassMaskA_1`,
    `SpellFamilyName`, `SpellIconID`,
    `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900168,                     -- ID: Eindeutige Spell-ID
 0x10000040,                 -- Attributes: PASSIVE (0x40) + NOT_SHAPESHIFT (0x10000000)
 0x10000000,                 -- AttributesEx3: DEATH_PERSISTENT
 1,                          -- CastingTimeIndex: 1 = Instant
 21,                         -- DurationIndex: 21 = Permanent (-1)
 1,                          -- RangeIndex: 1 = Self
 6,                          -- Effect_1: SPELL_EFFECT_APPLY_AURA
 50,                         -- EffectBasePoints_1: +50% (Wert des Modifiers)
 1,                          -- ImplicitTargetA_1: TARGET_UNIT_CASTER
 108,                        -- EffectAura_1: SPELL_AURA_ADD_PCT_MODIFIER
 0,                          -- EffectMiscValue_1: SPELLMOD_DAMAGE (0)
 0x400,                      -- EffectSpellClassMaskA_1: Ziel-Spell FamilyFlags[0]
 4,                          -- SpellFamilyName: 4 = Warrior
 132,                        -- SpellIconID: beliebiges Icon
 'Prot: Revenge Damage',     -- Name (englisch)
 0x003F3F);                  -- Name_Lang_Mask: alle Locales nutzen enUS
```

**Wichtige DBC-Attribute:**

| Attribut | Hex-Wert | Bedeutung |
|----------|----------|-----------|
| `SPELL_ATTR0_PASSIVE` | `0x40` | Spell ist unsichtbar, immer aktiv |
| `SPELL_ATTR0_NOT_SHAPESHIFT` | `0x10000000` | Bleibt in allen Stances |
| `SPELL_ATTR3_DEATH_PERSISTENT` | `0x10000000` | Überlebt Tod |

**Häufige EffectAura-Werte:**

| Aura-ID | Name | MiscValue | Anwendung |
|---------|------|-----------|-----------|
| 108 | `ADD_PCT_MODIFIER` | 0=DAMAGE, 11=COOLDOWN, 14=CAST_TIME | Spell-Modifier prozentual |
| 107 | `ADD_FLAT_MODIFIER` | 0=DAMAGE, 11=COOLDOWN | Spell-Modifier flat |
| 4 | `DUMMY` | — | Marker-Aura (C++ prüft via HasAura) |
| 42 | `PROC_TRIGGER_SPELL` | — | Triggert anderen Spell bei Proc |

**EffectSpellClassMask — Ziel-Spell identifizieren:**

Die Mask muss auf die SpellFamilyFlags des Ziel-Spells matchen. Drei 32-Bit-Felder:
- `EffectSpellClassMaskA_1` → SpellFamilyFlags[0]
- `EffectSpellClassMaskB_1` → SpellFamilyFlags[1]
- `EffectSpellClassMaskC_1` → SpellFamilyFlags[2]

> **WICHTIG**: SpellFamilyFlags IMMER aus der eigenen Spell.dbc verifizieren, nicht aus Online-DBs!

**Schritt 3: SQL in `mod_custom_spells.sql` einfügen**

```sql
-- Am Ende der Datei anhängen
DELETE FROM `spell_dbc` WHERE `ID` = 900168;
INSERT INTO `spell_dbc` (...) VALUES (...);
```

**Schritt 4: Client-DBC patchen** (siehe Abschnitt unten)

**Schritt 5: Server bauen und testen** (siehe Abschnitt unten)

---

### Weg B: Spell mit C++ SpellScript

Geeignet für: Conditional Procs, Multi-Spell-Triggers, AoE-Umbau, Block-Procs, Custom Formeln.

**Schritt 1: DBC-Entry erstellen (wie Weg A)**

Auch C++-Spells brauchen einen DBC-Entry. Für Marker-Auren (HasAura-Check) reicht eine DUMMY-Aura:

```sql
-- Marker-Aura: kein eigener Effekt, nur für HasAura() Check
(900169, 0x10000040, 0, 0, 0x10000000, 1, 21, 1,
 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Revenge AoE', 0x003F3F),
```

Für Proc-Auren (die über das spell_proc-System getriggert werden):
```sql
-- Proc-Aura: DUMMY-Effekt, Proc-Verhalten über spell_proc + C++
(900172, 0x10000040, 0, 0, 0x10000000, 1, 21, 1,
 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block AoE', 0x003F3F),
```

Für Helper-Spells (getriggerte Damage/Heal-Spells):
```sql
-- Helper: Instant AoE Physical Damage, TARGET_UNIT_SRC_AREA_ENEMY (22)
(900174, 0x10000000, 0, 0, 0, 1, 0, 1,
 2, 100, 500, 22, 0, 0, 0, 0, 4, 132, 'Block Shield Burst', 0x003F3F),
--       Effect=SCHOOL_DAMAGE(2), DieSides=100(random), BasePoints=500
```

**Schritt 2: Enum-Konstante hinzufügen** in `src/custom_spells.cpp`:

```cpp
enum CustomSpellIds
{
    // ... bestehende IDs ...
    SPELL_PROT_REVENGE_AOE_PASSIVE = 900169,
};
```

**Schritt 3: SpellScript/AuraScript-Klasse erstellen**

Es gibt 4 Haupt-Patterns:

**Pattern A: SpellScript mit AfterHit (Hook auf bestehenden Spell)**
```cpp
// Hooked auf Revenge (57823) via spell_script_names
class spell_custom_prot_revenge_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_prot_revenge_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target) return;

        Player* player = caster->ToPlayer();
        if (!player) return;

        // Marker-Aura prüfen
        if (!player->HasAura(SPELL_PROT_REVENGE_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Custom Logik hier
        int32 damage = GetHitDamage();
        caster->CastSpell(target, HELPER_SPELL_ID, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_prot_revenge_aoe::HandleAfterHit);
    }
};
```

**Pattern B: AuraScript mit Proc (passive Proc-Aura)**
```cpp
class spell_custom_prot_block_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_prot_block_aoe);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player) return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // HitMask prüfen (Block, Dodge, Parry, Crit etc.)
        if (!(eventInfo.GetHitMask() & PROC_HIT_BLOCK))
            return;

        player->CastSpell(player, HELPER_SPELL_ID, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_prot_block_aoe::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};
```

**Pattern C: AuraScript mit CheckProc (gefilterte Procs)**
```cpp
class spell_custom_speedy_bloodthirst : public AuraScript
{
    PrepareAuraScript(spell_custom_speedy_bloodthirst);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo) return false;
        // Nur auf Whirlwind (1680) procen
        return spellInfo->Id == 1680;
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        if (!player) return;
        player->RemoveSpellCooldown(SPELL_BLOODTHIRST, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_speedy_bloodthirst::CheckProc);
        OnProc += AuraProcFn(spell_custom_speedy_bloodthirst::HandleProc);
    }
};
```

**Pattern D: SpellScript mit OnEffectHitTarget (Damage Override)**
```cpp
class spell_custom_paragon_strike : public SpellScript
{
    PrepareSpellScript(spell_custom_paragon_strike);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        // Custom Damage-Formel
        int32 totalDmg = CalculateCustomDamage();
        SetHitDamage(totalDmg);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(
            spell_custom_paragon_strike::HandleDamage,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
```

**Schritt 4: Script registrieren** in `AddCustomSpellsScripts()`:

```cpp
void AddCustomSpellsScripts()
{
    // ... bestehende ...
    RegisterSpellScript(spell_custom_prot_revenge_aoe);
};
```

**Schritt 5: spell_script_names SQL** — Verknüpft Spell-ID mit C++ Klasse:

```sql
-- Für eigene Custom-Spell-IDs:
(900172, 'spell_custom_prot_block_aoe'),
-- Für Hooks auf bestehende Blizzard-Spells:
(57823, 'spell_custom_prot_revenge_aoe'),  -- hooked auf Revenge
(47502, 'spell_custom_prot_tc_rend_sunder'), -- hooked auf Thunderclap
```

> **Wichtig**: Hook auf bestehende Spells (57823, 47502, 1680) → die C++ Klasse wird bei JEDEM Cast dieses Spells ausgeführt. Immer `HasAura()` prüfen damit der Effekt nur aktiv ist wenn der Spieler die Passive hat!

**Schritt 6: spell_proc SQL** (nur für Proc-basierte Auren):

```sql
DELETE FROM `spell_proc` WHERE `SpellId` = 900172;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`,
    `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
    `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`,
    `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`,
    `Chance`, `Cooldown`, `Charges`) VALUES
(900172,    -- SpellId
 0,         -- SchoolMask (0 = alle)
 0,         -- SpellFamilyName (0 = alle, 4 = Warrior etc.)
 0, 0, 0,   -- SpellFamilyMask0/1/2 (0 = alle Spells, C++ filtert)
 0x2,       -- ProcFlags: PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK
 0,         -- SpellTypeMask (0 = alle)
 0,         -- SpellPhaseMask (0=alle, 2=HIT, 4=FINISH)
 0,         -- HitMask (0 = alle, C++ prüft PROC_HIT_BLOCK)
 0, 0,      -- AttributesMask, DisableEffectsMask
 0,         -- ProcsPerMinute (0 = nutzt Chance statt PPM)
 100,       -- Chance: 100% (C++ filtert zusätzlich)
 1000,      -- Cooldown: 1000ms (ICD)
 0);        -- Charges: 0 = unbegrenzt
```

**Häufige ProcFlags:**

| Flag | Hex | Bedeutung |
|------|-----|-----------|
| `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` | `0x4` | Eigener Melee-Autoattack |
| `PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK` | `0x2` | Gegnerischer Melee-Autoattack |
| `PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS` | `0x10` | Eigener Melee-Spell (Bloodthirst etc.) |
| `PROC_FLAG_DONE_PERIODIC` | `0x400000` | Eigener DoT-Tick |
| `PROC_FLAG_KILL` | `0x1` | Gegner getötet |
| `PROC_FLAG_TAKEN_DAMAGE` | `0x4000` | Schaden erhalten |
| Kombination: Melee+Spell | `0x14` | Autoattack ODER Melee-Spell |

**Häufige PROC_HIT Masken (für C++ eventInfo.GetHitMask()):**

| Mask | Bedeutung |
|------|-----------|
| `PROC_HIT_BLOCK` | Angriff wurde geblockt |
| `PROC_HIT_DODGE` | Angriff wurde ausgewichen |
| `PROC_HIT_PARRY` | Angriff wurde pariert |
| `PROC_HIT_CRITICAL` | Kritischer Treffer |

---

### Client-DBC Patchen (für beide Wege)

Custom Spells brauchen auch einen Eintrag in der **Client-seitigen Spell.dbc**, damit der WoW-Client Tooltips/Icons korrekt anzeigt. Der Server lädt seine Daten aus der `spell_dbc` DB-Tabelle, der Client braucht die DBC-Datei.

**Option 1: Python-Script `merge_spell_dbc.py`** (empfohlen für Batch)

```bash
# 1. spell_dbc Tabelle als CSV exportieren
mysql -u root acore_world -e "SELECT * FROM spell_dbc WHERE ID >= 900100" \
    --batch --raw | tr '\t' ';' > spell_dbc.csv

# 2. In Verzeichnis mit Spell.dbc und spell_dbc.csv:
python3 /home/user/share-public/python_scripts/merge_spell_dbc.py .

# 3. Output: Spell_merged.dbc → in Client und Server kopieren
cp Spell_merged.dbc /path/to/wow-client/Data/patch-4.MPQ/DBFilesClient/Spell.dbc
cp Spell_merged.dbc /path/to/server/data/dbc/Spell.dbc
```

**Option 2: Python-Script `copy_spells_dbc.py`** (für einzelne Spells)

```bash
# Spells aus einer DBC in eine andere kopieren
python3 copy_spells_dbc.py source_Spell.dbc target_Spell.dbc 900168 900169 900170

# Oder aus Datei:
python3 copy_spells_dbc.py source.dbc target.dbc --file spell_ids.txt
```

**Option 3: WoW Spell Editor** (GUI, für manuelle Bearbeitung)

Spell.dbc direkt im Editor öffnen, Spell anlegen, speichern.

**DBC-Dateien Standorte:**

| Datei | Server-Pfad | Client-Pfad |
|-------|-------------|-------------|
| `Spell.dbc` | `azerothcore-wotlk/data/dbc/` | `WoW/Data/patch-4.MPQ/DBFilesClient/` |
| Originale DBCs | — | `share-public/dbc/` (246 Dateien) |

> **Server-Ladereihenfolge**: Server lädt zuerst `Spell.dbc` aus dem Dateisystem, dann überschreibt `spell_dbc` DB-Tabelle alle dort definierten Einträge (field-by-field override). Neue IDs die NUR in `spell_dbc` existieren werden komplett aus der DB geladen.

---

### Server bauen und testen

```bash
# 1. Symlink prüfen
ls /path/to/azerothcore-wotlk/modules/mod-custom-spells

# 2. Build (falls CMake bereits konfiguriert)
cd /path/to/azerothcore-wotlk/build
make -j$(nproc)

# 3. Falls erster Build: CMake konfigurieren
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/azeroth-server \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DSCRIPTS=static -DMODULES=static \
    -DTOOLS_BUILD=none -DAPPS_BUILD=world-only
make -j$(nproc)

# 4. SQL anwenden (nach Server-Start)
# mod_custom_spells.sql wird automatisch geladen wenn das Modul installiert ist

# 5. In-Game testen
.aura 900168    # Passive Aura auf Spieler anwenden
.unaura 900168  # Passive Aura entfernen
```

---

### Checkliste: Neuer Custom Spell

```
□ Spell-ID reserviert und in CLAUDE.md dokumentiert
□ spell_dbc INSERT in mod_custom_spells.sql
□ (wenn C++) Enum-Konstante in custom_spells.cpp
□ (wenn C++) SpellScript/AuraScript Klasse implementiert
□ (wenn C++) RegisterSpellScript() in AddCustomSpellsScripts()
□ (wenn C++) spell_script_names INSERT in mod_custom_spells.sql
□ (wenn Proc) spell_proc INSERT in mod_custom_spells.sql
□ (wenn Helper) Helper-Spell DBC + ggf. C++ Script
□ Client-Spell.dbc gepatcht (für Tooltips)
□ Build erfolgreich (0 Errors)
□ In-Game getestet
□ CLAUDE.md Status auf "implementiert" → "getestet" aktualisiert
```

---

### spell_dbc Spalten-Referenz (wichtigste Felder)

Die `spell_dbc` Tabelle hat 257 Spalten. Hier die für Custom Spells relevantesten:

| Spalte | Typ | Beschreibung |
|--------|-----|-------------|
| `ID` | uint | Eindeutige Spell-ID |
| `Attributes` | uint | Flags: 0x40=PASSIVE, 0x10000000=NOT_SHAPESHIFT |
| `AttributesEx` | uint | Extended Flags 1 |
| `AttributesEx2` | uint | Extended Flags 2 |
| `AttributesEx3` | uint | Extended Flags 3 (0x10000000=DEATH_PERSISTENT) |
| `CastingTimeIndex` | uint | 1=Instant, andere Werte → SpellCastTimes.dbc |
| `DurationIndex` | uint | 0=Instant, 21=Permanent, andere → SpellDuration.dbc |
| `RangeIndex` | uint | 1=Self, 4=30yd, 6=100yd → SpellRange.dbc |
| `ProcTypeMask` | uint | Proc-Trigger-Flags (überschrieben durch spell_proc) |
| `ProcChance` | uint | Proc-Wahrscheinlichkeit (überschrieben durch spell_proc) |
| `Effect_1/2/3` | uint | 2=SCHOOL_DAMAGE, 6=APPLY_AURA, 3=DUMMY |
| `EffectDieSides_1/2/3` | int | Random-Range für Damage (0=kein Random) |
| `EffectBasePoints_1/2/3` | int | Basis-Wert (Damage, Modifier-%, etc.) |
| `ImplicitTargetA_1/2/3` | uint | 1=SELF, 6=ENEMY, 22=SRC_AREA_ENEMY |
| `EffectRadiusIndex_1/2/3` | uint | 8=5yd, 13=8yd, 14=10yd, 28=30yd |
| `EffectAura_1/2/3` | uint | Aura-Typ: 4=DUMMY, 108=ADD_PCT_MODIFIER |
| `EffectMiscValue_1/2/3` | int | Aura-spezifisch: 0=SPELLMOD_DAMAGE, 11=SPELLMOD_COOLDOWN |
| `EffectTriggerSpell_1/2/3` | uint | Spell-ID die bei Proc getriggert wird |
| `EffectSpellClassMaskA/B/C_1` | uint | SpellFamilyFlags[0/1/2] des Ziel-Spells |
| `SpellFamilyName` | uint | 0=Generic, 4=Warrior, 10=Paladin, 15=DK |
| `SpellClassMask_1/2/3` | uint | SpellFamilyFlags dieses Spells (für Proc-Matching) |
| `SpellIconID` | uint | Icon-ID aus SpellIcon.dbc |
| `MaxTargets` | uint | Max Targets (0=unlimited) |
| `SchoolMask` | uint | 1=Physical, 2=Holy, 4=Fire, 16=Shadow, 32=Arcane |
| `Name_Lang_enUS` | string | Spell-Name (englisch) |
| `Name_Lang_Mask` | uint | 0x003F3F = alle Locales nutzen enUS |

### SpellFamilyName Werte

| Wert | Klasse |
|------|--------|
| 0 | Generic |
| 3 | Mage |
| 4 | Warrior |
| 5 | Warlock |
| 6 | Priest |
| 7 | Druid |
| 8 | Rogue |
| 9 | Hunter |
| 10 | Paladin |
| 11 | Shaman |
| 15 | Death Knight |

## Current Custom Spells (Implemented)

| Spell ID | Script Name | Type | Effect |
|----------|-------------|------|--------|
| 900106 | `spell_custom_paragon_strike` | SpellScript | SCHOOL_DAMAGE: Base 666 + 66% AP, +1%/Paragon level |
| 900107 | `spell_custom_bladestorm_cd_reduce` | AuraScript | PROC: On melee damage dealt → reduce Bladestorm (46927) CD by 0.5s |
| 900139 | (DBC-only buff) | — | +50% Whirlwind damage per stack, max 5, 10s duration |
| 900140 | `spell_custom_bloody_whirlwind_passive` | AuraScript | PROC: On Bloodthirst hit → apply buff 900139 |
| 900141 | `spell_custom_speedy_bloodthirst` | AuraScript | PROC: On Whirlwind cast → reset Bloodthirst (23881) cooldown |
| 900142 | (DBC-only passive marker) | — | WW→Overpower passive: checked via HasAura in 1680 script |
| 900143 | (DBC-only passive marker) | — | WW→Slam passive: checked via HasAura in 1680 script |
| 900144 | `spell_custom_ww_overpower` | SpellScript | OnHit: Multiplies damage by Paragon bonus |
| 900145 | `spell_custom_ww_slam` | SpellScript | OnHit: Multiplies damage by Paragon bonus |
| 1680 | `spell_custom_bloody_whirlwind_consume` | SpellScript | AFTER_CAST: Removes 900139 stacks + single-target autocast |

### Bloody Whirlwind System (900139/900140)

- **900140** (passive aura, EFFECT_1 = `SPELL_AURA_PROC_TRIGGER_SPELL`): Sits on the player, procs on Bloodthirst hits
- **900139** (buff): +50% Whirlwind damage per stack, max 5 stacks, 10s duration
- **1680** (Whirlwind override): Consumes all 900139 stacks after cast
- **Bloodthirst detection**: `SpellFamilyName=4, SpellFamilyFlags[1]=0x00000400` (Bit 42)
- **Important**: The DBC `EffectSpellClassMask` (Class Mask Target Spells) is NOT used by the proc system — filtering is done entirely in the C++ `CheckProc` handler

## DBC Status

Spell IDs 900100-900109 (Warrior Arms) existieren in `Spell.dbc` und sind fertig implementiert.
Spell IDs 900134-900145 (Warrior Fury) existieren in `Spell.dbc` und sind fertig implementiert.
Spell IDs 900168-900175 (Warrior Prot + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900200-900210 (Paladin Holy + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900234-900241 (Paladin Prot) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900268-900275 (Paladin Ret + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900300-900304 (DK Blood) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900333 (DK Frost) existiert in `spell_dbc` Tabelle und ist implementiert.
Spell IDs 900366-900367 (DK Unholy + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.

Nächste freie Klassen-Blöcke: **900400+** (Shaman), **900500+** (Hunter), etc.

---

## Custom Spell Master Plan (All Classes)

> **Status-Legende**: `geplant` → `implementiert` → `getestet`

### ID-Block-Schema

> **Regel**: 100 IDs pro Klasse, 33 Plätze pro Spec (Spec1: +0..+32, Spec2: +33..+65, Spec3: +66..+99).

| Klasse | Block | Spec 1 | Spec 2 | Spec 3 |
|--------|-------|--------|--------|--------|
| **Warrior** | 900100-900199 | Arms: 900100-900132 | Fury: 900133-900238 | Prot: 900239-900199 |
| **Paladin** | 900200-900299 | Holy: 900200-900232 | Prot: 900233-900265 | Ret: 900266-900299 |
| **DK** | 900300-900399 | Blood: 900300-900332 | Frost: 900333-900365 | Unholy: 900366-900399 |
| **Shaman** | 900400-900499 | Ele: 900400-900432 | Enhance: 900433-900465 | Resto: 900466-900499 |
| **Hunter** | 900500-900599 | BM: 900500-900532 | MM: 900533-900565 | Surv: 900566-900599 |
| **Rogue** | 900600-900699 | Assa: 900600-900632 | Combat: 900633-900665 | Sub: 900666-900699 |
| **Mage** | 900700-900799 | Arcane: 900700-900732 | Fire: 900733-900765 | Frost: 900766-900799 |
| **Warlock** | 900800-900899 | Affli: 900800-900832 | Demo: 900833-900865 | Destro: 900866-900899 |
| **Priest** | 900900-900999 | Disc: 900900-900932 | Holy: 900933-900965 | Shadow: 900966-900999 |
| **Druid** | 901000-901099 | Balance: 901000-901032 | Feral: 901033-901065 | Resto: 901066-901099 |
| **Non-Class** | 901100-901199 | Global: 901100-901199 | — | — |

### Aktuelle Belegung

| Klasse | Spec | Belegt | Frei | Status |
|--------|------|--------|------|--------|
| Warrior | Arms | 900100-900109 (10) | 900110-900132 (23) | getestet |
| Warrior | Fury | 900134-900145 (12) | 900146-900165 (20) | getestet |
| Warrior | Prot | 900168-900175 (8) | 900176-900199 (24) | implementiert |
| Paladin | Holy | 900200-900210 (11) | 900211-900232 (22) | implementiert |
| Paladin | Prot | 900234-900241 (8) | 900242-900265 (24) | implementiert |
| Paladin | Ret | 900268-900275 (8) | 900276-900299 (24) | implementiert |
| DK | Blood | 900300-900304 (5) | 900305-900332 (28) | implementiert |
| DK | Frost | 900333 (1) | 900334-900365 (32) | implementiert |
| DK | Unholy | 900366-900367 (2) | 900368-900399 (32) | implementiert |

---

### Warrior — Arms (900100-900132)

| # | Spell ID | Effekt | Ansatz | Status |
|---|----------|--------|--------|--------|
| 1 | 900100 | Mortal Strike damage +50% | DBC | getestet |
| 2 | 900101 | Mortal Strike cd -2sec | DBC | getestet |
| 3 | 900102 | Overpower damage +50% | DBC | getestet |
| 4 | 900103 | Overpower +9 targets (AoE) | DBC+C++ | getestet |
| 5 | 900104 | Mortal Strike +9 targets (AoE) | DBC+C++ | getestet |
| 6 | 900105 | 20% Crit-Chance → Enhanced Execute (5 stacks) | C++ | getestet |
| 7 | 900106 | Enhanced Execute (Damage Spell = Paragon Strike) | C++ | getestet |
| 8 | 900107 | Phys. Damage → Bladestorm CD -0.5s | C++ | getestet |
| 9 | 900108 | Whirlwind unlimited targets | DBC | getestet |
| 10a | 900109 | WW 1 target → autocast Overpower | C++ | getestet |
| 10b | *neue ID* | Thunderclap → Rend + 5× Sunder Armor | C++ | geplant |

**TC→Rend+Sunder Details**: SpellScript auf Thunderclap. `AfterHit` pro Target: Rend casten + 5× Sunder Armor applyen. Wird auch von Prot geteilt. Braucht eigene Spell-ID (passive Marker-Aura) + Hook auf Original-TC oder eigene TC-Variante.

### Warrior — Fury (900133-900165)

| # | Spell ID | Effekt | Ansatz | Status |
|---|----------|--------|--------|--------|
| 1 | 900134 | Bloodthirst +50% damage | DBC | getestet |
| 2 | 900135 | Bloodthirst +9 targets (AoE) | DBC+C++ | getestet |
| 3 | 900136 | Whirlwind damage +50% | DBC | getestet |
| 4 | 900137 | Whirlwind unlimited targets | DBC | getestet |
| 5 | 900138 | Cleave unlimited targets | DBC | getestet |
| 6 | 900139 | 20% Auto-Attack → free Whirlwind | C++ | getestet |
| 7 | 900140 | BT → next WW +50% damage (Bloody WW passive) | C++ | getestet |
| 8 | 900141 | WW resets Bloodthirst CD | C++ | getestet |
| 9 | 900142+900144 | WW 1 target → autocast Overpower | C++ | getestet |
| — | 900143+900145 | WW 1 target → autocast Slam | C++ | getestet |
| 10 | (DBC) | Remove WW stance requirement | DBC | getestet |

### Warrior — Prot (900166-900199)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900168 | Revenge +50% damage | DBC | implementiert | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x400 (Revenge). spell_dbc Entry. |
| 2 | 900169 | Revenge unlimited targets | C++ | implementiert | Passive Marker-Aura (DUMMY). C++ SpellScript auf Revenge (57823): `AfterHit` → `AnyUnfriendlyUnitInObjectRangeCheck(8yd)` → DealDamage auf alle Feinde. Prüft `HasAura(900169)`. |
| 3 | 900170 | Thunderclap → Rend + 5× Sunder Armor | C++ | implementiert | Passive Marker-Aura (DUMMY). C++ SpellScript auf TC (47502): `AfterHit` pro Target → CastSpell(Rend 47465) + 5× CastSpell(SunderArmor 58567). Prüft `HasAura(900170)`. |
| 4 | 900171 | Thunderclap +50% damage | DBC | implementiert | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x80 (TC). spell_dbc Entry. |
| 5 | 900172 | AoE damage on Block | C++ | implementiert | Proc-Aura (DUMMY) mit spell_proc: ProcFlags=0x2, 100% Chance, 1s ICD. C++ HandleProc: Prüft `PROC_HIT_BLOCK` → CastSpell(900174). |
| 6 | 900173 | 10% Block → Enhanced Thunderclap | C++ | implementiert | Proc-Aura (DUMMY) mit spell_proc: ProcFlags=0x2, 10% Chance, 3s ICD. C++ HandleProc: Prüft `PROC_HIT_BLOCK` → CastSpell(900175). |
| H1 | 900174 | Helper: Block AoE Damage | DBC | implementiert | Instant AoE Physical Damage. Effect=SCHOOL_DAMAGE(2), Target=SRC_AREA_ENEMY(22), BasePoints=500+100rnd, Radius=8yd. |
| H2 | 900175 | Helper: Enhanced Thunderclap | DBC | implementiert | Instant AoE Physical Damage. Effect=SCHOOL_DAMAGE(2), Target=SRC_AREA_ENEMY(22), BasePoints=1000+200rnd, Radius=10yd. |

> **Hinweis Prot**: 900168/900171 sind rein DBC (kein C++ nötig). 900169/900170 hooken auf bestehende Spells (57823/47502) und prüfen Marker-Aura via HasAura. 900172/900173 nutzen das Proc-System mit Block-Detection. SpellFamilyFlags für Revenge (0x400) und TC (0x80) müssen in-game verifiziert werden.

---

### Paladin — Holy (900200-900232)

> Paladin SpellFamilyName = 10. Holy Shock (20473) ist ein Dummy-Spell der auf Damage (25912+) oder Heal (25914+) routet. Scripts hooken auf die Damage/Heal-Varianten via negative spell_script_names IDs (-25912, -25914 = alle Ränge).

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900900 | Holy Shock AoE damage | C++ | implementiert | Marker-Aura (DUMMY). C++ SpellScript auf HS-Damage (-25912): `AfterHit` → CastSpell(900208) AoE Holy Damage um Target. Prüft `HasAura(900200)`. |
| 2 | 900901 | Holy Shock AoE heal | C++ | implementiert | Marker-Aura (DUMMY). C++ SpellScript auf HS-Heal (-25914): `AfterHit` → CastSpell(900209) AoE Holy Heal um Target. Prüft `HasAura(900201)`. |
| 3 | 900902 | Holy Shock always both | C++ | implementiert | Marker-Aura (DUMMY). Zwei Scripts: (a) auf HS-Damage: `AfterHit` → auto-heal nächsten verletzten Ally (HS-Heal R7). (b) auf HS-Heal: `AfterHit` → auto-damage nächsten Feind (HS-Damage R7). Prüft `HasAura(900202)`. |
| 4 | 900933 | Holy Shock +50% | DBC | implementiert | Passive Aura: `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x200000 (Holy Shock). spell_dbc Entry. |
| 5 | 900966 | Consecration also heals | C++ | implementiert | Marker-Aura (DUMMY). C++ AuraScript auf Consecration (48819): `OnEffectPeriodic` → CastSpell(900210) AoE Holy Heal um Caster. Prüft `HasAura(900204)`. |
| 6 | 900333 | Consecration around you | DBC | implementiert | Marker-Aura. Consecration-DBC muss separat gepatcht werden (TargetA → `TARGET_DEST_CASTER`). **Shared mit Prot und Ret**. |
| 7 | 900366 | Consecration +50% | DBC | implementiert | Passive Aura: `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x20 (Consecration). |
| 8 | 900207 | Consecration +5sec | DBC | implementiert | Passive Aura: `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_DURATION` (17). BasePoints=5000ms. EffectSpellClassMaskA=0x20. |
| H1 | 900208 | Helper: HS AoE Damage | DBC | implementiert | Instant AoE Holy Damage. Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), BasePoints=800+100rnd, 10yd. |
| H2 | 900209 | Helper: HS AoE Heal | DBC | implementiert | Instant AoE Holy Heal. Effect=HEAL(10), Target=DEST_AREA_ALLY(30), BasePoints=800+100rnd, 10yd. |
| H3 | 900210 | Helper: Consec Heal Tick | DBC | implementiert | Instant AoE Holy Heal. Effect=HEAL(10), Target=SRC_AREA_ALLY(31), BasePoints=200+50rnd, 8yd. |

> **Hinweis Holy**: SpellFamilyFlags für Holy Shock (0x200000) und Consecration (0x20) müssen in-game verifiziert werden. 900205 (Consec around you) braucht zusätzlich einen DBC-Patch auf den Base-Spell (48819) um TargetA zu ändern — die Marker-Aura allein reicht nicht.

### Paladin — Prot (900233-900265)

> **ID-Shift**: Ursprünglich 900208-900238 geplant, aber 900208-900210 werden als Holy-Helper verwendet. Prot startet bei 900234.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900234 | Consecration around you | DBC | implementiert | Marker-Aura (DUMMY). Consecration-DBC muss separat gepatcht werden. **Shared mit Holy (900205) und Ret**. |
| 2 | 900235 | Avenger's Shield +9 targets | DBC | implementiert | `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_JUMP_TARGETS` (17). BasePoints=9. EffectSpellClassMaskA=0x4000. |
| 3 | 900236 | Avenger's Shield +50% damage | DBC | implementiert | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x4000. |
| 4 | 900237 | Holy Shield charges +99 | DBC | implementiert | `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_CHARGES` (4). BasePoints=99. EffectSpellClassMaskB=0x20 (flags[1]). |
| 5 | 900238 | Holy Shield +50% damage | DBC | implementiert | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskB=0x20 (flags[1]). |
| 6 | 900239 | AS leaves Consecration | C++ | implementiert | Marker + C++ auf AS (-48827): `AfterHit` → CastSpell(Consec 48819, triggered). Prüft `HasAura(900239)`. |
| 7 | 900240 | Judgement → free AS | C++ | implementiert | Marker + C++ auf Judgement Damage (54158): `AfterHit` → CastSpell(AS 48827, triggered). Prüft `HasAura(900240)`. |
| 8 | 900241 | Judgement cd -2sec | DBC | implementiert | `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_COOLDOWN` (11). BasePoints=-2000ms. EffectSpellClassMaskA=0x800000. |

> **Hinweis Prot**: SpellFamilyFlags verifizieren: AS=0x4000(flags[0]), HolyShield=0x20(flags[1]), Judgement=0x800000(flags[0]). 900237 (charges +99) muss getestet werden ob SPELLMOD_CHARGES funktioniert.

### Paladin — Ret (900266-900299)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900268 | Consecration around you | DBC | implementiert | Marker-Aura (DUMMY). **Shared Konzept mit Holy (900205) und Prot (900234)**. Separate ID pro Spec. |
| 2 | 900269 | Judgement cd -2sec | DBC | implementiert | `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_COOLDOWN` (11). BasePoints=-2000ms. EffectSpellClassMaskA=0x800000. **Shared Konzept mit Prot (900241)**. |
| 3 | 900270 | Divine Storm +6 targets | DBC | implementiert | Marker-Aura (DUMMY). DS-Base-Spell (53385) braucht DBC-Patch: `MaxAffectedTargets` auf 10. Kein SpellMod für AoE-Targets verfügbar. |
| 4 | 900271 | Divine Storm +50% damage | DBC | implementiert | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskB=0x20000 (DS flags[1], verify!). |
| 5 | 900272 | Crusader Strike +50% damage | DBC | implementiert | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x1 (CS flags[0], verify!). |
| 6 | 900273 | Crusader Strike +9 targets | C++ | implementiert | Marker-Aura (DUMMY). C++ SpellScript auf CS (-35395): `AfterHit` → DealDamage auf 9 Extra-Feinde in 8yd. Prüft `HasAura(900273)`. |
| 7 | 900274 | CS/Judge/DS → Exorcism buff | C++ | implementiert | Passive Proc-Aura (DUMMY). spell_proc: ProcFlags=0x10, 100%. C++ CheckProc filtert auf CS(35395)/Judge(54158)/DS(53385). HandleProc → CastSpell(900275). |
| H1 | 900275 | Exorcism Power (buff) | DBC | implementiert | Stacking Buff: `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). +50% Exorcism dmg pro Stack, max 10. 30s Duration. Consumed by Exorcism (48801) via C++ AfterCast. |

> **Hinweis Ret**: SpellFamilyFlags verifizieren: DS=0x20000(flags[1]), CS=0x1(flags[0]), Exorcism=0x200000(flags[0]). 900270 (DS +6 targets) braucht DBC-Patch auf Base-Spell. Exorcism-Buff (900275) EffectSpellClassMaskA=0x200000 muss korrekt Exorcism matchen.

---

### DK — Blood (900300-900332)

> DK SpellFamilyName = 15. Dancing Rune Weapon (49028) beschwört ein Rune Weapon (NPC 27893) das Spells des DK kopiert.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900300 | 3 Rune Weapons gleichzeitig | C++ | implementiert | Marker-Aura (DUMMY). C++ SpellScript auf DRW (49028): `AfterCast` → 2× CastSpell(DRW, triggered=true). Prüft `HasAura(900300)`. |
| 2 | 900301 | Rune Weapon double-casts | C++ | implementiert | Marker-Aura (DUMMY). C++ AuraScript auf DRW (49028): `OnEffectProc` → extra CastSpell/DealMeleeDamage. Prüft `HasAura(900301)`. Rune Weapon castet jeden Spell 2× statt 1×. |
| 3 | 900302 | Heart Strike +50% damage | DBC | implementiert | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x2000000 (HS flags[0], verify!). |
| 4 | 900303 | Heart Strike +9 targets | C++ | implementiert | Marker-Aura (DUMMY). C++ SpellScript auf HS (-55050): `AfterHit` → DealDamage auf 9 Extra-Feinde in 8yd. Prüft `HasAura(900303)`. |
| 5 | 900304 | Dealing damage → chance Death Coil | C++ | implementiert | Proc-Aura (DUMMY). spell_proc: ProcFlags=0x14, 15% Chance, 3s ICD. C++ HandleProc → CastSpell(47632 Death Coil Damage, triggered). |

### DK — Frost (900333-900365)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900333 | Ghoul → Frost Wyrm | C++ | implementiert | Marker-Aura (DUMMY). C++ SpellScript auf Raise Dead (46584): `AfterCast` → SetDisplayId(26752) + SetObjectScale(0.5) auf Ghoul (NPC 26125). Prüft `HasAura(900333)`. **Hinweis**: Nur visueller Replace. Für eigene AI/Spells braucht es custom creature_template. |

### DK — Unholy (900366-900399)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900366 | DoTs → Shadow AoE proc | C++ | implementiert | Proc-Aura (DUMMY). spell_proc: ProcFlags=0x400000 (DONE_PERIODIC), 20% Chance, 2s ICD. C++ HandleProc → CastSpell(900367, triggered) auf DoT-Target. |
| H1 | 900367 | Shadow Eruption (helper) | DBC | implementiert | Instant AoE Shadow Damage. Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), SchoolMask=32(Shadow), BasePoints=600+150rnd, 8yd. |

> **Hinweis DK**: Heart Strike SpellFamilyFlags[0]=0x2000000 verifizieren! 900300 (3 DRW) castet DRW nochmal als triggered — kann zu Aura-Stacking-Issues führen wenn nicht korrekt behandelt. 900333 (Frost Wyrm) ist aktuell nur ein DisplayID-Swap; für echte Frost-AI braucht es creature_template + CreatureScript.

---

### Shaman — Elemental (900400-900432)

> Shaman SpellFamilyName = 11. Totem-Replacement ist ein Kern-Feature für alle 3 Specs — jede Spec braucht eigene Variante da unterschiedliche Totems/Creatures relevant sind.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900400 | Chain Lightning +6 targets, no damage reduction | DBC/C++ | geplant | Chain Lightning (421/CL ranks) hat normalerweise 3 Bounces mit 30% Reduce pro Bounce. DBC: `EffectChainTarget` auf 9 erhöhen. Damage-Reduction entfernen: C++ SpellScript `CalculateChainDamage` oder Aura `SPELL_AURA_ADD_PCT_MODIFIER` die Chain-Penalty auf 0 setzt. |
| 2 | 900401 | Totems replaced by creatures that follow/fight (Ele) | C++ | geplant | Großes System: Totem-Summon-Spells abfangen (`SpellScript`), statt Totem-NPC ein Creature spawnen das dem Caster folgt (`FollowMovementGenerator`) und kämpft. Braucht eigene AI-Scripts für jedes Totem-Creature. Creature behält Totem-Effekte (Auren) aber ist mobil + angreifbar. Evtl. eigene NPCs pro Totem-Typ. |
| 3 | 900402 | Fire Elemental Totem → Ragnaros | C++ + DBC | geplant | Fire Elemental Totem (2894) beschwört Fire Elemental (15438). Override: Neuen NPC "Ragnaros" erstellen mit eigenem Model (DisplayID vom Ragnaros-Boss, z.B. 11121), eigener AI mit Fire-Spells + evtl. On-Hit-Proc. SpellScript auf Totem-Spell → Summon Ragnaros-NPC statt Fire Elemental. Balancing: Stats/Duration anpassen. |
| 4 | 900403 | Lightning Overload chance erhöht + wirkt auf Lava Burst | C++ | geplant | Lightning Overload (Talent 30675) proct auf Lightning Bolt/Chain Lightning. (a) Chance erhöhen: Modify Proc-Chance des Talents via Aura. (b) Lava Burst hinzufügen: SpellScript oder `spell_proc` erweitern → Overload-Proc auch bei Lava Burst (51505) auslösen. Braucht evtl. eigenen Proc-Handler. |
| 5 | 900404 | Lava Burst spreads Fire Shock DoT | C++ | geplant | SpellScript auf Lava Burst `AfterHit`: Prüfe ob Target Fire Shock (8050) hat → `GetCaster()->CastSpell()` Fire Shock auf alle Feinde im Radius (z.B. 10yd) um Target. Oder: Iterate nahe Enemies und appliziere Fire Shock Aura. Ähnlich wie Warrior Rend-Spread. |
| 6 | 900405 | Flame Shock ticks chance to reset Lava Burst CD | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_PERIODIC` (0x400000), SpellFamilyMask für Flame Shock. `HandleProc`: Chance X% → `GetCaster()->GetSpellHistory()->ResetCooldown(51505, true)` (Lava Burst). `spell_proc` mit Chance + ICD. |
| 7 | 900406 | Lava Burst two charges | DBC/C++ | geplant | Charge-System: DBC `ChargeCategory` + `CategoryCharges` = 2 für Lava Burst. Oder C++: Eigene Aura die bei Lava Burst-CD eine zweite "Aufladung" trackt und CD sofort resetet wenn noch Charges übrig. Charge-System in WotLK ist limitiert → vermutlich C++ nötig. |
| 8 | 900407 | Clearcasting → Lava Burst instant | DBC | geplant | Clearcasting (Elemental Focus, 16246) gibt Buff. Passive Aura: Wenn Clearcasting aktiv → `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_CASTING_TIME` = -100% auf Lava Burst (SpellFamilyMask). Lava Burst hat bereits 2s Cast. |

### Shaman — Enhancement (900433-900465)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900433 | Totems replaced by summons that follow/fight (Enhance) | C++ | geplant | Gleiche Kern-Mechanik wie 900401 (Ele Totem-Replacement), aber Enhance-spezifische Creatures mit Melee-AI statt Caster-AI. Shared Code-Basis mit 900251 möglich. |
| 2 | 900434 | 5 Maelstrom stacks → summons empowered, AoE on attack 5s | C++ | geplant | Hook auf Maelstrom Weapon (53817) Stack-Erreichen (5 Stacks). Bei 5 Stacks: Buff auf alle aktiven Summons → `SPELL_AURA_PROC_TRIGGER_SPELL` oder AI-Flag das AoE-Damage bei jedem Angriff für 5s auslöst. Braucht Tracking welche Summons aktiv sind. |
| 3 | 900435 | Summons +50% damage | DBC/C++ | geplant | Passive Aura auf Caster: Alle Summons (Pets/Guardians) erhalten `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` +50%. Oder: Aura auf Summon-Creatures via Owner-Check. |
| 4 | 900436 | Auto attacks chance to summon a wolf | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4). `HandleProc`: Chance X% → `CastSpell(SummonWolf)`. Wolf-NPC: temporärer Guardian mit Timer (z.B. 15s), Melee-AI, Wolf-Model. Braucht eigenen NPC + Summon-Spell. |
| 5 | 900437 | Spirit Wolves inherit your haste | C++ | geplant | Feral Spirit (51533) beschwört 2 Spirit Wolves (29264). CreatureScript/PetAI: Bei Summon → Owner Haste-Rating auslesen → Attackspeed des Wolves anpassen via `ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME)`. Evtl. periodisch updaten falls Caster-Haste sich ändert. |
| 6 | 900438 | Spirit Wolves 5% chance Chain Lightning on melee hit | C++ | geplant | CreatureScript für Spirit Wolf (29264): `OnMeleeHit` oder Proc-Aura auf Wolf → 5% Chance → `CastSpell(Chain Lightning, triggered=true)`. Chain Lightning Rank basierend auf Caster-Level. |

### Shaman — Restoration (900466-900499)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900466 | Totems replaced by creatures that follow/fight (Resto) | C++ | geplant | Gleiche Kern-Mechanik wie 900401/900433, aber Resto-spezifische Creatures mit Healer/Support-AI. Healing-Totems (Healing Stream etc.) werden zu folgenden Heal-Creatures. Shared Code-Basis. |
| 2 | 900467 | Mana regen +2% per missing mana % | C++ | geplant | Passive Aura mit `SPELL_AURA_OBS_MOD_POWER` oder custom `HandlePeriodicManaRegen`. Berechnung: `missingManaPct = 1 - (currentMana / maxMana)` → `bonusRegen = missingManaPct * 2% * baseRegen`. Bei 50% Mana → +100% Regen, bei 10% Mana → +180% Regen. Skaliert umgekehrt proportional. |

> **Helper-Spells Shaman**: 900401/900433/900466 (Totem-Replacement) ist ein gemeinsames System — braucht evtl. mehrere Helper-NPCs pro Totem-Typ (Fire, Earth, Water, Air Totem-Creatures). 900436 (Wolf-Summon) braucht eigenen Wolf-NPC + Summon-Spell. 900434 (Maelstrom Empower) braucht AoE-Buff-Spell für Summons. IDs ab 900468+ für Helper verfügbar.

> **Besonders aufwändig**: Das Totem-Replacement-System (900401/900433/900466) ist das ambitionierteste Feature bisher — jedes Totem braucht ein eigenes folgendes Creature mit passender AI. Ragnaros-Replacement (900252) braucht Custom-Boss-NPC mit Fire-AI. Spirit Wolf Haste-Inheritance (900262) braucht dynamische Stat-Synchronisation.

---

### Hunter — Shared (900500-900532)

> Hunter SpellFamilyName = 9. "Get back arrows" und "Multishot unlimited targets" gelten für alle 3 Specs → shared Spells.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900900 | Get back arrows (no ammo consumption) | C++ | geplant | Hunter verbraucht Ammo bei jedem Ranged-Angriff. Ansatz: `PlayerScript::OnBeforeItemUse` oder `SpellScript` auf Auto Shot (75) → nach Schuss Ammo-Count wiederherstellen. Oder: Passive Aura die Ammo-Verbrauch auf 0 setzt. Core-seitig: `Player::RemoveAmmo()` Hook → Skip wenn Aura aktiv. Alternativ einfacher: periodisch Ammo-Stack auf Max setzen. |
| 2 | 900901 | Multi-Shot unlimited targets | DBC/C++ | geplant | Multi-Shot (2643) trifft normalerweise 3 Ziele. DBC: `MaxAffectedTargets` entfernen/sehr hoch setzen. C++: `OnObjectAreaTargetSelect` → keine Target-Limit-Filterung. Spell trifft dann alle Feinde im Cone/Radius. |

### Hunter — Beast Mastery (900500-900532)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900902 | Pet damage +50% | DBC | geplant | Passive Aura auf Hunter: `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` +50% für Pet. Oder: Aura mit `SPELL_AURA_MOD_DAMAGE_DONE_FOR_MECHANIC` auf Pet anwenden. SpellFamilyName=9, Target=Pet. Ähnlich wie bestehende Pet-Talent-Buffs (z.B. Unleashed Fury). |
| 2 | 900933 | Pet attack speed +50% | DBC | geplant | Passive Aura: `SPELL_AURA_MOD_ATTACKSPEED` -50% (= 50% schneller) auf Pet. Target: Owner's Pet. Vergleichbar mit Serpent's Swiftness (Talent) aber stärker. |
| 3 | 900966 | Pet chance to deal AoE damage | C++ | geplant | Proc-Aura auf Pet: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4). `HandleProc`: Chance X% → CastSpell(AoE-Damage-Spell, triggered=true) zentriert auf Pet's Target. Braucht Helper-AoE-Spell (z.B. 900505). `spell_proc` mit Chance + ICD. |

### Hunter — Marksmanship (900533-900565)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900533 | Auto Shot bounces +9 targets | C++ | geplant | Auto Shot (75) ist normalerweise Single-Target. SpellScript `AfterHit`: Bei Auto Shot → Chain-Bounce-Logik: Finde bis zu 9 weitere Feinde im Radius um Target → CastSpell(Auto-Shot-Damage, triggered=true) auf jedes. Damage kann pro Bounce gleich bleiben (kein Reduce wie Chain Lightning). Braucht Helper-Damage-Spell (z.B. 900535) für den Bounce-Damage. |
| 2 | 900534 | Autocast Multi-Shot every 0.1s for 2s, 50% slow while channeling | C++ | geplant | Aktivierter Spell: Beim Cast → Channel-Aura (2s Duration). Periodic Tick alle 100ms → CastSpell(Multi-Shot, triggered=true). Gleichzeitig: `SPELL_AURA_MOD_SPEED_SLOW_ALL` -50% auf Caster während Channel. 20 Multi-Shots in 2s = extrem hoher Burst. Braucht eigene Channel-Spell-DBC + PeriodicTrigger-Logik. Balancing-Warnung: Enormer Damage-Output. |

### Hunter — Survival (900566-900599)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900566 | Chance to drop Explosive Trap on damage dealt | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_RANGED_AUTO_ATTACK|DONE_SPELL_RANGED_DMG_CLASS` (0x44). `HandleProc`: Chance X% → CastSpell(Explosive Trap, triggered=true) an Target-Position. Explosive Trap (49067) normalerweise am Caster platziert — braucht evtl. modifizierte Version die am Ziel spawnt (`SummonGameObject` an Target-Coords). Oder: Direkt den Explosion-Damage-Spell (49064) am Target casten. |

> **Helper-Spells Hunter**: 900304 (Pet AoE) braucht AoE-Damage-Spell (z.B. 900505). 900533 (Auto Shot Bounce) braucht Bounce-Damage-Spell (z.B. 900535). 900534 (Multi-Shot Channel) braucht eigene Channel-Spell-DBC-Entry. 900566 (Explosive Trap on Damage) braucht evtl. modifizierte Trap-Spell-Variante. IDs ab 900505+ für Helper verfügbar.

> **Besonders aufwändig**: 900534 (Multi-Shot Autocast Channel) ist ein neuartiges Konzept — 20 Multi-Shots in 2s braucht sorgfältiges Balancing und Performance-Tests (viele Projectiles gleichzeitig). 900533 (Auto Shot Bounce) ist ein Custom-Chain-System das es im Base-Game nicht gibt.

---

### Rogue — Shared (900600-900632)

> Rogue SpellFamilyName = 8. "Energy regen +50%" gilt für alle 3 Specs → shared Spell.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900600 | Energy regeneration +50% | DBC | geplant | Passive Aura: `SPELL_AURA_MOD_POWER_REGEN_PERCENT` +50% für Energy (Power Type 3). Rogue Base Energy Regen = 10/s → wird 15/s. Vergleichbar mit Vitality-Talent. SpellFamilyName=8, Target=Self. |

### Rogue — Assassination (900600-900632)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900601 | Mutilate damage +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Mutilate (SpellFamilyFlags für Mutilate 5374/27576). Oder `SPELL_AURA_MOD_DAMAGE_DONE_FOR_MECHANIC` spezifisch. Einfacher Damage-Multiplikator. |
| 2 | 900602 | Poison damage +50% | DBC/C++ | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf alle Poison-Spells (Instant Poison 57965, Deadly Poison 57970, Wound Poison 57975, etc.). Alle Poison-Spells haben gemeinsame SpellFamilyFlags → ein Aura-Effekt kann mehrere abdecken. Alternativ C++: `OnDamage`-Hook → Wenn Damage-Spell School=Nature + SpellFamily=Rogue → ×1.5. |
| 3 | 900603 | Poison damage has chance to unleash Poison Nova | C++ | geplant | Proc-Aura: `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS|NEG` mit SpellFamily-Filter auf Poison-Spells. `HandleProc`: Chance X% → CastSpell(Poison Nova Helper, triggered=true) am Target. Poison Nova = AoE Nature Damage um Target. Braucht Helper-AoE-Spell (z.B. 900604). ICD empfohlen (z.B. 3s). |

### Rogue — Combat (900633-900665)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900633 | Sinister Strike damage +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Sinister Strike (48638). SpellFamilyFlags für SS identifizieren. Einfacher Damage-Multiplikator. |
| 2 | 900634 | Sinister Strike +9 targets | C++/DBC | geplant | Sinister Strike (48638) ist Single-Target Melee. Ansatz: SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Melee-Radius. CastSpell(SS-Damage-Helper, triggered=true) auf jedes Target. Ähnlich wie Hunter Auto Shot Bounce (900305). Braucht Helper-Spell (z.B. 900637). |
| 3 | 900635 | Blade Dance (Blade Flurry) duration 2 min | DBC | geplant | Blade Flurry (13877) hat normalerweise 15s Duration. DBC-Patch: Duration auf 120000ms (2 min) setzen. Einfache DBC-Änderung. |
| 4 | 900636 | Blade Dance (Blade Flurry) +9 targets | C++/DBC | geplant | Blade Flurry (13877) trifft normalerweise 1 zusätzliches Ziel. DBC: `MaxAffectedTargets` auf 10 setzen. C++: Falls Target-Limit hardcoded → `OnObjectAreaTargetSelect` überschreiben. Blade Flurry kopiert Melee-Damage auf zusätzliche Targets → alle 9 extra Targets bekommen den Damage. |

### Rogue — Subtlety (900666-900699)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900666 | Hemorrhage damage +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Hemorrhage (48660). SpellFamilyFlags für Hemorrhage. Einfacher Damage-Multiplikator wie Mutilate (900351). |
| 2 | 900667 | Hemorrhage +9 targets | C++ | geplant | Hemorrhage (48660) ist Single-Target Melee. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Melee-Radius. CastSpell(Hemorrhage-Damage-Helper, triggered=true) auf jedes. Gleicher Ansatz wie SS +9 (900355). Braucht Helper-Spell (z.B. 900668). |

> **Helper-Spells Rogue**: 900353 (Poison Nova) braucht AoE-Nature-Damage-Spell (z.B. 900604). 900634 (SS +9) braucht Chain-Damage-Helper (z.B. 900637). 900667 (Hemorrhage +9) braucht Chain-Damage-Helper (z.B. 900668). IDs ab 900604+ für Helper verfügbar.

> **Besonders aufwändig**: 900634 (SS +9 targets) und 900667 (Hemorrhage +9 targets) sind Custom-Chain-Melee-Systeme — Melee-Abilities auf mehrere Targets verteilen ist ungewöhnlich im Base-Game. 900636 (Blade Flurry +9 targets) könnte Performance-intensiv sein bei vielen Mobs.

---

### Mage — Shared (900700-900732)

> Mage SpellFamilyName = 3. "Channeling Evocation increases spell damage" gilt für alle 3 Specs → shared Spell.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900700 | Channeling Evocation increases spell damage | C++ | geplant | Evocation (12051) ist ein Channel-Spell (8s). Ansatz: AuraScript auf Evocation → `OnApply`: Buff mit `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` (ALL_SCHOOLS) auf Caster stacken, pro Tick steigend. `OnRemove`: Buff bleibt X Sekunden oder permanent. Alternativ: Während Channel stacking Aura, z.B. +10% Spell Damage pro Sekunde Channel → bis +80% nach vollem Channel. Duration des Buffs nach Channel konfigurierbar. |

### Mage — Arcane (900700-900732)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900701 | Mana regen +2% per missing mana % | C++ | geplant | Dynamische Mana-Regen-Skalierung. `PlayerScript::OnUpdateManaRegen` oder periodic Aura-Tick: Berechne fehlende Mana% → setze Regen-Bonus = fehlende% × 2%. Bei 50% Mana fehlen → +100% Mana Regen. Bei 90% fehlen → +180%. Passive Aura mit `SPELL_AURA_OBS_MOD_POWER` oder C++ Hook auf `Player::RegenerateMana()`. Sehr starke Mana-Sustain-Mechanik. |
| 2 | 900702 | Arcane Barrage +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Arcane Barrage (44781). SpellFamilyName=3, SpellFamilyFlags für Barrage. Einfacher Damage-Multiplikator. |
| 3 | 900703 | Arcane Barrage +9 targets | DBC/C++ | geplant | Arcane Barrage (44781) trifft normalerweise 3 Targets. DBC: `MaxAffectedTargets` auf 10+ setzen. Oder C++: `OnObjectAreaTargetSelect` → Target-Limit entfernen. |
| 4 | 900704 | Arcane Blast cast time -50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` (SPELLMOD_CASTING_TIME) -50% auf Arcane Blast (42897). Base Cast Time 2.5s → 1.25s. Stapelt mit Arcane Blast Debuff (noch schneller mit Stacks). |
| 5 | 900705 | Arcane Blast +9 targets | C++ | geplant | Arcane Blast (42897) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Radius. CastSpell(AB-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900710). |
| 6 | 900706 | Arcane Charges stack up to 8 | DBC/C++ | geplant | Arcane Blast Debuff (36032) stackt normalerweise bis 4. DBC: `StackAmount` auf 8 setzen. C++: Falls hardcoded → `AuraScript::OnStackChange` → Allow stacks >4. Jeder Stack erhöht AB Damage +15% und Mana Cost +150% (Base-Werte). 8 Stacks = +120% Damage, +1200% Mana Cost. Balancing beachten! |
| 7 | 900707 | Arcane Explosion generates 1 Arcane Charge (like Arcane Blast) | C++ | geplant | Arcane Explosion (42921) ist AoE Instant. SpellScript `AfterCast` → ApplyAura(Arcane Blast Debuff 36032, 1 Stack) auf Caster. Gleiche Mechanik wie AB aber ohne Consume. AE wird zu einem AoE Arcane Charge Generator. |
| 8 | 900708 | Below 30% health → activate Mana Shield + restore all mana | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_TAKEN_DAMAGE` (0x4000). `HandleProc`: Wenn Health <30% → CastSpell(Mana Shield 43020, triggered=true) + SetPower(MANA, MaxMana). ICD empfohlen (z.B. 60s) um Abuse zu verhindern. Sehr starke Überlebens-Mechanik: Volle Mana + Shield bei Low HP. |
| 9 | 900709 | Blink target location selection | C++ | geplant | Blink (1953) teleportiert normalerweise 20yd vorwärts. Ansatz: Override Blink → Click-to-Blink mit Target-Location. SpellScript `HandleDummy`: Lese SpellDestination → Teleport Caster dorthin (max Range z.B. 40yd). DBC: Spell Target-Type auf `TARGET_DEST_DEST` ändern. Braucht Client-seitig: Spell zeigt Ground-Target-Cursor. Vergleichbar mit Heroic Leap Targeting. |

### Mage — Fire (900733-900765)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900733 | Fireball +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Fireball (42833). SpellFamilyName=3, SpellFamilyFlags für Fireball. Einfacher Damage-Multiplikator. |
| 2 | 900734 | Fireball +9 targets | C++ | geplant | Fireball (42833) ist Single-Target Projectile. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Radius. CastSpell(Fireball-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900739). |
| 3 | 900735 | Pyroblast +9 targets | C++ | geplant | Pyroblast (42891) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(Pyro-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900740). |
| 4 | 900736 | Pyroblast +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Pyroblast (42891). Einfacher Damage-Multiplikator. |
| 5 | 900737 | Fire Blast off GCD and usable while casting | DBC/C++ | geplant | Fire Blast (42873): DBC → `StartRecoveryCategory` = 0 (off GCD). Attribute `SPELL_ATTR4_CAN_CAST_WHILE_CASTING` setzen. Ermöglicht Fire Blast als Weave-Spell während Fireball/Pyroblast Cast. Vergleichbar mit Retail-Fire-Mage Design. |
| 6 | 900738 | Pyroblast now triggers Hot Streak | C++ | geplant | Hot Streak (48108) proct normalerweise bei 2 aufeinanderfolgenden Crits. Ansatz: Pyroblast Cast (auch non-crit) → automatisch Hot Streak Buff (48108) applyen. SpellScript auf Pyroblast `AfterCast` → AuraScript: ApplyAura(Hot Streak). Effekt: Jeder Pyroblast gibt guaranteed nächsten Instant Pyroblast. Extrem starker Damage-Loop! |

### Mage — Frost (900766-900799)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900766 | Frostbolt +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Frostbolt (42842). Einfacher Damage-Multiplikator. |
| 2 | 900767 | Frostbolt +9 targets | C++ | geplant | Frostbolt (42842) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(Frostbolt-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900772). |
| 3 | 900768 | Ice Lance +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Ice Lance (42914). Einfacher Damage-Multiplikator. |
| 4 | 900769 | Ice Lance +9 targets | C++ | geplant | Ice Lance (42914) ist Single-Target Instant. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(Ice-Lance-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900773). |
| 5 | 900770 | Water Elemental is permanent | DBC/C++ | geplant | Summon Water Elemental (31687) hat normalerweise 45s Duration. DBC: Duration auf -1 (permanent) oder sehr hohen Wert setzen. C++: Falls Duration hardcoded → `OnSummon` Hook → SetDuration(0) (permanent). Ähnlich wie Hunter Pet — Water Ele bleibt bis Tod/Dismiss. |
| 6 | 900771 | Frost Comet Shower spell | C++ | geplant | Neuer aktiver Spell: Channel oder Instant → Spawnt mehrere Frost-Comets an zufälligen Positionen im Target-Bereich über X Sekunden. Jeder Comet = AoE Frost Damage + Slow. Implementierung: Periodic Trigger → SummonGameObject oder CastSpell(Comet-Impact-Helper) an Random-Position im Radius. Visuell: Blizzard-ähnlich aber mit größeren Einzel-Impacts. Braucht Helper-Spells + ggf. Custom Visual. Vergleichbar mit Meteor (Fire) aber Frost-themed. |

> **Helper-Spells Mage**: 900705 (AB +9) → Helper 900422. 900734 (Fireball +9) → Helper 900423. 900735 (Pyro +9) → Helper 900424. 900767 (Frostbolt +9) → Helper 900425. 900769 (Ice Lance +9) → Helper 900426. 900771 (Comet Shower) braucht Impact-Helper + evtl. Visual-Spells. IDs ab 900710+ für Helper verfügbar.

> **Besonders aufwändig**: 900701 (Mana Regen Scaling) braucht dynamische Berechnung pro Regen-Tick. 900709 (Blink Target Location) braucht Client-seitig Ground-Target-Cursor — evtl. DBC-Spell-Target-Type-Patch nötig. 900738 (Pyro → Hot Streak Loop) ist ein guaranteed Instant-Pyro-Chain — extrem starker Burst, Balancing kritisch. 900771 (Frost Comet Shower) ist ein komplett neuer Spell mit Custom-Visuals. Fire Meteor (nicht explizit als ID, wird in 900421-Beschreibung referenziert) ist ähnlich aufwändig.

---

### Warlock — Affliction (900800-900832)

> Warlock SpellFamilyName = 5. Affliction fokussiert auf DoT-Verstärkung und DoT-Spread.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900800 | DoTs have a chance to deal shadow damage AoE | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_PERIODIC` (0x400000). `HandleProc`: Wenn Periodic-Damage-Tick von Warlock-DoT → Chance X% → CastSpell(Shadow-AoE-Helper, triggered=true) zentriert auf DoT-Target. Shadow AoE = Area Damage um das Target. Braucht Helper-AoE-Spell (z.B. 900803). ICD empfohlen (z.B. 2s). |
| 2 | 900801 | Corruption +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Corruption (47813). SpellFamilyName=5, SpellFamilyFlags für Corruption. Einfacher Damage-Multiplikator für Periodic + Initial Damage. |
| 3 | 900802 | DoTs have a chance to spread to 2 additional targets on tick | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_PERIODIC`. `HandleProc`: Wenn DoT-Tick → Chance X% → Finde 2 nächste Feinde im Radius die den DoT NICHT haben → ApplyAura(gleicher DoT) auf sie. Muss DoT-Spell-ID aus ProcEventInfo extrahieren und auf neue Targets casten. Sehr starke Spread-Mechanik — kann exponentiell wachsen! Braucht evtl. Max-Target-Cap pro Cast. |

### Warlock — Demonology (900833-900865)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900833 | Killing an enemy increases Demon Form duration | C++ | geplant | Metamorphosis (47241) hat 30s Duration. `PlayerScript::OnKill` oder `KillCredit`-Hook: Wenn Player in Metamorphosis (HasAura 47241) → ExtendAura Duration um X Sekunden (z.B. +5s pro Kill). `Aura::SetDuration(GetDuration() + 5000)`. Kein Cap oder mit Cap (z.B. max 120s). Farming-Sustain-Mechanik. |
| 2 | 900834 | Demon Form: periodic shadow AoE + self heal | C++ | geplant | Passive Aura aktiv nur während Metamorphosis (47241). Periodic Tick alle X Sekunden → CastSpell(Shadow-AoE-Helper, triggered=true) um Caster + CastSpell(Heal-Helper, triggered=true) auf Caster. Heal = % des Damage dealt oder flat. Ansatz: AuraScript auf Metamorphosis → `OnPeriodicTick` oder separater Periodic-Trigger-Spell der nur aktiv ist wenn Meta-Aura vorhanden. |
| 3 | 900835 | Demons have chance to spawn lesser version of themselves | C++ | geplant | Proc-Aura auf Warlock-Pet: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4). `HandleProc`: Chance X% → SummonCreature(Lesser-Version-NPC) an Pet-Position. Lesser Version: Temporärer NPC (z.B. 30s Duration), reduzierte Stats (50% HP/Damage), gleicher NPC-Typ aber mit Suffix. Braucht Custom Creature-Templates pro Pet-Typ (Lesser Imp, Lesser Felguard, etc.). ICD empfohlen (z.B. 30s). |
| 4 | 900836 | Imp Firebolt +50% damage | DBC | geplant | Passive Aura auf Warlock (transferred to Pet): `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Imp Firebolt (47964). Oder: Aura direkt auf Pet via Owner-Aura-Scaling. Einfacher Damage-Multiplikator. |
| 5 | 900837 | Imp Firebolt +9 targets | C++ | geplant | Imp Firebolt (47964) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Radius. CastSpell(Firebolt-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900841). Imp wird zum AoE-Caster. |
| 6 | 900838 | Felguard AoE unlimited targets | DBC/C++ | geplant | Felguard Cleave (47994) trifft normalerweise begrenzte Targets. DBC: `MaxAffectedTargets` entfernen/sehr hoch. C++: `OnObjectAreaTargetSelect` → kein Target-Limit. |
| 7 | 900839 | Felguard +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` +50% auf Felguard (alle Schools). Applied via Owner-to-Pet Aura Scaling. Einfacher Damage-Multiplikator für alle Felguard-Abilities. |
| 8 | 900840 | Sacrificing pet grants ALL pet bonuses | C++ | geplant | Demonic Sacrifice (18788) gibt normalerweise einen Buff abhängig vom geopferten Pet-Typ (Imp→Fire Dmg, VW→HP Regen, etc.). Ansatz: SpellScript Override → Beim Sacrifice → ApplyAura für ALLE Pet-Typ-Buffs gleichzeitig (Imp-Bonus + VW-Bonus + Succubus-Bonus + Felhunter-Bonus + Felguard-Bonus). Braucht Liste aller Sacrifice-Buff-IDs und Apply aller auf einmal. |

### Warlock — Destruction (900866-900899)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900866 | Shadow Bolt +9 targets | C++ | geplant | Shadow Bolt (47809) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(SB-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900871). |
| 2 | 900867 | Shadow Bolt +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Shadow Bolt (47809). Einfacher Damage-Multiplikator. |
| 3 | 900868 | Chaos Bolt +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Chaos Bolt (59172). Einfacher Damage-Multiplikator. |
| 4 | 900869 | Chaos Bolt cooldown -2 sec | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_FLAT_MODIFIER` (SPELLMOD_COOLDOWN) -2000ms auf Chaos Bolt (59172). Base CD 12s → 10s. Oder DBC direkt: `RecoveryTime` reduzieren. |
| 5 | 900870 | Chaos Bolt +9 targets | C++ | geplant | Chaos Bolt (59172) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(CB-Damage-Helper, triggered=true). Chaos Bolt ignoriert Resistances — Helper-Spell sollte das ebenfalls tun (Attribute `SPELL_ATTR0_NO_IMMUNITIES` o.ä.). Braucht Helper-Spell (z.B. 900872). |

> **Helper-Spells Warlock**: 900800 (DoT AoE) → Shadow-AoE-Helper 900466. 900837 (Imp Firebolt +9) → Helper 900467. 900866 (SB +9) → Helper 900468. 900870 (CB +9) → Helper 900469. 900835 (Lesser Demons) braucht Custom Creature-Templates. IDs ab 900803+ für Helper verfügbar.

> **Besonders aufwändig**: 900802 (DoT Spread) kann exponentiell wachsen — braucht Target-Cap um Server-Performance zu schützen. 900835 (Lesser Demon Spawn) braucht Custom Creature-Templates pro Pet-Typ mit eigener AI. 900840 (Sacrifice All Bonuses) muss alle Pet-Typ-Buffs korrekt identifizieren und stacken. 900833 (Meta Duration Extension) + 900834 (Meta AoE+Heal) zusammen machen Demo-Lock zu einem permanent transformierten AoE-Healer-Tank-Hybrid.

---

### Priest — Discipline (900900-900932)

> Priest SpellFamilyName = 6. Disc fokussiert auf Shield-Enhancement.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900900 | Shields explode on breaking/fading | C++ | geplant | Power Word: Shield (48066) und andere Absorb-Shields. AuraScript `OnRemove`: Wenn RemoveMode = EXPIRE (fade) oder ENEMY_SPELL (break durch Damage) → CastSpell(Shield-Explosion-Helper, triggered=true) zentriert auf Shield-Target. Explosion = Holy/Shadow AoE Damage im Radius, Damage skaliert mit verbleibendem/absorbiertem Shield-Amount. Braucht Helper-AoE-Spell (z.B. 900903). Sehr cool thematisch — Disc wird zum AoE-DPS via Shields. |
| 2 | 900901 | Shields +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf PW:Shield (48066) Absorb-Amount. SpellFamilyName=6, SpellFamilyFlags für PW:S. Erhöht Absorb-Wert um 50%. Stackt mit existierenden Talents (Improved PW:S, Twin Disciplines, etc.). |
| 3 | 900902 | Weakened Soul only 5 sec CD | DBC | geplant | Weakened Soul (6788) Debuff hat normalerweise 15s Duration. DBC: Duration auf 5000ms setzen. Ermöglicht viel häufigeres Re-Shielding. Einfache DBC-Duration-Änderung. Synergiert stark mit 900900 (Shield Explosion) — mehr Shields = mehr Explosions. |

### Priest — Holy (900933-900965)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900933 | Direct heals have chance to cast Holy Fire on enemies in 10y radius of target | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS` (Positive Spell = Heal). `HandleProc`: Wenn Direct Heal (Flash Heal 48071, Greater Heal 48063, etc. — kein HoT) → Chance X% → Finde alle Feinde im 10yd Radius um Heal-Target → CastSpell(Holy Fire 48135, triggered=true) auf jeden. Dual-Purpose-Heal: Heilen + gleichzeitig AoE DPS. Braucht SpellFamily-Filter um nur Direct Heals zu triggern (nicht Renew/PoM). ICD empfohlen. |

### Priest — Shadow (900966-900999)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900966 | DoTs have a chance to deal shadow damage AoE | C++ | geplant | Gleicher Ansatz wie Warlock 900800. Passive Proc-Aura: `PROC_FLAG_DONE_PERIODIC`. `HandleProc`: Wenn Shadow-DoT-Tick (SW:Pain 48125, VT 48160, Devouring Plague 48300) → Chance X% → CastSpell(Shadow-AoE-Helper, triggered=true) am Target. Braucht Helper-AoE-Spell (z.B. 900968). ICD empfohlen. |
| 2 | 900967 | DoTs have a chance to spread to 2 additional targets on tick | C++ | geplant | Gleicher Ansatz wie Warlock 900802. Passive Proc-Aura: `PROC_FLAG_DONE_PERIODIC`. `HandleProc`: DoT-Tick → Chance X% → Finde 2 nächste Feinde ohne den DoT → ApplyAura(gleicher DoT). Muss DoT-Spell-ID aus ProcEventInfo extrahieren. Gleiche Warnung: kann exponentiell wachsen, braucht Target-Cap. |

> **Helper-Spells Priest**: 900900 (Shield Explosion) → Holy/Shadow-AoE-Helper 900506. 900966 (DoT AoE) → Shadow-AoE-Helper 900507. IDs ab 900903+ für Helper verfügbar.

> **Besonders aufwändig**: 900900 (Shield Explosion) braucht korrekte `OnRemove`-Detection (fade vs. dispel vs. break) und Damage-Skalierung basierend auf Shield-Amount. 900933 (Heal → Holy Fire AoE) ist ein neuartiges Dual-Purpose-Konzept — muss sauber zwischen Direct Heals und HoTs unterscheiden. Shadow-DoT-Mechaniken (900966/900967) sind identisch zu Warlock — Code kann geshared werden.

---

### Non-Class — Global (901100-901199)

> Globale Passive-Spells die für ALLE Klassen gelten. SpellFamilyName = 0 (Generic). Werden allen Spielern auf Paragon-Level automatisch granted.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 901100 | Cast while moving | DBC/C++ | geplant | Passive Aura: Muss alle Spells des Spielers betreffen. Ansatz: `SPELL_ATTR5_CAN_CHANNEL_WHEN_MOVING` reicht nur für Channels. Für alle Casts: C++ Hook auf `Spell::CheckCast()` → Skip `SPELL_FAILED_MOVING` Check wenn Aura aktiv. Oder: `Player::isMoving()` Override. Alternativ DBC: Aura mit `SPELL_AURA_CAST_WHILE_WALKING` (Aura 330, existiert in WotLK-DBC). Mächtigster globaler Buff — eliminiert Cast-Time-Weakness komplett für alle Caster-Klassen. |
| 2 | 901101 | Kill enemy → heal 5% total HP | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_KILL` (0x1). `HandleProc`: `GetCaster()->CastCustomSpell(Heal-Helper, SPELLVALUE_BASE_POINT0, MaxHealth * 5 / 100, GetCaster(), triggered=true)`. Oder: `GetCaster()->ModifyHealth(MaxHealth * 0.05)`. Einfacher On-Kill-Heal. Kein ICD nötig (Kill-Events sind natürlich rate-limited). Braucht ggf. Helper-Heal-Spell (z.B. 901105) für Combat-Log-Visibility. |
| 3 | 901102 | Attacks 25% chance to hit again (Extra Attack) | C++/DBC | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4) + `PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS` (0x10). Chance 25%. `HandleProc`: CastSpell(Extra-Attack-Helper, triggered=true) auf Target — wiederholt den letzten Angriff. Für Melee: `SPELL_AURA_ADD_EXTRA_ATTACKS` (wie Windfury/Sword Spec). Für Ranged/Spell: SpellScript `AfterHit` → CastSpell(gleicher Spell, triggered=true) mit 25% Chance. Achtung: Muss rekursive Procs verhindern (Extra-Attack triggert nicht nochmal). |
| 4 | 901103 | Spells/abilities 10% chance to hit all enemies within 10y | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG` + `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` + `PROC_FLAG_DONE_RANGED_AUTO_ATTACK`. Chance 10%. `HandleProc`: Finde alle Feinde im 10yd Radius um Target → CastSpell(Damage-Helper, triggered=true) auf jedes. Damage = gleicher Amount wie Original-Hit. Braucht ProcEventInfo → GetDamageInfo → GetDamage() für Damage-Wert. Braucht Helper-Damage-Spell (z.B. 901106). ICD empfohlen (z.B. 1s). |
| 5 | 901104 | Avoid attack → counter attack | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK` (0x2) mit `PROC_HIT_DODGE|PROC_HIT_PARRY|PROC_HIT_BLOCK` (Avoid-Events). `HandleProc`: Wenn Dodge/Parry/Block → CastSpell(Counter-Attack-Helper, triggered=true) auf Attacker. Counter Attack = Instant Melee-Damage zurück. Vergleichbar mit Rogue Riposte oder Warrior Overpower Proc — aber automatisch und für alle Klassen. Braucht Helper-Damage-Spell (z.B. 901107). |

> **Helper-Spells Non-Class**: 901101 (Kill Heal) → Heal-Helper 900555. 901103 (AoE Proc) → Damage-Helper 900556. 901104 (Counter Attack) → Damage-Helper 900557. IDs ab 901105+ für Helper verfügbar.

> **Besonders aufwändig**: 901100 (Cast While Moving) ist der mächtigste Buff im gesamten System — verändert das Gameplay fundamental für alle Caster. Muss robust implementiert sein (Channel + Cast + Interruptible-Spells). 901102 (Extra Attack 25%) muss rekursive Procs sauber verhindern. 901103 (10% AoE Proc) braucht ICD um Spam bei schnellen DoT-Ticks zu verhindern.

---

### Entscheidungshilfe: DBC vs. C++

#### Rein DBC (kein C++ nötig)

| Effekt-Typ | DBC Aura | DBC MiscValue | Beispiel |
|------------|----------|---------------|----------|
| Damage +X% | `ADD_PCT_MODIFIER` (108) | `SPELLMOD_DAMAGE` (0) | MS +50%, BT +50%, TC +50%, Revenge +50% |
| Cooldown -Xs | `ADD_FLAT_MODIFIER` (107) | `SPELLMOD_COOLDOWN` (11) | MS cd -2s (BasePoints=-2000) |
| Cast Time -X% | `ADD_PCT_MODIFIER` (108) | `SPELLMOD_CASTING_TIME` (14) | AB cast -50% |
| Unlimited targets | Direkte DBC-Änderung | `MaxAffectedTargets=0` | WW, Cleave |
| Remove stance req | Direkte DBC-Änderung | `Stances=0` | WW stance removal |

#### C++ erforderlich

| Mechanik | Grund für C++ | Beispiele |
|----------|---------------|-----------|
| Conditional Procs | Komplexe Bedingungen (Crit, Target-Count, CD) | 20% Crit → Execute, WW 1-target autocast |
| Multi-Spell Triggers | Ein Proc löst mehrere Spells aus | TC → Rend + 5× Sunder |
| Single→AoE Umbau | Custom target selection | Revenge unlimited, Overpower +9 targets |
| Block/Dodge/Parry Procs | HitMask-Filtering | AoE on block, 10% block → TC |
| Custom Damage Formeln | Nicht-Standard Berechnung | Paragon Strike (666 + 0.66×AP + PL%) |
| CD Manipulation | Runtime CD-Änderungen | Melee → Bladestorm CD -0.5s |

#### Häufige Fehlerquellen

1. **SpellFamilyFlags falsch**: IMMER aus eigener Spell.dbc verifizieren, nie aus Online-DBs
2. **MaxAffectedTargets=0 global**: Betrifft ALLE Spieler, nicht nur mit Passive → für bedingte Targets C++ nutzen
3. **Proc-Loop**: Helper-Spells können erneut procen → ICD in spell_proc setzen
4. **spell_script_names fehlt**: C++ Klasse wird nicht geladen → Spell hat keinen Effekt
5. **DurationIndex vergessen**: Passive Aura braucht DurationIndex=21 (permanent)
6. **Attributes fehlt PASSIVE**: Ohne 0x40 ist der Spell castbar statt permanent aktiv

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
