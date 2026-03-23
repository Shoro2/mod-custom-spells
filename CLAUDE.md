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

Spell IDs 900100-900121 existieren alle in `Spell.dbc` (22 Einträge) und sind fertig implementiert (DBC + C++ wo nötig).
Einzige Ausnahme: **Thunderclap → Rend + 5× Sunder Armor** (Arms/Prot) fehlt noch.

Nächste freie ID für neue Spells: **900122+**

---

## Custom Spell Master Plan (All Classes)

> **Status-Legende**: `geplant` → `implementiert` → `getestet`

### ID-Block-Schema

| Klasse | Spec | ID-Block | Anzahl | Status |
|--------|------|----------|--------|--------|
| Warrior | Arms | 900100-900109 | 10 | getestet (bis auf TC→Rend+Sunder: geplant) |
| Warrior | Fury | 900110-900121 | 12 | getestet |
| Warrior | Prot | 900122-900127 | 6 | geplant |
| Paladin | Holy | 900150-900157 | 8 | geplant |
| Paladin | Prot | 900158-900165 | 8 | geplant |
| Paladin | Ret | 900166-900172 | 7 | geplant |
| DK | Blood | 900200-900204 | 5 | geplant |
| DK | Frost | 900205 | 1 | geplant |
| DK | Unholy | 900206 | 1 | geplant |
| (weitere Klassen) | — | ab 900250+ | — | Wartet auf weitere Teile |

---

### Warrior — Arms (900100-900109)

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

### Warrior — Fury (900110-900121)

| # | Spell ID | Effekt | Ansatz | Status |
|---|----------|--------|--------|--------|
| 1 | 900110 | Bloodthirst +50% damage | DBC | getestet |
| 2 | 900111 | Bloodthirst +9 targets (AoE) | DBC+C++ | getestet |
| 3 | 900112 | Whirlwind damage +50% | DBC | getestet |
| 4 | 900113 | Whirlwind unlimited targets | DBC | getestet |
| 5 | 900114 | Cleave unlimited targets | DBC | getestet |
| 6 | 900115 | 20% Auto-Attack → free Whirlwind | C++ | getestet |
| 7 | 900116 | BT → next WW +50% damage (Bloody WW passive) | C++ | getestet |
| 8 | 900117 | WW resets Bloodthirst CD | C++ | getestet |
| 9 | 900118+900120 | WW 1 target → autocast Overpower | C++ | getestet |
| — | 900119+900121 | WW 1 target → autocast Slam | C++ | getestet |
| 10 | (DBC) | Remove WW stance requirement | DBC | getestet |

### Warrior — Prot (ab 900122)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900122 | Revenge +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für Revenge. |
| 2 | 900123 | Revenge unlimited targets | C++ | geplant | Revenge ist Single-Target. AoE-Umbau: DBC TargetA→`TARGET_UNIT_SRC_AREA_ENEMY` + SpellScript für Target-Filterung. |
| 3 | 900124 | Thunderclap → Rend + 5× Sunder Armor | C++ | geplant | Passive Marker-Aura, TC-Hook prüft via `HasAura`. `AfterHit` pro Target: CastSpell(Rend) + 5× CastSpell(SunderArmor). Wird auch von Arms geteilt. |
| 4 | 900125 | Thunderclap +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für TC. |
| 5 | 900126 | AoE damage on Block | C++ | geplant | Proc-Aura mit `PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK` + HitMask=BLOCK. HandleProc: CastSpell(AoE-Damage-Spell). Braucht zusätzlichen AoE-Damage-Spell (eigene ID). |
| 6 | 900127 | 10% Block-Chance → Enhanced Thunderclap | C++ | geplant | Passive Proc-Aura: Block-Proc, 10% Chance. Triggered: Enhanced TC Spell (eigene ID, höherer Damage). |

> **Hinweis Prot**: Spells 900126 und 900127 brauchen jeweils zusätzliche Helper-Spells (AoE-Damage-Spell bzw. Enhanced TC). Diese können IDs ab 900128+ verwenden.

---

### Paladin — Holy (900150-900157)

> Paladin SpellFamilyName = 10. Shared Spells (Consecration around you, Judgement cd) können spec-übergreifend dieselbe ID verwenden, wenn alle 3 Specs den Effekt haben.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900150 | Holy Shock deals AoE damage | C++ | geplant | SpellScript auf Holy Shock (20473). `OnEffectHitTarget`: AoE-Damage auf alle Feinde im Radius um das Ziel casten (triggered Spell). Oder DBC-Clone mit `TARGET_UNIT_DEST_AREA_ENEMY`. |
| 2 | 900151 | Holy Shock heals AoE | C++ | geplant | Analog zu #1: AoE-Heal auf alle Verbündeten im Radius. Oder DBC-Clone mit `TARGET_UNIT_DEST_AREA_ALLY`. |
| 3 | 900152 | Holy Shock always does both (damage + heal) | C++ | geplant | SpellScript: `AfterHit` prüft ob Damage- oder Heal-Version gecastet wurde und castet automatisch die andere Version auf passendes Ziel (nächster Feind/Verbündeter). |
| 4 | 900153 | Holy Shock damage/healing +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE` (für Damage) + zweiter Effekt für Healing, SpellFamilyMask für Holy Shock. |
| 5 | 900154 | Consecration also heals | C++ | geplant | AuraScript auf Consecration-Aura (48819). `OnEffectPeriodic`: Zusätzlich Heal auf Verbündete im Radius casten. Oder zweiten periodischen Heal-Effekt via DBC hinzufügen. |
| 6 | 900155 | Consecration is around you (centered on caster) | DBC | geplant | DBC-Änderung: TargetA von `TARGET_DEST_TARGET` auf `TARGET_DEST_CASTER` ändern. Spell folgt dann dem Caster. **Shared mit Prot (900158) und Ret (900166)** — ggf. eine gemeinsame ID. |
| 7 | 900156 | Consecration damage/heal +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für Consecration. |
| 8 | 900157 | Consecration duration +5sec | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_FLAT_MODIFIER` + `SPELLMOD_DURATION` (17), BasePoints=5000 (ms). SpellFamilyMask für Consecration. |

### Paladin — Prot (900158-900165)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900158 | Consecration is around you | DBC | geplant | **Shared mit Holy (900155) und Ret (900166)** — identischer Effekt. Ggf. eine gemeinsame Spell-ID verwenden. |
| 2 | 900159 | Avenger's Shield +9 targets | DBC | geplant | DBC: `MaxAffectedTargets` auf 12 setzen (3 base + 9). Oder passive Aura falls bedingt. Avenger's Shield (48827) bounced normalerweise 3×. |
| 3 | 900160 | Avenger's Shield +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für AS. |
| 4 | 900161 | Holy Shield charges +99 | DBC/C++ | geplant | DBC: `SPELL_AURA_ADD_FLAT_MODIFIER` + `SPELLMOD_CHARGES` (16), BasePoints=99. Alternativ direkt die Charges im DBC-Eintrag von Holy Shield (48952) erhöhen. Muss getestet werden ob SPELLMOD_CHARGES funktioniert. |
| 5 | 900162 | Holy Shield +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für Holy Shield. |
| 6 | 900163 | Avenger's Shield leaves Consecration on hit | C++ | geplant | SpellScript auf AS: `OnEffectHitTarget` pro getroffenes Ziel → CastSpell(Consecration, triggered=true) an der Zielposition. Braucht evtl. eigene Consecration-Variante (kürzere Dauer?). |
| 7 | 900164 | Judgement → free Avenger's Shield | C++ | geplant | Passive Proc-Aura: Proc auf Judgement-Cast (SpellFamilyName=10, Judgement-FamilyFlags). `HandleProc`: CastSpell(Avenger's Shield, triggered=true) auf das Judgement-Ziel. |
| 8 | 900165 | Judgement cd -2sec | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_FLAT_MODIFIER` + `SPELLMOD_COOLDOWN`, BasePoints=-2000. SpellFamilyMask für Judgement. **Shared mit Ret (900167)**. |

### Paladin — Ret (900166-900172)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900166 | Consecration is around you | DBC | geplant | **Shared mit Holy (900155) und Prot (900158)** — identischer Effekt. |
| 2 | 900167 | Judgement cd -2sec | DBC | geplant | **Shared mit Prot (900165)** — identischer Effekt. |
| 3 | 900168 | Divine Storm +6 targets | DBC | geplant | DBC: `MaxAffectedTargets` auf 10 setzen (4 base + 6). DS (53385) trifft normalerweise 4 Ziele. |
| 4 | 900169 | Divine Storm +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für DS. |
| 5 | 900170 | Crusader Strike +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für CS (35395). |
| 6 | 900171 | Crusader Strike +9 targets | C++ | geplant | CS ist Single-Target. AoE-Umbau: DBC TargetA→`TARGET_UNIT_SRC_AREA_ENEMY` + SpellScript für Target-Cap. |
| 7 | 900172 | CS/Judgement/DS → Exorcism +50% buff (max 10 stacks) | C++ | geplant | Passive Proc-Aura (wie Bloody Whirlwind System): Proc auf CS/Judgement/DS-Cast → stacking Buff auf Caster (+50% Exorcism damage pro Stack, max 10). Exorcism-SpellScript konsumiert Stacks. Braucht: Passive (900172) + Buff-Spell (eigene ID) + ggf. Exorcism-Hook (eigene ID). |

> **Shared Spells Paladin**: "Consecration around you" (Holy/Prot/Ret) und "Judgement cd -2sec" (Prot/Ret) tauchen mehrfach auf. Optionen: (a) Eine gemeinsame ID pro Effekt, die allen Specs gegeben wird. (b) Separate IDs pro Spec für unabhängige Zuordnung. Empfehlung: Separate IDs, da Specs über ein Talent/Unlock-System zugewiesen werden und nicht alle Specs gleichzeitig aktiv sind.

> **Helper-Spells Paladin**: 900172 (Exorcism-Buff-System) braucht 2-3 IDs: Passive Proc-Aura + Stacking Buff + evtl. Exorcism-Consume-Hook. 900163 (AS→Consecration) braucht evtl. eine eigene Consecration-Variante. Diese Helper-Spells können IDs ab 900173+ verwenden.

---

### DK — Blood (900200-900204)

> DK SpellFamilyName = 15. Dancing Rune Weapon (49028) beschwört ein Rune Weapon das Spells des DK kopiert.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900200 | 3 Rune Weapons gleichzeitig | C++ | geplant | Dancing Rune Weapon (49028) beschwört normalerweise 1 Weapon. Braucht SpellScript `AfterCast`: 2 zusätzliche CastSpell(DRW, triggered) oder direktes Spawnen von 2 Extra-NPCs via `SummonCreature`. Die Summon-NPC-ID (27893) muss 3× existieren können. Evtl. eigene Summon-Spell-Varianten nötig. |
| 2 | 900201 | Rune Weapon double-casts | C++ | geplant | Das Rune Weapon kopiert bereits Caster-Spells. "Double cast" = jeder kopierte Spell wird 2× ausgeführt. Braucht Hook im DRW-AI-Script (oder `CreatureScript`): Bei jedem `SpellHit` des Weapons → gleichen Spell nochmal casten. Alternativ: Aura auf Weapon mit Proc → re-cast. |
| 3 | 900202 | Heart Strike damage +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_DAMAGE`, SpellFamilyMask für Heart Strike (55050). |
| 4 | 900203 | Heart Strike +9 targets | DBC/C++ | geplant | Heart Strike trifft normalerweise 3 Ziele. DBC: `MaxAffectedTargets` auf 12 setzen. Falls bedingt → C++ `OnObjectAreaTargetSelect`. |
| 5 | 900204 | Dealing damage → chance to cast Death Coil | C++ | geplant | Passive Proc-Aura: ProcFlags=`DONE_MELEE_AUTO_ATTACK|DONE_SPELL_MELEE_DMG_CLASS` (0x14), Chance=X% (festlegen!). `HandleProc`: CastSpell(Death Coil, triggered=true) auf aktuelles Ziel. Braucht `spell_proc` Eintrag + ICD um Spam zu verhindern. |

### DK — Frost (900205)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900205 | Ghoul durch Frost Wyrm ersetzen | C++ + DBC | geplant | Raise Dead (46584) beschwört Ghoul (NPC 26125). Ansatz: (a) Neuen NPC "Frost Wyrm" erstellen (`creature_template`) mit eigenem Model, Stats, AI. (b) SpellScript auf Raise Dead: Override Summon-NPC-ID auf Frost Wyrm NPC. Oder DBC: `EffectMiscValue` (Summon-Entry) auf neue NPC-ID ändern. (c) Frost Wyrm braucht eigenes AI-Script (`CreatureScript`) mit passenden Frost-Angriffen. **Aufwand**: Hoch — neues Creature mit Model, AI, Spells, Balancing. |

### DK — Unholy (900206)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900206 | DoTs → chance Shadow AoE damage | C++ | geplant | Passive Proc-Aura: Proc auf periodischen Schaden (`PROC_FLAG_DONE_PERIODIC`, 0x400000). `HandleProc`: CastSpell(Shadow-AoE-Damage, triggered=true) auf das DoT-Ziel. Der AoE-Spell dealt Shadow-Damage an alle Feinde im Radius um das Ziel. Braucht: (a) Passive Proc-Aura (900206), (b) Shadow-AoE-Damage-Spell (eigene ID, z.B. 900207), (c) `spell_proc` mit Chance + ICD. |

> **Helper-Spells DK**: 900206 (Unholy DoT AoE) braucht einen separaten Shadow-AoE-Damage-Spell (z.B. 900207). 900200 (3 Rune Weapons) braucht evtl. zusätzliche Summon-Spell-Varianten. 900205 (Frost Wyrm) braucht neuen NPC-Eintrag + AI-Script. IDs ab 900207+ für Helper verfügbar.

> **Besonders aufwändig**: 900200 (3 Rune Weapons) und 900205 (Frost Wyrm) sind die komplexesten Spells bisher. Rune Weapons brauchen Core-Verständnis der DRW-Mechanik, Frost Wyrm braucht komplett neues Creature mit Model + AI.

---

### Implementierungs-Referenz

#### Rein DBC (kein C++ nötig) — via `spell_dbc` Tabelle oder WoW Spell Editor

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
3. **Helper-Spells**: Viele Effekte brauchen 2+ Spell-IDs (Passive + Trigger + evtl. Buff). Genügend ID-Raum pro Spec einplanen
4. **spell_proc Tabelle**: Jeder proc-basierte Spell braucht einen Eintrag in `spell_proc` mit korrekten ProcFlags
5. **Testen**: Neue DBC-Modifier immer in-game testen — manche Aura-Typen funktionieren nicht für alle Spell-Kombinationen

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
