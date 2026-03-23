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
| Shaman | Ele | 900250-900257 | 8 | geplant |
| Shaman | Enhance | 900258-900263 | 6 | geplant |
| Shaman | Resto | 900264-900265 | 2 | geplant |
| Hunter | Shared | 900300-900301 | 2 | geplant |
| Hunter | BM | 900302-900304 | 3 | geplant |
| Hunter | MM | 900305-900306 | 2 | geplant |
| Hunter | Surv | 900307 | 1 | geplant |
| Rogue | Shared | 900350 | 1 | geplant |
| Rogue | Assa | 900351-900353 | 3 | geplant |
| Rogue | Combat | 900354-900357 | 4 | geplant |
| Rogue | Sub | 900358-900359 | 2 | geplant |
| Mage | Shared | 900400 | 1 | geplant |
| Mage | Arcane | 900401-900409 | 9 | geplant |
| Mage | Fire | 900410-900415 | 6 | geplant |
| Mage | Frost | 900416-900421 | 6 | geplant |
| Warlock | Affli | 900450-900452 | 3 | geplant |
| Warlock | Demo | 900453-900460 | 8 | geplant |
| Warlock | Destro | 900461-900465 | 5 | geplant |
| (weitere Klassen) | — | ab 900500+ | — | Wartet auf weitere Teile |

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

### Shaman — Elemental (900250-900257)

> Shaman SpellFamilyName = 11. Totem-Replacement ist ein Kern-Feature für alle 3 Specs — jede Spec braucht eigene Variante da unterschiedliche Totems/Creatures relevant sind.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900250 | Chain Lightning +6 targets, no damage reduction | DBC/C++ | geplant | Chain Lightning (421/CL ranks) hat normalerweise 3 Bounces mit 30% Reduce pro Bounce. DBC: `EffectChainTarget` auf 9 erhöhen. Damage-Reduction entfernen: C++ SpellScript `CalculateChainDamage` oder Aura `SPELL_AURA_ADD_PCT_MODIFIER` die Chain-Penalty auf 0 setzt. |
| 2 | 900251 | Totems replaced by creatures that follow/fight (Ele) | C++ | geplant | Großes System: Totem-Summon-Spells abfangen (`SpellScript`), statt Totem-NPC ein Creature spawnen das dem Caster folgt (`FollowMovementGenerator`) und kämpft. Braucht eigene AI-Scripts für jedes Totem-Creature. Creature behält Totem-Effekte (Auren) aber ist mobil + angreifbar. Evtl. eigene NPCs pro Totem-Typ. |
| 3 | 900252 | Fire Elemental Totem → Ragnaros | C++ + DBC | geplant | Fire Elemental Totem (2894) beschwört Fire Elemental (15438). Override: Neuen NPC "Ragnaros" erstellen mit eigenem Model (DisplayID vom Ragnaros-Boss, z.B. 11121), eigener AI mit Fire-Spells + evtl. On-Hit-Proc. SpellScript auf Totem-Spell → Summon Ragnaros-NPC statt Fire Elemental. Balancing: Stats/Duration anpassen. |
| 4 | 900253 | Lightning Overload chance erhöht + wirkt auf Lava Burst | C++ | geplant | Lightning Overload (Talent 30675) proct auf Lightning Bolt/Chain Lightning. (a) Chance erhöhen: Modify Proc-Chance des Talents via Aura. (b) Lava Burst hinzufügen: SpellScript oder `spell_proc` erweitern → Overload-Proc auch bei Lava Burst (51505) auslösen. Braucht evtl. eigenen Proc-Handler. |
| 5 | 900254 | Lava Burst spreads Fire Shock DoT | C++ | geplant | SpellScript auf Lava Burst `AfterHit`: Prüfe ob Target Fire Shock (8050) hat → `GetCaster()->CastSpell()` Fire Shock auf alle Feinde im Radius (z.B. 10yd) um Target. Oder: Iterate nahe Enemies und appliziere Fire Shock Aura. Ähnlich wie Warrior Rend-Spread. |
| 6 | 900255 | Flame Shock ticks chance to reset Lava Burst CD | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_PERIODIC` (0x400000), SpellFamilyMask für Flame Shock. `HandleProc`: Chance X% → `GetCaster()->GetSpellHistory()->ResetCooldown(51505, true)` (Lava Burst). `spell_proc` mit Chance + ICD. |
| 7 | 900256 | Lava Burst two charges | DBC/C++ | geplant | Charge-System: DBC `ChargeCategory` + `CategoryCharges` = 2 für Lava Burst. Oder C++: Eigene Aura die bei Lava Burst-CD eine zweite "Aufladung" trackt und CD sofort resetet wenn noch Charges übrig. Charge-System in WotLK ist limitiert → vermutlich C++ nötig. |
| 8 | 900257 | Clearcasting → Lava Burst instant | DBC | geplant | Clearcasting (Elemental Focus, 16246) gibt Buff. Passive Aura: Wenn Clearcasting aktiv → `SPELL_AURA_ADD_PCT_MODIFIER` + `SPELLMOD_CASTING_TIME` = -100% auf Lava Burst (SpellFamilyMask). Lava Burst hat bereits 2s Cast. |

### Shaman — Enhancement (900258-900263)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900258 | Totems replaced by summons that follow/fight (Enhance) | C++ | geplant | Gleiche Kern-Mechanik wie 900251 (Ele Totem-Replacement), aber Enhance-spezifische Creatures mit Melee-AI statt Caster-AI. Shared Code-Basis mit 900251 möglich. |
| 2 | 900259 | 5 Maelstrom stacks → summons empowered, AoE on attack 5s | C++ | geplant | Hook auf Maelstrom Weapon (53817) Stack-Erreichen (5 Stacks). Bei 5 Stacks: Buff auf alle aktiven Summons → `SPELL_AURA_PROC_TRIGGER_SPELL` oder AI-Flag das AoE-Damage bei jedem Angriff für 5s auslöst. Braucht Tracking welche Summons aktiv sind. |
| 3 | 900260 | Summons +50% damage | DBC/C++ | geplant | Passive Aura auf Caster: Alle Summons (Pets/Guardians) erhalten `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` +50%. Oder: Aura auf Summon-Creatures via Owner-Check. |
| 4 | 900261 | Auto attacks chance to summon a wolf | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4). `HandleProc`: Chance X% → `CastSpell(SummonWolf)`. Wolf-NPC: temporärer Guardian mit Timer (z.B. 15s), Melee-AI, Wolf-Model. Braucht eigenen NPC + Summon-Spell. |
| 5 | 900262 | Spirit Wolves inherit your haste | C++ | geplant | Feral Spirit (51533) beschwört 2 Spirit Wolves (29264). CreatureScript/PetAI: Bei Summon → Owner Haste-Rating auslesen → Attackspeed des Wolves anpassen via `ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME)`. Evtl. periodisch updaten falls Caster-Haste sich ändert. |
| 6 | 900263 | Spirit Wolves 5% chance Chain Lightning on melee hit | C++ | geplant | CreatureScript für Spirit Wolf (29264): `OnMeleeHit` oder Proc-Aura auf Wolf → 5% Chance → `CastSpell(Chain Lightning, triggered=true)`. Chain Lightning Rank basierend auf Caster-Level. |

### Shaman — Restoration (900264-900265)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900264 | Totems replaced by creatures that follow/fight (Resto) | C++ | geplant | Gleiche Kern-Mechanik wie 900251/900258, aber Resto-spezifische Creatures mit Healer/Support-AI. Healing-Totems (Healing Stream etc.) werden zu folgenden Heal-Creatures. Shared Code-Basis. |
| 2 | 900265 | Mana regen +2% per missing mana % | C++ | geplant | Passive Aura mit `SPELL_AURA_OBS_MOD_POWER` oder custom `HandlePeriodicManaRegen`. Berechnung: `missingManaPct = 1 - (currentMana / maxMana)` → `bonusRegen = missingManaPct * 2% * baseRegen`. Bei 50% Mana → +100% Regen, bei 10% Mana → +180% Regen. Skaliert umgekehrt proportional. |

> **Helper-Spells Shaman**: 900251/900258/900264 (Totem-Replacement) ist ein gemeinsames System — braucht evtl. mehrere Helper-NPCs pro Totem-Typ (Fire, Earth, Water, Air Totem-Creatures). 900261 (Wolf-Summon) braucht eigenen Wolf-NPC + Summon-Spell. 900259 (Maelstrom Empower) braucht AoE-Buff-Spell für Summons. IDs ab 900266+ für Helper verfügbar.

> **Besonders aufwändig**: Das Totem-Replacement-System (900251/900258/900264) ist das ambitionierteste Feature bisher — jedes Totem braucht ein eigenes folgendes Creature mit passender AI. Ragnaros-Replacement (900252) braucht Custom-Boss-NPC mit Fire-AI. Spirit Wolf Haste-Inheritance (900262) braucht dynamische Stat-Synchronisation.

---

### Hunter — Shared (900300-900301)

> Hunter SpellFamilyName = 9. "Get back arrows" und "Multishot unlimited targets" gelten für alle 3 Specs → shared Spells.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900300 | Get back arrows (no ammo consumption) | C++ | geplant | Hunter verbraucht Ammo bei jedem Ranged-Angriff. Ansatz: `PlayerScript::OnBeforeItemUse` oder `SpellScript` auf Auto Shot (75) → nach Schuss Ammo-Count wiederherstellen. Oder: Passive Aura die Ammo-Verbrauch auf 0 setzt. Core-seitig: `Player::RemoveAmmo()` Hook → Skip wenn Aura aktiv. Alternativ einfacher: periodisch Ammo-Stack auf Max setzen. |
| 2 | 900301 | Multi-Shot unlimited targets | DBC/C++ | geplant | Multi-Shot (2643) trifft normalerweise 3 Ziele. DBC: `MaxAffectedTargets` entfernen/sehr hoch setzen. C++: `OnObjectAreaTargetSelect` → keine Target-Limit-Filterung. Spell trifft dann alle Feinde im Cone/Radius. |

### Hunter — Beast Mastery (900302-900304)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900302 | Pet damage +50% | DBC | geplant | Passive Aura auf Hunter: `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` +50% für Pet. Oder: Aura mit `SPELL_AURA_MOD_DAMAGE_DONE_FOR_MECHANIC` auf Pet anwenden. SpellFamilyName=9, Target=Pet. Ähnlich wie bestehende Pet-Talent-Buffs (z.B. Unleashed Fury). |
| 2 | 900303 | Pet attack speed +50% | DBC | geplant | Passive Aura: `SPELL_AURA_MOD_ATTACKSPEED` -50% (= 50% schneller) auf Pet. Target: Owner's Pet. Vergleichbar mit Serpent's Swiftness (Talent) aber stärker. |
| 3 | 900304 | Pet chance to deal AoE damage | C++ | geplant | Proc-Aura auf Pet: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4). `HandleProc`: Chance X% → CastSpell(AoE-Damage-Spell, triggered=true) zentriert auf Pet's Target. Braucht Helper-AoE-Spell (z.B. 900308). `spell_proc` mit Chance + ICD. |

### Hunter — Marksmanship (900305-900306)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900305 | Auto Shot bounces +9 targets | C++ | geplant | Auto Shot (75) ist normalerweise Single-Target. SpellScript `AfterHit`: Bei Auto Shot → Chain-Bounce-Logik: Finde bis zu 9 weitere Feinde im Radius um Target → CastSpell(Auto-Shot-Damage, triggered=true) auf jedes. Damage kann pro Bounce gleich bleiben (kein Reduce wie Chain Lightning). Braucht Helper-Damage-Spell (z.B. 900309) für den Bounce-Damage. |
| 2 | 900306 | Autocast Multi-Shot every 0.1s for 2s, 50% slow while channeling | C++ | geplant | Aktivierter Spell: Beim Cast → Channel-Aura (2s Duration). Periodic Tick alle 100ms → CastSpell(Multi-Shot, triggered=true). Gleichzeitig: `SPELL_AURA_MOD_SPEED_SLOW_ALL` -50% auf Caster während Channel. 20 Multi-Shots in 2s = extrem hoher Burst. Braucht eigene Channel-Spell-DBC + PeriodicTrigger-Logik. Balancing-Warnung: Enormer Damage-Output. |

### Hunter — Survival (900307)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900307 | Chance to drop Explosive Trap on damage dealt | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_RANGED_AUTO_ATTACK|DONE_SPELL_RANGED_DMG_CLASS` (0x44). `HandleProc`: Chance X% → CastSpell(Explosive Trap, triggered=true) an Target-Position. Explosive Trap (49067) normalerweise am Caster platziert — braucht evtl. modifizierte Version die am Ziel spawnt (`SummonGameObject` an Target-Coords). Oder: Direkt den Explosion-Damage-Spell (49064) am Target casten. |

> **Helper-Spells Hunter**: 900304 (Pet AoE) braucht AoE-Damage-Spell (z.B. 900308). 900305 (Auto Shot Bounce) braucht Bounce-Damage-Spell (z.B. 900309). 900306 (Multi-Shot Channel) braucht eigene Channel-Spell-DBC-Entry. 900307 (Explosive Trap on Damage) braucht evtl. modifizierte Trap-Spell-Variante. IDs ab 900308+ für Helper verfügbar.

> **Besonders aufwändig**: 900306 (Multi-Shot Autocast Channel) ist ein neuartiges Konzept — 20 Multi-Shots in 2s braucht sorgfältiges Balancing und Performance-Tests (viele Projectiles gleichzeitig). 900305 (Auto Shot Bounce) ist ein Custom-Chain-System das es im Base-Game nicht gibt.

---

### Rogue — Shared (900350)

> Rogue SpellFamilyName = 8. "Energy regen +50%" gilt für alle 3 Specs → shared Spell.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900350 | Energy regeneration +50% | DBC | geplant | Passive Aura: `SPELL_AURA_MOD_POWER_REGEN_PERCENT` +50% für Energy (Power Type 3). Rogue Base Energy Regen = 10/s → wird 15/s. Vergleichbar mit Vitality-Talent. SpellFamilyName=8, Target=Self. |

### Rogue — Assassination (900351-900353)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900351 | Mutilate damage +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Mutilate (SpellFamilyFlags für Mutilate 5374/27576). Oder `SPELL_AURA_MOD_DAMAGE_DONE_FOR_MECHANIC` spezifisch. Einfacher Damage-Multiplikator. |
| 2 | 900352 | Poison damage +50% | DBC/C++ | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf alle Poison-Spells (Instant Poison 57965, Deadly Poison 57970, Wound Poison 57975, etc.). Alle Poison-Spells haben gemeinsame SpellFamilyFlags → ein Aura-Effekt kann mehrere abdecken. Alternativ C++: `OnDamage`-Hook → Wenn Damage-Spell School=Nature + SpellFamily=Rogue → ×1.5. |
| 3 | 900353 | Poison damage has chance to unleash Poison Nova | C++ | geplant | Proc-Aura: `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS|NEG` mit SpellFamily-Filter auf Poison-Spells. `HandleProc`: Chance X% → CastSpell(Poison Nova Helper, triggered=true) am Target. Poison Nova = AoE Nature Damage um Target. Braucht Helper-AoE-Spell (z.B. 900360). ICD empfohlen (z.B. 3s). |

### Rogue — Combat (900354-900357)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900354 | Sinister Strike damage +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Sinister Strike (48638). SpellFamilyFlags für SS identifizieren. Einfacher Damage-Multiplikator. |
| 2 | 900355 | Sinister Strike +9 targets | C++/DBC | geplant | Sinister Strike (48638) ist Single-Target Melee. Ansatz: SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Melee-Radius. CastSpell(SS-Damage-Helper, triggered=true) auf jedes Target. Ähnlich wie Hunter Auto Shot Bounce (900305). Braucht Helper-Spell (z.B. 900361). |
| 3 | 900356 | Blade Dance (Blade Flurry) duration 2 min | DBC | geplant | Blade Flurry (13877) hat normalerweise 15s Duration. DBC-Patch: Duration auf 120000ms (2 min) setzen. Einfache DBC-Änderung. |
| 4 | 900357 | Blade Dance (Blade Flurry) +9 targets | C++/DBC | geplant | Blade Flurry (13877) trifft normalerweise 1 zusätzliches Ziel. DBC: `MaxAffectedTargets` auf 10 setzen. C++: Falls Target-Limit hardcoded → `OnObjectAreaTargetSelect` überschreiben. Blade Flurry kopiert Melee-Damage auf zusätzliche Targets → alle 9 extra Targets bekommen den Damage. |

### Rogue — Subtlety (900358-900359)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900358 | Hemorrhage damage +50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Hemorrhage (48660). SpellFamilyFlags für Hemorrhage. Einfacher Damage-Multiplikator wie Mutilate (900351). |
| 2 | 900359 | Hemorrhage +9 targets | C++ | geplant | Hemorrhage (48660) ist Single-Target Melee. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Melee-Radius. CastSpell(Hemorrhage-Damage-Helper, triggered=true) auf jedes. Gleicher Ansatz wie SS +9 (900355). Braucht Helper-Spell (z.B. 900362). |

> **Helper-Spells Rogue**: 900353 (Poison Nova) braucht AoE-Nature-Damage-Spell (z.B. 900360). 900355 (SS +9) braucht Chain-Damage-Helper (z.B. 900361). 900359 (Hemorrhage +9) braucht Chain-Damage-Helper (z.B. 900362). IDs ab 900360+ für Helper verfügbar.

> **Besonders aufwändig**: 900355 (SS +9 targets) und 900359 (Hemorrhage +9 targets) sind Custom-Chain-Melee-Systeme — Melee-Abilities auf mehrere Targets verteilen ist ungewöhnlich im Base-Game. 900357 (Blade Flurry +9 targets) könnte Performance-intensiv sein bei vielen Mobs.

---

### Mage — Shared (900400)

> Mage SpellFamilyName = 3. "Channeling Evocation increases spell damage" gilt für alle 3 Specs → shared Spell.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900400 | Channeling Evocation increases spell damage | C++ | geplant | Evocation (12051) ist ein Channel-Spell (8s). Ansatz: AuraScript auf Evocation → `OnApply`: Buff mit `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` (ALL_SCHOOLS) auf Caster stacken, pro Tick steigend. `OnRemove`: Buff bleibt X Sekunden oder permanent. Alternativ: Während Channel stacking Aura, z.B. +10% Spell Damage pro Sekunde Channel → bis +80% nach vollem Channel. Duration des Buffs nach Channel konfigurierbar. |

### Mage — Arcane (900401-900409)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900401 | Mana regen +2% per missing mana % | C++ | geplant | Dynamische Mana-Regen-Skalierung. `PlayerScript::OnUpdateManaRegen` oder periodic Aura-Tick: Berechne fehlende Mana% → setze Regen-Bonus = fehlende% × 2%. Bei 50% Mana fehlen → +100% Mana Regen. Bei 90% fehlen → +180%. Passive Aura mit `SPELL_AURA_OBS_MOD_POWER` oder C++ Hook auf `Player::RegenerateMana()`. Sehr starke Mana-Sustain-Mechanik. |
| 2 | 900402 | Arcane Barrage +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Arcane Barrage (44781). SpellFamilyName=3, SpellFamilyFlags für Barrage. Einfacher Damage-Multiplikator. |
| 3 | 900403 | Arcane Barrage +9 targets | DBC/C++ | geplant | Arcane Barrage (44781) trifft normalerweise 3 Targets. DBC: `MaxAffectedTargets` auf 10+ setzen. Oder C++: `OnObjectAreaTargetSelect` → Target-Limit entfernen. |
| 4 | 900404 | Arcane Blast cast time -50% | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` (SPELLMOD_CASTING_TIME) -50% auf Arcane Blast (42897). Base Cast Time 2.5s → 1.25s. Stapelt mit Arcane Blast Debuff (noch schneller mit Stacks). |
| 5 | 900405 | Arcane Blast +9 targets | C++ | geplant | Arcane Blast (42897) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Radius. CastSpell(AB-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900422). |
| 6 | 900406 | Arcane Charges stack up to 8 | DBC/C++ | geplant | Arcane Blast Debuff (36032) stackt normalerweise bis 4. DBC: `StackAmount` auf 8 setzen. C++: Falls hardcoded → `AuraScript::OnStackChange` → Allow stacks >4. Jeder Stack erhöht AB Damage +15% und Mana Cost +150% (Base-Werte). 8 Stacks = +120% Damage, +1200% Mana Cost. Balancing beachten! |
| 7 | 900407 | Arcane Explosion generates 1 Arcane Charge (like Arcane Blast) | C++ | geplant | Arcane Explosion (42921) ist AoE Instant. SpellScript `AfterCast` → ApplyAura(Arcane Blast Debuff 36032, 1 Stack) auf Caster. Gleiche Mechanik wie AB aber ohne Consume. AE wird zu einem AoE Arcane Charge Generator. |
| 8 | 900408 | Below 30% health → activate Mana Shield + restore all mana | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_TAKEN_DAMAGE` (0x4000). `HandleProc`: Wenn Health <30% → CastSpell(Mana Shield 43020, triggered=true) + SetPower(MANA, MaxMana). ICD empfohlen (z.B. 60s) um Abuse zu verhindern. Sehr starke Überlebens-Mechanik: Volle Mana + Shield bei Low HP. |
| 9 | 900409 | Blink target location selection | C++ | geplant | Blink (1953) teleportiert normalerweise 20yd vorwärts. Ansatz: Override Blink → Click-to-Blink mit Target-Location. SpellScript `HandleDummy`: Lese SpellDestination → Teleport Caster dorthin (max Range z.B. 40yd). DBC: Spell Target-Type auf `TARGET_DEST_DEST` ändern. Braucht Client-seitig: Spell zeigt Ground-Target-Cursor. Vergleichbar mit Heroic Leap Targeting. |

### Mage — Fire (900410-900415)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900410 | Fireball +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Fireball (42833). SpellFamilyName=3, SpellFamilyFlags für Fireball. Einfacher Damage-Multiplikator. |
| 2 | 900411 | Fireball +9 targets | C++ | geplant | Fireball (42833) ist Single-Target Projectile. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Radius. CastSpell(Fireball-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900423). |
| 3 | 900412 | Pyroblast +9 targets | C++ | geplant | Pyroblast (42891) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(Pyro-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900424). |
| 4 | 900413 | Pyroblast +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Pyroblast (42891). Einfacher Damage-Multiplikator. |
| 5 | 900414 | Fire Blast off GCD and usable while casting | DBC/C++ | geplant | Fire Blast (42873): DBC → `StartRecoveryCategory` = 0 (off GCD). Attribute `SPELL_ATTR4_CAN_CAST_WHILE_CASTING` setzen. Ermöglicht Fire Blast als Weave-Spell während Fireball/Pyroblast Cast. Vergleichbar mit Retail-Fire-Mage Design. |
| 6 | 900415 | Pyroblast now triggers Hot Streak | C++ | geplant | Hot Streak (48108) proct normalerweise bei 2 aufeinanderfolgenden Crits. Ansatz: Pyroblast Cast (auch non-crit) → automatisch Hot Streak Buff (48108) applyen. SpellScript auf Pyroblast `AfterCast` → AuraScript: ApplyAura(Hot Streak). Effekt: Jeder Pyroblast gibt guaranteed nächsten Instant Pyroblast. Extrem starker Damage-Loop! |

### Mage — Frost (900416-900421)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900416 | Frostbolt +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Frostbolt (42842). Einfacher Damage-Multiplikator. |
| 2 | 900417 | Frostbolt +9 targets | C++ | geplant | Frostbolt (42842) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(Frostbolt-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900425). |
| 3 | 900418 | Ice Lance +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Ice Lance (42914). Einfacher Damage-Multiplikator. |
| 4 | 900419 | Ice Lance +9 targets | C++ | geplant | Ice Lance (42914) ist Single-Target Instant. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(Ice-Lance-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900426). |
| 5 | 900420 | Water Elemental is permanent | DBC/C++ | geplant | Summon Water Elemental (31687) hat normalerweise 45s Duration. DBC: Duration auf -1 (permanent) oder sehr hohen Wert setzen. C++: Falls Duration hardcoded → `OnSummon` Hook → SetDuration(0) (permanent). Ähnlich wie Hunter Pet — Water Ele bleibt bis Tod/Dismiss. |
| 6 | 900421 | Frost Comet Shower spell | C++ | geplant | Neuer aktiver Spell: Channel oder Instant → Spawnt mehrere Frost-Comets an zufälligen Positionen im Target-Bereich über X Sekunden. Jeder Comet = AoE Frost Damage + Slow. Implementierung: Periodic Trigger → SummonGameObject oder CastSpell(Comet-Impact-Helper) an Random-Position im Radius. Visuell: Blizzard-ähnlich aber mit größeren Einzel-Impacts. Braucht Helper-Spells + ggf. Custom Visual. Vergleichbar mit Meteor (Fire) aber Frost-themed. |

> **Helper-Spells Mage**: 900405 (AB +9) → Helper 900422. 900411 (Fireball +9) → Helper 900423. 900412 (Pyro +9) → Helper 900424. 900417 (Frostbolt +9) → Helper 900425. 900419 (Ice Lance +9) → Helper 900426. 900421 (Comet Shower) braucht Impact-Helper + evtl. Visual-Spells. IDs ab 900422+ für Helper verfügbar.

> **Besonders aufwändig**: 900401 (Mana Regen Scaling) braucht dynamische Berechnung pro Regen-Tick. 900409 (Blink Target Location) braucht Client-seitig Ground-Target-Cursor — evtl. DBC-Spell-Target-Type-Patch nötig. 900415 (Pyro → Hot Streak Loop) ist ein guaranteed Instant-Pyro-Chain — extrem starker Burst, Balancing kritisch. 900421 (Frost Comet Shower) ist ein komplett neuer Spell mit Custom-Visuals. Fire Meteor (nicht explizit als ID, wird in 900421-Beschreibung referenziert) ist ähnlich aufwändig.

---

### Warlock — Affliction (900450-900452)

> Warlock SpellFamilyName = 5. Affliction fokussiert auf DoT-Verstärkung und DoT-Spread.

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900450 | DoTs have a chance to deal shadow damage AoE | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_PERIODIC` (0x400000). `HandleProc`: Wenn Periodic-Damage-Tick von Warlock-DoT → Chance X% → CastSpell(Shadow-AoE-Helper, triggered=true) zentriert auf DoT-Target. Shadow AoE = Area Damage um das Target. Braucht Helper-AoE-Spell (z.B. 900466). ICD empfohlen (z.B. 2s). |
| 2 | 900451 | Corruption +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Corruption (47813). SpellFamilyName=5, SpellFamilyFlags für Corruption. Einfacher Damage-Multiplikator für Periodic + Initial Damage. |
| 3 | 900452 | DoTs have a chance to spread to 2 additional targets on tick | C++ | geplant | Passive Proc-Aura: `PROC_FLAG_DONE_PERIODIC`. `HandleProc`: Wenn DoT-Tick → Chance X% → Finde 2 nächste Feinde im Radius die den DoT NICHT haben → ApplyAura(gleicher DoT) auf sie. Muss DoT-Spell-ID aus ProcEventInfo extrahieren und auf neue Targets casten. Sehr starke Spread-Mechanik — kann exponentiell wachsen! Braucht evtl. Max-Target-Cap pro Cast. |

### Warlock — Demonology (900453-900460)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900453 | Killing an enemy increases Demon Form duration | C++ | geplant | Metamorphosis (47241) hat 30s Duration. `PlayerScript::OnKill` oder `KillCredit`-Hook: Wenn Player in Metamorphosis (HasAura 47241) → ExtendAura Duration um X Sekunden (z.B. +5s pro Kill). `Aura::SetDuration(GetDuration() + 5000)`. Kein Cap oder mit Cap (z.B. max 120s). Farming-Sustain-Mechanik. |
| 2 | 900454 | Demon Form: periodic shadow AoE + self heal | C++ | geplant | Passive Aura aktiv nur während Metamorphosis (47241). Periodic Tick alle X Sekunden → CastSpell(Shadow-AoE-Helper, triggered=true) um Caster + CastSpell(Heal-Helper, triggered=true) auf Caster. Heal = % des Damage dealt oder flat. Ansatz: AuraScript auf Metamorphosis → `OnPeriodicTick` oder separater Periodic-Trigger-Spell der nur aktiv ist wenn Meta-Aura vorhanden. |
| 3 | 900455 | Demons have chance to spawn lesser version of themselves | C++ | geplant | Proc-Aura auf Warlock-Pet: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4). `HandleProc`: Chance X% → SummonCreature(Lesser-Version-NPC) an Pet-Position. Lesser Version: Temporärer NPC (z.B. 30s Duration), reduzierte Stats (50% HP/Damage), gleicher NPC-Typ aber mit Suffix. Braucht Custom Creature-Templates pro Pet-Typ (Lesser Imp, Lesser Felguard, etc.). ICD empfohlen (z.B. 30s). |
| 4 | 900456 | Imp Firebolt +50% damage | DBC | geplant | Passive Aura auf Warlock (transferred to Pet): `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Imp Firebolt (47964). Oder: Aura direkt auf Pet via Owner-Aura-Scaling. Einfacher Damage-Multiplikator. |
| 5 | 900457 | Imp Firebolt +9 targets | C++ | geplant | Imp Firebolt (47964) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden im Radius. CastSpell(Firebolt-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900467). Imp wird zum AoE-Caster. |
| 6 | 900458 | Felguard AoE unlimited targets | DBC/C++ | geplant | Felguard Cleave (47994) trifft normalerweise begrenzte Targets. DBC: `MaxAffectedTargets` entfernen/sehr hoch. C++: `OnObjectAreaTargetSelect` → kein Target-Limit. |
| 7 | 900459 | Felguard +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` +50% auf Felguard (alle Schools). Applied via Owner-to-Pet Aura Scaling. Einfacher Damage-Multiplikator für alle Felguard-Abilities. |
| 8 | 900460 | Sacrificing pet grants ALL pet bonuses | C++ | geplant | Demonic Sacrifice (18788) gibt normalerweise einen Buff abhängig vom geopferten Pet-Typ (Imp→Fire Dmg, VW→HP Regen, etc.). Ansatz: SpellScript Override → Beim Sacrifice → ApplyAura für ALLE Pet-Typ-Buffs gleichzeitig (Imp-Bonus + VW-Bonus + Succubus-Bonus + Felhunter-Bonus + Felguard-Bonus). Braucht Liste aller Sacrifice-Buff-IDs und Apply aller auf einmal. |

### Warlock — Destruction (900461-900465)

| # | Spell ID | Effekt | Ansatz | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900461 | Shadow Bolt +9 targets | C++ | geplant | Shadow Bolt (47809) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(SB-Damage-Helper, triggered=true). Braucht Helper-Spell (z.B. 900468). |
| 2 | 900462 | Shadow Bolt +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Shadow Bolt (47809). Einfacher Damage-Multiplikator. |
| 3 | 900463 | Chaos Bolt +50% damage | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% auf Chaos Bolt (59172). Einfacher Damage-Multiplikator. |
| 4 | 900464 | Chaos Bolt cooldown -2 sec | DBC | geplant | Passive Aura: `SPELL_AURA_ADD_FLAT_MODIFIER` (SPELLMOD_COOLDOWN) -2000ms auf Chaos Bolt (59172). Base CD 12s → 10s. Oder DBC direkt: `RecoveryTime` reduzieren. |
| 5 | 900465 | Chaos Bolt +9 targets | C++ | geplant | Chaos Bolt (59172) ist Single-Target. SpellScript `AfterHit` → Chain zu 9 weiteren Feinden. CastSpell(CB-Damage-Helper, triggered=true). Chaos Bolt ignoriert Resistances — Helper-Spell sollte das ebenfalls tun (Attribute `SPELL_ATTR0_NO_IMMUNITIES` o.ä.). Braucht Helper-Spell (z.B. 900469). |

> **Helper-Spells Warlock**: 900450 (DoT AoE) → Shadow-AoE-Helper 900466. 900457 (Imp Firebolt +9) → Helper 900467. 900461 (SB +9) → Helper 900468. 900465 (CB +9) → Helper 900469. 900455 (Lesser Demons) braucht Custom Creature-Templates. IDs ab 900466+ für Helper verfügbar.

> **Besonders aufwändig**: 900452 (DoT Spread) kann exponentiell wachsen — braucht Target-Cap um Server-Performance zu schützen. 900455 (Lesser Demon Spawn) braucht Custom Creature-Templates pro Pet-Typ mit eigener AI. 900460 (Sacrifice All Bonuses) muss alle Pet-Typ-Buffs korrekt identifizieren und stacken. 900453 (Meta Duration Extension) + 900454 (Meta AoE+Heal) zusammen machen Demo-Lock zu einem permanent transformierten AoE-Healer-Tank-Hybrid.

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
