# Change log

Newest entry first. Changes before 2026-09-28 are recorded only in
[`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md).

## 2026-10-08 — Operator pause and original tooltip scope

The five native Mage emergency-shield cases and their exact owned cleanup passed.
The subsequent Warlock custom-picker hover check did not test the requested Mage
spellbook tooltips. That original task remains open; the picker title screenshots
are not spellbook or description-body acceptance. The operator stopped testing
and requested publication of the existing work before continuing another day.
No new test or host deployment accompanies this source snapshot.

## 2026-10-04 — Spell-row hovers and emergency shield eligibility

- Native spell-link hovers cover the existing checkbox and its label; owned
  tooltips clear on leave/hide, including pooled-row repaint. The server
  whitelist, GM rank checks and state payload remain unchanged.
- Emergency Mana Shield900708 now rejects ineligible hits in CheckProc, before
  core proc preparation spends its60s cooldown. Chance, cooldown, inclusive
  <=30% threshold and the existing effect handler are preserved.
- T0 on the Windows operator box: Lua5.2 syntax check and source/DB/core proc
  order review. No build, Lua deployment, SQL, restart, patch9 or native T1/T2:
  shared resources await the coordinator's grant. Wave3 is untouched.
- Paired vault change corrects Mage IDs and distinguishes original concept
  from current source behavior; coordinator owns shared queue/log/MIG updates.

## 2026-09-28 — GM-only "Learn all Talents" in `/spells` (branch `claude/ptr-template-kit-8e5054`)

- feat(lua): the `State` payload carries a GM flag (`1` when `player:GetGMRank()` is at least
  `SEC_GAMEMASTER` 2, else `0`); the client shows a **Learn all Talents** button in its own
  row above Learn All / Forget All only for `1` and lifts the list by one row while it is
  shown. A click sends `LearnAllTalents`; the server re-checks the rank, runs
  `player:RunCommand("forgotten learnall <own guid>")` (mod-forgotten-talents: every
  Forgotten Talents node at its maximum rank, free) and answers with `State`. The core
  applies the command's `RBAC_PERM_COMMAND_MODIFY` and GM log as if typed; the GUID keeps a
  selected player from receiving the learn-all. Learn All / Forget All and `Toggle` are
  unchanged.
- Evidence: T0 only — both files parse under Lua 5.2 (`luac -p`), and a mock harness ran
  the server handlers (GM, moderator, player, uint32-max GUID) and the client panel (flag
  on/off/absent, list anchor 48 ↔ 76, click payload). Needs mod-forgotten-talents with
  `.forgotten learnall` in the worldserver; deploy the pair, then relog or `/aio reset`.
