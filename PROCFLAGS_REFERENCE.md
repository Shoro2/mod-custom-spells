# ProcFlags Reference (Corrected)

> **IMPORTANT**: This table replaces the ProcFlags table in `CLAUDE.md` (which had wrong values and led to bugs in several `spell_proc` entries). Verified against `azerothcore-wotlk/src/server/game/Spells/SpellMgr.h`.

## Old (incorrect) values in CLAUDE.md

```
PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK = 0x2     ❌  (was actually KILL)
PROC_FLAG_DONE_PERIODIC           = 0x400000 ❌  (was actually DONE_MAINHAND_ATTACK)
PROC_FLAG_KILL                    = 0x1     ❌  (was actually KILLED)
PROC_FLAG_TAKEN_DAMAGE            = 0x4000   ❌  (was actually DONE_SPELL_MAGIC_DMG_CLASS_POS)
```

## Correct values (from AzerothCore SpellMgr.h)

| Flag | Hex | Bit | Meaning |
|------|-----|-----|-----------|
| `PROC_FLAG_NONE` | `0x0` | — | No proc |
| `PROC_FLAG_KILLED` | `0x1` | 0 | Own death (killed by an aggressor) |
| `PROC_FLAG_KILL` | `0x2` | 1 | Killed an enemy (XP/honor-eligible) |
| `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` | `0x4` | 2 | Own melee auto-attack |
| `PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK` | `0x8` | 3 | Enemy melee auto-attack |
| `PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS` | `0x10` | 4 | Own melee spell |
| `PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS` | `0x20` | 5 | Enemy melee spell |
| `PROC_FLAG_DONE_RANGED_AUTO_ATTACK` | `0x40` | 6 | Own ranged auto-attack |
| `PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK` | `0x80` | 7 | Enemy ranged auto-attack |
| `PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS` | `0x100` | 8 | Own ranged spell (Steady/Aimed Shot) |
| `PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS` | `0x200` | 9 | Enemy ranged spell |
| `PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS` | `0x400` | 10 | Own positive non-magic spell |
| `PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS` | `0x800` | 11 | — |
| `PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG` | `0x1000` | 12 | Own negative non-magic spell |
| `PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG` | `0x2000` | 13 | — |
| `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS` | `0x4000` | 14 | Own positive magic spell (**heals!**) |
| `PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS` | `0x8000` | 15 | — |
| `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG` | `0x10000` | 16 | Own negative magic spell |
| `PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG` | `0x20000` | 17 | — |
| `PROC_FLAG_DONE_PERIODIC` | `0x40000` | 18 | Own DoT/HoT tick |
| `PROC_FLAG_TAKEN_PERIODIC` | `0x80000` | 19 | Received DoT/HoT tick |
| `PROC_FLAG_TAKEN_DAMAGE` | `0x100000` | 20 | Any damage taken |
| `PROC_FLAG_DONE_TRAP_ACTIVATION` | `0x200000` | 21 | Trap activation |
| `PROC_FLAG_DONE_MAINHAND_ATTACK` | `0x400000` | 22 | Main-hand hit (auto+spell) |
| `PROC_FLAG_DONE_OFFHAND_ATTACK` | `0x800000` | 23 | Off-hand hit (auto+spell) |
| `PROC_FLAG_DEATH` | `0x1000000` | 24 | Dies |

## Common combinations

| Hex | Purpose | Examples |
|-----|-------|-----------|
| `0x14` | Own melee auto + melee spell | Bladestorm CD reduce, Death Coil proc, extra attack |
| `0x140` | Own ranged auto + ranged spell | Hunter Trap proc |
| `0x10014` | Melee auto + melee spell + magic neg | Cleave proc |

## HitMask: on-block procs need an explicit mask (found 2026-07-19)

`spell_proc.HitMask = 0` does **not** mean "any hit". `SpellMgr::CanSpellTriggerProcOnEvent`
substitutes a default for TAKEN procs: `PROC_HIT_NORMAL | PROC_HIT_CRITICAL`.
A **full block** nullifies the damage, so the event's hit mask only carries
`PROC_HIT_BLOCK (0x40) | PROC_HIT_FULL_BLOCK (0x2000)` — the default never
matches and the proc silently never fires. (Partial blocks sneak through
because they still carry `PROC_HIT_NORMAL`; with high block value vs. normal
mobs nearly every block is a full block, so "on block" procs looked dead.)

**Rule:** every on-block `spell_proc` entry needs `HitMask = 0x2040`
(`PROC_HIT_BLOCK | PROC_HIT_FULL_BLOCK`). Applied to 900173 (Block→Enhanced TC).
900172 (Block→AoE) was retired the same day — reworked into the Devastate
Lightning cast hook, which is not a proc aura.

## Bugs that this fix corrected

The following `spell_proc` entries had wrong ProcFlags (ProcFlags = 0x2 misread as KILL; 0x400000 misread as DONE_PERIODIC). Corrected in `mod_custom_spells_*.sql`:

| SpellId | Description | Old | New |
|---------|-------------|-----|-----|
| 900172 | Block→AoE | `0x2` (KILL) ❌ | `0x8` TAKEN_MELEE_AUTO ✓ |
| 900173 | Block→Enhanced TC | `0x2` ❌ | `0x8` ✓ |
| 900366 | DK Unholy DoT-AoE | `0x400000` (MAINHAND) ❌ | `0x40000` DONE_PERIODIC ✓ |
| 900405 | Shaman FS-Reset-LvB | `0x400000` ❌ | `0x40000` ✓ |
| 900566 | Hunter Trap Proc | `0x44` (Melee+Ranged Auto) ❌ | `0x140` (Ranged Auto+Spell) ✓ |
| 901066 | Druid HoT→Treant | `0x400000` ❌ | `0x40000` ✓ |
| 900933 | Priest Heal→Holy-Fire | `0x10000` (Magic-Neg) ❌ | `0x4000` (Magic-Pos) ✓ |
| 901101 | Global Kill→Heal | `0x1` (KILLED) ❌ | `0x2` KILL ✓ |
| 901104 | Global Counter-Attack | `0x2` (KILL) ❌ | `0x8` TAKEN_MELEE_AUTO ✓ |
