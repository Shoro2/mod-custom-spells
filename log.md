# Change log

Newest entry first. Changes before 2026-09-28 are recorded only in
[`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md).

## 2026-10-08 — Rework: bot-tested spells, concept spells, spellbook texts, cursed passives

Operator request: revise, test and fix the `/spells` picker spells with server-side bots, give
them spellbook tooltips, then add them to the cursed-item passive pool (CoA classes out of scope);
then build the concept spells that never existed ("ja neue spells im anschluss bauen").
- Fixes from code review and bot runs in all ten classes and the global block (list: share-public
  `custom-spells/08-rework-20261008.md` §3): modifiers on the wrong op or mask, dead Fury combos,
  front-only Thunder Clap, "+9 targets" = 10 targets, Holy Shock same-map pick, Critical Execution
  crit gate, Arcane charges via SPELLMOD_MAX_AURA_STACKS, Fire Blast GCD via OnSpellPrepare, Hot
  Streak loop, Holy Fire helper without facing, minion auras (pet/summon damage and haste, totem
  damage through the owner), mana regeneration by missing mana, Barrage slow, Mutilate mask,
  triggered poisons proc Poison Nova, Starfall stacks, summon health/despawn heal, thread-safe ICDs,
  Divine Storm +6 targets through the core's own target cap (aura 277; the old AfterHit script hit
  Divine Storm's own targets a second time).
- 24 concept spells: Warrior 900133; DK 900306/900334-900340/900369-900373; Shaman 900410-900418;
  Druid 901035; Mage 900714-900716 and the active 900741 Meteor; global 901111 (+5 % damage).
  "Cast while moving" needs client work and stays out.
- `data/spellbook_enUS.json` + `tools/client_spell_sync.py`: names, tooltips, icons and helper buff
  tooltips into both client `Spell.dbc` carriers; picker labels from the same file. Replaces the
  2026-10-04 draft (`data/tooltips_enUS.json`, `tools/patch_tooltips.py`).
- Bot scenarios `tests/cs_<class>.tbs` and `tests/cs_concept_*.tbs` (mod-fl-testbots, slot 4).
- Docs: CLAUDE.md slimmed under 8 KB; `INDEX.md`, `functions.md`, `data_structure.md`, `todo.md` new.
- Evidence: T1 on the Windows workbench (bot runs listed in the vault doc §5); T2 owed in game.

## 2026-10-08 — Operator pause and original tooltip scope

The five native Mage emergency-shield cases and their exact owned cleanup passed.
The subsequent Warlock custom-picker hover check did not test the requested Mage
spellbook tooltips. That original task remains open; the picker title screenshots
are not spellbook or description-body acceptance. The operator stopped testing
and requested publication of the existing work before continuing another day.
No new test or host deployment accompanies this source snapshot.

## 2026-10-04 — Offline tooltip data draft

- `data/tooltips_enUS.json`:138 descriptions covering135 empty picker texts,
  preserving2 Wave3 descriptions and replacing900106's literal Unknown String.
  Concept/source conflicts remain explicit review notes; this is not released.
- `tools/patch_tooltips.py`: writes separate complete DBC candidates, preserving
  gameplay dwords, IDs/order, other string slots and the existing string block;
  validates exact missing-row allowances and reuses strings for idempotence.
- T0 on Windows: actual server/hot/overlay candidates verified byte-idempotent
  with no empty present picker descriptions; gameplay preserved. Separate137-row
  SQL candidates preserve every existing value and non-INSERT statement.
  No source SQL, live DB, shared carrier or MPQ was changed. Native text/locale
  verification and concept decisions remain pending with the coordinator.

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
