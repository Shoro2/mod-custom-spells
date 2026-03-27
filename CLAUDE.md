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
│   ├── custom_spells_common.h        # Shared header: enum, constants, includes
│   ├── custom_spells.cpp             # Main: calls per-class registration functions
│   ├── custom_spells_warrior.cpp     # Warrior (Arms/Fury/Prot)
│   ├── custom_spells_paladin.cpp     # Paladin (Holy/Prot/Ret)
│   ├── custom_spells_dk.cpp          # Death Knight (Blood/Frost/Unholy)
│   ├── custom_spells_shaman.cpp      # Shaman (Ele/Enhance/Resto)
│   ├── custom_spells_hunter.cpp      # Hunter (BM/MM/Surv)
│   ├── custom_spells_rogue.cpp       # Rogue (Assa/Combat/Sub)
│   └── custom_spells_druid.cpp       # Druid (Balance/Feral/Resto)
├── conf/
│   └── mod_custom_spells.conf.dist   # Config: CustomSpells.Enable
└── data/sql/db-world/
    └── mod_custom_spells.sql          # spell_script_names, spell_dbc, spell_proc
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

## DBC Status

Spell IDs 900100-900109 (Warrior Arms) existieren in `Spell.dbc` und sind fertig implementiert.
Spell IDs 900134-900145 (Warrior Fury) existieren in `Spell.dbc` und sind fertig implementiert.
Spell IDs 900168-900175 (Warrior Prot + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900200-900210 (Paladin Holy + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900234-900241 (Paladin Prot) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900268-900275 (Paladin Ret + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900300-900304 (DK Blood) existieren in `spell_dbc` Tabelle und sind implementiert.
Spell IDs 900333, 900368 (DK Frost + Frost Breath helper) existieren in `spell_dbc` Tabelle und sind implementiert.
NPC 900333 (Frost Wyrm) existiert in `creature_template` mit AI-Script `npc_custom_frost_wyrm`.
Spell IDs 900366-900367 (DK Unholy + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.

Spell IDs 900400-900408 (Shaman Ele + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.

Spell IDs 900433-900440 (Shaman Enhance + Helper) existieren in `spell_dbc` Tabelle und sind implementiert.
NPC 900436 (Spirit Wolf) existiert in `creature_template` für Wolf-Summon-Proc.
Spell IDs 900466-900467 (Shaman Resto) existieren in `spell_dbc` Tabelle und sind implementiert.

Spell IDs 900500-900567 (Hunter Shared + BM + MM + Surv + Helpers) existieren in `spell_dbc` Tabelle und sind implementiert.

Spell IDs 901000-901073 (Druid Balance + Feral + Resto + Helpers) existieren in `spell_dbc` Tabelle und sind implementiert.
NPC 901066 (Healing Treant) existiert in `creature_template` für HoT-Treant-Proc.

Spell IDs 900600-900669 (Rogue Assa + Combat + Sub + Helpers) existieren in `spell_dbc` Tabelle und sind implementiert.

Nächste freie Klassen-Blöcke: **900700+** (Mage), **900800+** (Warlock), etc.

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
