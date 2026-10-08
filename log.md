# Change log

Newest entry first. Changes before 2026-09-28 are recorded only in
[`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md).

## 2026-10-08 — 900715 Mirror Shield: no SpellPhaseMask on a taken proc

The HOST9 boot (bug-report thread) showed "`spell_proc` table entry for SpellId 900715 has `SpellPhaseMask`
value defined, but it won't be used for defined `ProcFlags` value" - the workbench had it since the rework.
Its proc reacts to hits taken (0x222A8), where the core ignores the phase mask: 2 -> 0 in
`mod_custom_spells_e_mage.sql`, no change in behaviour; functions.md pitfall 24.

## 2026-10-08 — Second pet: the pet's size and happiness

Operator (thread "Paragon-Klassenzauber"): "ja mitspiegeln" (the second hunter pet mirrors the pet's
happiness) and "außerdem ist ein pet deutlich kleiner als das andere".
- Size: both units sent the same look (31049) and scale (1.0); the client draws a unit with a pet number
  at its creature family's size and a creature without a family at its model's (client decompile,
  `0x0071C110`; functions.md pitfall 23) - the copy (900525, no family) was the Diseased Young Wolf's
  0.4 against the wolf family's 1.0. `ClientModelScale` mirrors the rule; the copy's scale = the pet's x
  the ratio, with a pet's reach, kept every 0.5 s (`ApplySecondPetSize`).
- Happiness: 900526 gets a second effect, physical damage done (weapon damage only); `MirrorHappiness`
  sets +25 / 0 / -25 from the pet's happiness state, the pet's own 125 / 100 / 75 %.
- `.cspet status`: a "Second pet look" line (scale, pet scale, size ratio, happiness, the weapon damage
  factor against the pet's own with its happiness - 1 while mirrored);
  `cs_second_pet.tbs` checks both (Snow Tracker Wolf 604: ratio 1.818; the Voidwalker copy 1).

## 2026-10-08 — Revision: the operator's second concept list

Operator 2026-10-08 16:38: a revised class-spell list (share-public
`custom-spells/09-concept-revision-20261008.md`); later the same day "Felhunter: Spell Lock is aoe now,
Devour Magic also" and option "B" for the second pet (its own AIO control bar, inside this round).
- 9 built spells changed: Paragon damage (666 + 5 x Paragon level) for Beast Cleave, Poison Nova (now 5 %
  on poison damage), both Shadow Eruptions and the shield explosion; Explosive Traps on any physical damage
  (the Explosive Trap Effect at the target, 900567 retired), Starfall +50 % area (901002), Metamorphosis:
  Fel Vigor (900834: Immolation Aura heals, demons +100 %), Holy Fire Heals (50 %, one random enemy, 30 yd).
- 44 new lines, 45 new picker passives: Hunter 900507/900537-900540/900568/900571/900572, second pet
  900525 (+ Warlock 900858, `custom_spells_second_pet.cpp`, `lua/CustomSpells_PetBar_*.lua`, `.cspet`),
  Druid 901006/901036/901037/901039/901052-901054, Rogue 900605/900639/900642/900670-900672/900674,
  Warlock 900804/900846/900848/900850/900852/900873/900875, Priest 900904-900906/900935-900938/900941/
  900942/900969-900972/900975; creatures 900525, 900964, 900999.
- Found by the bots and fixed: Mutilate's extra targets belong on its strikes; a percent GCD modifier only
  works for Backdraft (Shadow Dance: Flow is a flat -500 ms); the tentacle and the guardian need
  UNIT_FLAG_PLAYER_CONTROLLED against neutral mobs; Running Aim counts a server spline as movement.
- Found by the client probe and fixed: the pet bar's spell flag shadowed its autocast texture (Lua error,
  the bar never showed).
- Tests: `tests/cs_rev_<class>.tbs` (hunter, druid, rogue, warlock, priest), `tests/cs_second_pet.tbs`;
  docs: functions.md (revision section, pitfalls 19-22), data_structure.md, todo.md, CustomSpells.md.
- Evidence: T1 on the Windows workbench (bot runs in the vault doc 09 §6, client probe of the bar);
  T2 owed in game.

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
