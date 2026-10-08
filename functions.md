# functions.md - mod-custom-spells

How the module works and how to extend it. What it is: [`CLAUDE.md`](./CLAUDE.md); the file map:
[`data_structure.md`](./data_structure.md).

## Picker, spellbook texts and cursed-item pool (2026-10-08)

- **Picker** `/spells` (= `/cs`): `lua/CustomSpells_Server.lua` `CLASS_SPELLS` is the whitelist a class may
  learn; its labels are the spellbook names of `data/spellbook_enUS.json` (`tools/client_spell_sync.py --lua`
  rewrites them).
- **Spellbook texts**: `data/spellbook_enUS.json` holds name, tooltip and icon of every picker spell and the
  buff tooltips of the helper auras a player sees; `tools/client_spell_sync.py --write` writes them, plus the
  server rows' numbers, into both client `Spell.dbc` carriers (see `tools/README.md`), then the workspace's
  `30_build_hot_dbc_patch.py --deploy` ships `patch-9.MPQ`.
- **Cursed-item passives**: every passive of the picker is also a Paragon passive a cursed item can roll for
  its owner's spec. mod-paragon-itemgen's `tools/gen_class_passives.py` turns the manifest into
  `paragon_passive_spells_class.sql` (one equip-spell enchantment per spell, frozen ids 950007+); the
  player-castable actives (Barrage, Targeted Blink, Comet Shower, Meteor) stay out of the pool.
- **Bot scenarios** (mod-fl-testbots, slot 4): `tests/cs_<class>.tbs` per class, `tests/cs_concept_*.tbs`
  for the concept spells built on 2026-10-08. Queue one with
  `INSERT INTO acore_world.testbots_run (Scenario) VALUES ('cs_warrior');`.

## Revision 2026-10-08: the second concept list

The operator's second list (share-public `custom-spells/09-concept-revision-20261008.md`) added 44 lines
in Hunter, Druid, Rogue, Warlock and Priest and changed 9 built spells (Paragon damage = 666 + 5 x Paragon
level for the "damage by Paragon level" lines). Mechanics worth knowing before you touch them:
- **Second pet** (900525 hunter, 900858 warlock; `custom_spells_second_pet.cpp`): a plain Guardian
  (SummonProperties 61, marker aura 900526) next to the real pet - never `Player::GetPet()`, never the native
  pet bar. A PlayerScript reconciles it every 0.5 s (hunter: a copy of the current pet with its model, level,
  talents, bar spells and the owner's `spell_pet_auras`; warlock: the demon that was out when another one was
  summoned). Its own bar: `lua/CustomSpells_PetBar_*.lua` (AIO); clicks go to the server Lua, which validates
  them and runs `.cspet attack|follow|stay|aggressive|defensive|passive|cast|autocast|status|sync`
  (SEC_PLAYER, own second pet only); the C++ pushes the state back as addon whispers with prefix `CSPB`
  (`X` / `F` / `S` / `C` lines). Scripts that require `IsPet()` let the marker count too (Beast Cleave,
  Lesser Demons). What reaches only the real pet: everything the core aims at `GetPet()` (Mend Pet, Kill
  Command, Bestial Wrath, Spirit Bond, Soul Link). The hunter pet's happiness reaches the copy as the
  marker's second effect (physical damage done +25 / 0 / -25, set every 0.5 s = the pet's 125 / 100 / 75 %
  melee weapon damage), and the copy is drawn at the pet's size (pitfall 23); `.cspet status` prints both
  in its "Second pet look" line.
- **Behind the target from the front** (900674 rogue, 901053 druid; `custom_spells_rogue.cpp`): at startup the
  module clears `SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET` from every spell carrying it and enforces it itself
  (SPELL_FAILED_NOT_BEHIND) unless the caster has a marker (`AddFrontalAttackMarker`).
- **Finisher bursts** (901054 Berserk, 900639 Adrenaline Rush): `AddComboFrenzyRule` (rogue file) gives the
  speed aura while the cooldown lasts and a Paragon burst on every 5-combo-point finisher.
- **Stacking buffs with a timer of their own** (Lunar Frenzy 901007, Ursine Bulwark 901040): a refresh by
  CastSpell restarts every non-DoT periodic timer, so stacks are added with SetStackAmount + RefreshDuration
  (`AddStackKeepTicking`, druid file).
- **Free channels** (Drain Life 900806, Mind Flay 900974): periodic helper auras on the enemy, not channels,
  so the player's own casts go on; they pick only enemies already in combat.
- **Mobile Hellfire / Rain of Fire around you** (900873/900875): the cast's channel is ended and a self-aura
  ticker (900874/900876) deals the ticks around the warlock, like the mobile Consecration.
- **Summons that fight for the priest** (900964 Redemption Guardian, 900999 Lesser Tentacle) carry
  `UNIT_FLAG_PLAYER_CONTROLLED`: two creatures may fight only if one is hostile to the other, and the
  training dummies (and many mobs) are neutral.

## Picker UI

**Spell-row hover (2026-10-04).** Hovering a checkbox or its label opens
the native `spell:<id>` tooltip, including unlearned entries. Leaving/hiding the
row clears its owned tooltip, including pooled-row repaint. The descriptions
come from client Spell.dbc - since 2026-10-08 written from `data/spellbook_enUS.json`.

**GM-only "Learn all Talents" (2026-09-28).** The panel also carries a button for
mod-forgotten-talents: every Forgotten Talents node at its maximum rank, free of charge.
The server's `State` payload has a third argument, `1` when `player:GetGMRank()` is at least
`SEC_GAMEMASTER` (2) and `0` otherwise; the client shows the button in its own row above
Learn All / Forget All only for `1`. A click sends `LearnAllTalents`; the handler checks the
rank again (a forged request from a player is dropped like an unknown `Toggle`) and runs
`player:RunCommand("forgotten learnall <own guid>")`. That bridge was chosen over the
global `RunCommand` (console context: no player target, output only in the server log,
runs a tick later, and `.forgotten learnall` is `Console::No`) and over the client typing
the chat command (no server-side re-check): `player:RunCommand` runs the command in the
GM's own session, so the core enforces its `RBAC_PERM_COMMAND_MODIFY` permission and GM
command log as if typed, and the answer lands in the GM's chat frame. The explicit GUID
keeps a selected player from receiving the learn-all instead of the clicking GM. The
button needs mod-forgotten-talents built into the worldserver; without it the click
answers "Command '...' does not exist". Relog or `/aio reset` after deploying the pair.

## Spell editor convention (numeric values)


When working on custom spells, keep in mind:
- Numeric values (damage, healing, absorption) are **real in-game values**, not internal DBC encodings. The editor converts to DBC format: `EffectBasePoints = real_value − 1` (off-by-one). Detail: [03-procs-and-flags.md#off-by-one-basepoints](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/03-procs-and-flags.md#off-by-one-basepoints).
- WotLK balancing: low-level 30–150, mid 200–600, high 800–2500, boss 3000–10000+.
- Spells have at most 3 effects (Effect1/2/3 in `Spell.dbc`).
- Periodic effects: `AmplitudeSeconds` for the tick interval, `DurationSeconds` for the total duration.
- Summon spells must have `DurationSeconds` set (determines summon lifetime).
- Tooltip tokens: `$d` (duration), `$s1` (BasePoints effect 1), etc.
- Icon: passed as a semantic hint (e.g. "frost", "fiery melee strike") — the editor resolves via fuzzy match against `SpellIcon.dbc`.

## Two paths for custom spells (decision aid)


```
                 ┌──────────────────────────┐
                 │  Plan a new custom spell  │
                 └────────────┬─────────────┘
                              │
                 ┌────────────▼─────────────┐
                 │  Does the spell need C++? │
                 └──┬───────────────────┬───┘
                    │                   │
               No   │                   │ Yes
                    │                   │
        ┌───────────▼──────┐  ┌─────────▼──────────┐
        │  Path A: DBC only│  │  Path B: DBC + C++ │
        │  (spell_dbc SQL) │  │  (DBC + SpellScript)│
        └───────────┬──────┘  └─────────┬──────────┘
                    │                   │
                    └─────────┬─────────┘
                              │
                 ┌────────────▼─────────────┐
                 │  Patch the client DBC    │
                 │  (Spell.dbc for tooltips) │
                 └────────────┬─────────────┘
                              │
                 ┌────────────▼─────────────┐
                 │  Build the server + test │
                 └──────────────────────────┘
```

| Effect type | Path |
|------------|------|
| Damage ±X %, Cooldown ±X s, Cast time ±X %, unlimited targets, passive stat modifiers, SpellFamilyMask-based buffs | **Path A** (pure DBC) |
| Conditional procs, multi-spell triggers, single→AoE conversion, block/dodge/parry procs, custom damage formulas, runtime cooldown manipulation | **Path B** (DBC + C++ SpellScript / AuraScript) |

Full step-by-step recipe (with the 4 SpellScript patterns A/B/C/D, the `spell_dbc` insert example, `spell_script_names` registration, build & test, and a 12-item checklist) is in [`share-public/docs/custom-spells/04-adding-a-spell.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/04-adding-a-spell.md).

## Key APIs (SpellScript)


- `GetCaster()` / `GetHitUnit()` — caster and target units
- `SetHitDamage(amount)` / `GetHitDamage()` — override / read effect damage
- `GetSpellInfo()` — `SpellInfo` of the spell being cast
- `GetCaster()->ToPlayer()` — cast to `Player` for player-specific APIs
- `player->GetTotalAttackPowerValue(BASE_ATTACK)` — melee AP
- `player->GetAuraCount(auraId)` — aura stack count
- `player->ModifySpellCooldown(spellId, deltaMs)` — adjust cooldown (negative = reduce)
- `player->RemoveSpellCooldown(spellId, true)` — clear cooldown (with client update)
- `LOG_INFO("module", "format {}", args)` — logging
- `RegisterSpellScript(ClassName)` — register in `AddCustomSpellsScripts()`

When hooking on existing Blizzard spells via `spell_script_names`, the C++ class runs on **every** cast of that spell. Always check `HasAura(<marker_aura>)` and `sConfigMgr->GetOption<bool>("CustomSpells.Enable", true)` so the effect is only active when the player has the passive and the module is enabled.

## SpellFamilyName values


| Value | Class | Value | Class |
|------:|-------|------:|-------|
| 0 | Generic | 8 | Rogue |
| 3 | Mage | 9 | Hunter |
| 4 | Warrior | 10 | Paladin |
| 5 | Warlock | 11 | Shaman |
| 6 | Priest | 15 | Death Knight |
| 7 | Druid | | |

## Common pitfalls


1. **SpellFamilyFlags wrong**: ALWAYS verify against the project's own `Spell.dbc` (LOG_INFO debug pattern in [`03-spell-system.md`](https://github.com/Shoro2/share-public/blob/main/docs/03-spell-system.md#critical-always-verify-spellfamilyflags-via-debug-log)), never against online DBs (wowhead, wowdb).
2. **`MaxAffectedTargets=0` set globally**: this affects ALL players, not only those with the passive — for conditional targets use C++.
3. **Proc loop**: helper spells can re-proc → set ICD in `spell_proc` (`Cooldown` field) and / or check `SPELL_ATTR3_CAN_PROC_FROM_PROCS`.
4. **`spell_script_names` missing**: the C++ class is not loaded → spell has no effect.
5. **`DurationIndex` forgotten**: a passive aura needs `DurationIndex=21` (permanent).
6. **Attributes missing PASSIVE**: without `0x40` the spell is castable instead of permanently active.
7. **Off-by-one BasePoints**: writing `BasePoints=50` for "+50 %" yields **+51 %** in-game. Store `49`, not `50`. Detail: [`03-procs-and-flags.md#off-by-one-basepoints`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/03-procs-and-flags.md#off-by-one-basepoints). (Only applies with `EffectDieSides=1`; with `EffectDieSides=0` the value is used as-is.)
8. **`EffectSpellClassMaskA/B/C` are PER-EFFECT flag96 masks**: `A_1..A_3` = the three flag words of **effect 1**, `B_*` = effect 2, `C_*` = effect 3. A spellmod whose target spell has its family bit in `SpellFamilyFlags[1]` needs the mask in **`EffectSpellClassMaskA_2`** — writing it into `B_1` gives effect 1 an empty mask and the modifier silently affects nothing. Always verify the mask against the server `Spell.dbc` `SpellClassMask_1..3` of the target spell (2026-07-18 repair wave fixed 25+ rows with this bug).
9. **`EffectMiscValue` on aura 107/108 is the SpellModOp**: 0=DAMAGE, 1=DURATION, 4=CHARGES, 10=CASTING_TIME, 11=COOLDOWN, 14=COST, 17=JUMP_TARGETS, 22=DOT. "Instant"/"-X% cast" is op 10 (not 14), "duration" is op 1 and "double HoTs/DoTs" is op 22 (not 17).
10. **Extra-target damage must be dealt directly, not cast via helper spells**: `CastCustomSpell(target, HELPER, &damage, ...)` with a server-only helper produced no visible damage in-game. Use the T2-proven pattern instead: build `SpellNonMeleeDamage` with the ORIGINAL spell's `SpellInfo`, then `DealSpellDamage` + `SendSpellNonMeleeDamageLog` (heals: `HealInfo` + `HealBySpell`). The client renders the known spell id; no client DBC entry needed.
11. **`Attributes` 0x10000000 = SPELL_ATTR0_NOT_IN_COMBAT_ONLY_PEACEFUL** ("cannot be used in combat") — never put it on castable actives or helpers. Triggered casts bypass it, player casts do not.
12. **Area effects need `EffectRadiusIndex`** (13 = 10 yd): a `TARGET_UNIT_*_AREA_*` effect with radius index 0 searches a 0-yd radius and silently hits nothing (2026-07-18: eleven helpers in files b/c/d shipped without the column). Anchor semantics: 15/30 = around the caster, 16/31 = around the explicit cast target.
13. **Verify effect ids against `SharedDefines.h`**, never from memory: `SPELL_EFFECT_ADD_EXTRA_ATTACKS` is **19** (901108 shipped with 32 and did nothing).
14. **`EquippedItemClass` must be -1** ("no requirement") — the `spell_dbc` TABLE DEFAULT is 0 (= consumable), and the core's equipped-item gate (right after the proc `CheckProc` hook, plus `Spell::CheckItems`) then drops every proc and cast with "HasItemFitToSpellRequirements: Not handled spell requirement for item class 0". This silently killed ALL proc-driven customs until 2026-07-18; `mod_custom_spells_z_fixups.sql` forces -1 across the block. More generally: the table defaults are NOT DBC-neutral — always audit new columns' defaults against a real Spell.dbc row.
15. **"+9 targets" on a single-target spell is jump-target value 10**: `Spell::SelectImplicitChainTargets`
    adds `SPELLMOD_JUMP_TARGETS` to the effect's ChainTarget (0 for a single-target spell) and searches
    `max - 1` extra targets, so value 9 reaches 8 more enemies (fixed 2026-10-08 for every "+9").
16. **Triggered casts fire no proc aura** (`SpellMgr::CanSpellTriggerProcOnEvent`) and still check facing
    and range: a proc that casts a facing spell (Holy Fire) reaches only the enemies in front - cast a helper
    copy without the facing rule (900934).
17. **Global cooldown below 1 s is clamped** (`Spell::TriggerGlobalCooldown`, `MIN_GCD`): "off the GCD" needs
    `GetGlobalCooldownMgr().CancelGlobalCooldown` after the cast (900737).
18. **Two scripts on one spell run in no fixed order**: the core's Arcane Blast script re-applied 36032 after
    ours raised its stacks; a stack cap belongs in `SPELLMOD_MAX_AURA_STACKS` (31, this fork), not in C++.
19. **A percent `SPELLMOD_GLOBAL_COOLDOWN` (aura 108) does nothing** outside the Backdraft case:
    `Player::ApplySpellMod` applies it only when the same aura already modified that cast. Use a flat
    modifier (aura 107, e.g. -500 ms) - Shadow Dance: Flow 900673.
20. **The core queues casts** (`SpellQueue.Enabled = 1`, window 400 ms): a request within 400 ms of the end
    of a GCD or cast is queued silently and runs later, one with more left fails NOT_READY (67).
21. **Area caps: use aura 277** (`SPELL_AURA_MOD_MAX_AFFECTED_TARGETS` on the spell's mask). Hand-rolled
    extra-target damage hits the spell's own targets again (Divine Storm, 2026-10-08).
22. **A spell that only triggers its damage per target** (Mutilate 48666 -> 48665/48664) needs the jump
    targets on the triggered strikes: a strike triggered on a far chain target fails its range check.

23. **The client sizes a pet by its family, any other creature by its model** (build 12340, `0x0071C110`):
    a unit's base size is the model's (`CreatureDisplayInfo` x `CreatureModelData` scale), raised to its
    creature family's size at its level (`CreatureFamily.dbc` min/max scale and level) - and set to the
    family's size outright when the unit has a pet number; `OBJECT_FIELD_SCALE_X` multiplies that. Same
    look and same scale field can so draw differently: the Diseased Young Wolf (look 0.4) as a hunter pet
    at the wolf family's 1.0, its copy (creature 900525, no family) at 0.4. `ClientModelScale` in
    `custom_spells_second_pet.cpp` mirrors the rule; the copy's scale makes up the difference.

## Code style


AzerothCore conventions:
- 4-space indentation, no tabs
- `Type const*` (not `const Type*`)
- `UPPER_SNAKE_CASE` for spell/NPC constants with prefix `SPELL_CUSTOM_*`
- `UpperCamelCase` for class/method names
- No braces around single-line if/else/for/while

CI runs `apps/ci/ci-codestyle.sh` which rejects: trailing whitespace, tabs, multiple consecutive blank lines, and `LOG_*` calls that use `ObjectGuid::GetCounter()` (use `ObjectGuid::ToString().c_str()` instead).

## Loader convention


The loader function in `mod_custom_spells_loader.cpp` must be named `Addmod_custom_spellsScripts()` — module folder name with `-` replaced by `_`.
