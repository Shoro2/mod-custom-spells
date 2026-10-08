# todo.md - mod-custom-spells

Open tasks. Done items go to `log.md` and leave this list (never strike through).

## Owed checks

- **(high)** T2 in game: spellbook names/tooltips/icons of the picker spells and of the concept spells
  (client patch-9 of 2026-10-08), one class of the operator's choice with the new spells; the second-pet
  bar (hunter or warlock).
- **(high)** Client check owed, no rogue/druid test character on CRTEST1-4: Backstab/Shred from the front
  with 900674/901053 (does the 3.3.5a client send the cast?), and Shadow Dance: Flow's 0.5 s GCD (does the
  client keep its own 1 s?). The server side is bot-tested.
- **(medium)** Host: share-public `forgotten-land/15` MIG-091 (modules), MIG-092 (Lua), MIG-093 (patch-9),
  in the HOST9 window.
- **(low)** Bot check "dismiss right after a demon swap" (the swap window closes once a pet is out).

## Concept gaps

- **(low)** "You can now cast while moving" (901100): the 3.3.5a client cancels the cast on movement
  before the server is asked - needs client work (FLStream.dll), not a spell.
- **(low)** 900737 Fire Blast "usable while casting" and "inside another spell's global cooldown": the
  client and the core's `CheckCast` refuse both before any script runs.
- **(low)** Cursed-item passives "only applied while in this spec" (concept): an item's passive works in
  any spec today.

## Behaviour notes (working, not concept-exact)

- **(low)** The warlock's second demon is remembered in memory only: a logout ends it; its level and
  spells are frozen at the swap. Arenas and battlegrounds do not restrict either second pet.
- **(low)** 900845 Fel Fury (+100 % in Metamorphosis) reaches every guardian of the warlock, not only demons.
- **(low)** The free Drain Life / Mind Flay can miss (no always-hit attribute); Atonement 900904 heals
  players only, not pets.
- **(low)** A second, different stabled pet instead of the copy (operator's option C, own project).
- **(low)** 900406 Lava Burst two charges: both charges recharge together (a cooldown toggle), not one at a
  time.
- **(low)** 900437 Spirit Wolves get the shaman's haste once, at their summon.
- **(low)** 901069 Thorns Rejuvenation reacts to any direct damage on the Thorns target, not only melee.

## Cleanup

- **(low)** Dead helper rows 900176, 900276, 900305, 900506 (old bounce helpers no script casts):
  retire them from the class SQL files (`client_spell_sync.py` already skips them).
