# todo.md - mod-custom-spells

Open tasks. Done items go to `log.md` and leave this list (never strike through).

## Owed checks

- **(high)** T2 in game: spellbook names/tooltips/icons of the picker spells and of the concept spells
  (client patch-9 of 2026-10-08), one class of the operator's choice with the new spells.
- **(medium)** Host: share-public `forgotten-land/15` MIG-091 (modules), MIG-092 (Lua), MIG-093 (patch-9).

## Concept gaps

- **(low)** "You can now cast while moving" (901100): the 3.3.5a client cancels the cast on movement
  before the server is asked - needs client work (FLStream.dll), not a spell.
- **(low)** 900737 Fire Blast "usable while casting" and "inside another spell's global cooldown": the
  client and the core's `CheckCast` refuse both before any script runs.
- **(low)** Cursed-item passives "only applied while in this spec" (concept): an item's passive works in
  any spec today.

## Behaviour notes (working, not concept-exact)

- **(low)** 900406 Lava Burst two charges: both charges recharge together (a cooldown toggle), not one at a
  time.
- **(low)** 900437 Spirit Wolves get the shaman's haste once, at their summon.
- **(low)** 901069 Thorns Rejuvenation reacts to any direct damage on the Thorns target, not only melee.

## Cleanup

- **(low)** Dead helper rows 900176, 900276, 900305, 900506 (old bounce helpers no script casts):
  retire them from the class SQL files (`client_spell_sync.py` already skips them).
