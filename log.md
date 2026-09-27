# Change log

Newest entry first. Changes before 2026-09-28 are recorded only in
[`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md).

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
