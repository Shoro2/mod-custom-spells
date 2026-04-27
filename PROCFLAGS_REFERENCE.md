# ProcFlags Reference (Corrected)

> **WICHTIG**: Diese Tabelle ersetzt die ProcFlags-Tabelle in `CLAUDE.md` (die hatte falsche Werte und führte zu Bugs in mehreren spell_proc-Einträgen). Verifiziert gegen `azerothcore-wotlk/src/server/game/Spells/SpellMgr.h`.

## Alte (falsche) Werte in CLAUDE.md

```
PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK = 0x2     ❌  (war eigentlich KILL)
PROC_FLAG_DONE_PERIODIC           = 0x400000 ❌  (war eigentlich DONE_MAINHAND_ATTACK)
PROC_FLAG_KILL                    = 0x1     ❌  (war eigentlich KILLED)
PROC_FLAG_TAKEN_DAMAGE            = 0x4000   ❌  (war eigentlich DONE_SPELL_MAGIC_DMG_CLASS_POS)
```

## Korrekte Werte (aus AzerothCore SpellMgr.h)

| Flag | Hex | Bit | Bedeutung |
|------|-----|-----|-----------|
| `PROC_FLAG_NONE` | `0x0` | — | Kein Proc |
| `PROC_FLAG_KILLED` | `0x1` | 0 | Eigener Tod (von einem Aggressor getötet) |
| `PROC_FLAG_KILL` | `0x2` | 1 | Gegner getötet (XP/Honor-würdig) |
| `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` | `0x4` | 2 | Eigener Melee-Autoattack |
| `PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK` | `0x8` | 3 | Gegnerischer Melee-Autoattack |
| `PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS` | `0x10` | 4 | Eigener Melee-Spell |
| `PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS` | `0x20` | 5 | Gegner-Melee-Spell |
| `PROC_FLAG_DONE_RANGED_AUTO_ATTACK` | `0x40` | 6 | Eigener Ranged-Autoattack |
| `PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK` | `0x80` | 7 | Gegner-Ranged-Autoattack |
| `PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS` | `0x100` | 8 | Eigener Ranged-Spell (Steady/Aimed Shot) |
| `PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS` | `0x200` | 9 | Gegner-Ranged-Spell |
| `PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS` | `0x400` | 10 | Eigener positiver Non-Magic-Spell |
| `PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS` | `0x800` | 11 | — |
| `PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG` | `0x1000` | 12 | Eigener negativer Non-Magic-Spell |
| `PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG` | `0x2000` | 13 | — |
| `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS` | `0x4000` | 14 | Eigener positiver Magic-Spell (**Heals!**) |
| `PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS` | `0x8000` | 15 | — |
| `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG` | `0x10000` | 16 | Eigener negativer Magic-Spell |
| `PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG` | `0x20000` | 17 | — |
| `PROC_FLAG_DONE_PERIODIC` | `0x40000` | 18 | Eigener DoT/HoT-Tick |
| `PROC_FLAG_TAKEN_PERIODIC` | `0x80000` | 19 | Erhaltener DoT/HoT-Tick |
| `PROC_FLAG_TAKEN_DAMAGE` | `0x100000` | 20 | Beliebigen Schaden erhalten |
| `PROC_FLAG_DONE_TRAP_ACTIVATION` | `0x200000` | 21 | Trap-Aktivierung |
| `PROC_FLAG_DONE_MAINHAND_ATTACK` | `0x400000` | 22 | Mainhand-Hit (Auto+Spell) |
| `PROC_FLAG_DONE_OFFHAND_ATTACK` | `0x800000` | 23 | Offhand-Hit (Auto+Spell) |
| `PROC_FLAG_DEATH` | `0x1000000` | 24 | Stirbt |

## Häufige Kombinationen

| Hex | Zweck | Beispiele |
|-----|-------|-----------|
| `0x14` | Eigener Melee-Auto + Melee-Spell | Bladestorm CD-Reduce, Death Coil Proc, Extra Attack |
| `0x140` | Eigener Ranged-Auto + Ranged-Spell | Hunter Trap Proc |
| `0x10014` | Melee-Auto + Melee-Spell + Magic-Neg | Cleave Proc |

## Bugs die durch diesen Fix korrigiert wurden

Folgende `spell_proc`-Einträge hatten falsche ProcFlags (ProcFlags = 0x2 als KILL fehlinterpretiert; 0x400000 als DONE_PERIODIC fehlinterpretiert). Korrigiert in `mod_custom_spells_*.sql`:

| SpellId | Beschreibung | Alt | Neu |
|---------|-------------|-----|-----|
| 900172 | Block→AoE | `0x2` (KILL) ❌ | `0x8` TAKEN_MELEE_AUTO ✓ |
| 900173 | Block→Enhanced TC | `0x2` ❌ | `0x8` ✓ |
| 900366 | DK Unholy DoT-AoE | `0x400000` (MAINHAND) ❌ | `0x40000` DONE_PERIODIC ✓ |
| 900405 | Shaman FS-Reset-LvB | `0x400000` ❌ | `0x40000` ✓ |
| 900566 | Hunter Trap Proc | `0x44` (Melee+Ranged-Auto) ❌ | `0x140` (Ranged-Auto+Spell) ✓ |
| 901066 | Druid HoT→Treant | `0x400000` ❌ | `0x40000` ✓ |
| 900933 | Priest Heal→Holy-Fire | `0x10000` (Magic-Neg) ❌ | `0x4000` (Magic-Pos) ✓ |
| 901101 | Global Kill→Heal | `0x1` (KILLED) ❌ | `0x2` KILL ✓ |
| 901104 | Global Counter-Attack | `0x2` (KILL) ❌ | `0x8` TAKEN_MELEE_AUTO ✓ |
