# CLAUDE.md - mod-custom-spells

> **Central project wiki**: This module is part of a multi-repo WoW server project. The cross-cutting documentation, additional info, and Python tools live in the [share-public](https://github.com/Shoro2/share-public) repository:
> - [`docs/custom-spells/`](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells) — **Curated spec docs** (one file per class spec with status, source links, implementation notes), plus cross-cutting topics ([`05-complex-spells.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/05-complex-spells.md): recursion guard, target caps, ICDs, custom NPCs, OnRemove detection, channel/cast, owner→pet) and the ID block scheme ([`02-id-blocks.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/02-id-blocks.md))
> - [`docs/03-spell-system.md`](https://github.com/Shoro2/share-public/blob/main/docs/03-spell-system.md) — SpellScript/AuraScript class hierarchy + proc pipeline + DBC override
> - [`CLAUDE.md`](https://github.com/Shoro2/share-public/blob/main/CLAUDE.md) — Overall architecture (legacy, 60 KB deep reference)
> - [`claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md) — Change history, project plans, prioritized TODOs
> - [`python_scripts/`](https://github.com/Shoro2/share-public/tree/main/python_scripts) — DBC patching tools (`patch_dbc.py`, `copy_spells_dbc.py`), Paragon spell generator (`add_paragon_spell.py`)
> - [`dbc/`](https://github.com/Shoro2/share-public/tree/main/dbc) — All 246 WoW client DBC files (Spell.dbc, SpellItemEnchantment.dbc, etc.)
> - [`mysqldbextracts/`](https://github.com/Shoro2/share-public/tree/main/mysqldbextracts) — Complete DB column structure (`mysql_column_list_all.txt`), CSV exports (`creature_template.csv`, `item_template.csv`)
>
> **CustomSpells.md** in this repo remains the master ID catalog (source of truth for IDs, effects, status). `share-public/docs/custom-spells/` is the navigable, cross-linked view for dev/AI sessions.
>
> **All changes to this or the other repos must be logged there.**
>
> **When working on custom spells, keep in mind:**
> - Numeric values (damage, healing, absorption) are always **real in-game values**, not internal DBC encodings (BasePoints = player value, the editor converts to DBC format: `EffectBasePoints = BasePoints - 1`)
> - Respect WotLK balancing: low-level 30–150, mid 200–600, high 800–2500, boss 3000–10000+
> - Spells have at most 3 effects (Effect1/2/3 in spell.dbc)
> - Periodic effects: AmplitudeSeconds for the tick interval, DurationSeconds for the total duration
> - Summon spells must have DurationSeconds set (determines summon duration)
> - Spell tooltips can use tokens: `$d` (duration), `$s1` (BasePoints effect 1), etc.
> - The icon is given as a semantic hint (e.g. "frost", "fiery melee strike") — the editor resolves it via fuzzy match against SpellIcon.dbc

## Project Overview

AzerothCore module for defining custom spell effects via C++ SpellScripts. Each custom spell gets its own SpellScript class that hooks into the spell's DBC effects (e.g. School Damage) and overrides the damage/behavior.

## Doc cross-refs

| What are you looking for? | Look here |
|----------------|----------------|
| ID range of a class / next free ID | [`docs/custom-spells/02-id-blocks.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/02-id-blocks.md) |
| What does spell X do? Status? Implementation notes? | `docs/custom-spells/specs/<class>-<spec>.md` (e.g. [`warrior-arms`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warrior-arms.md)) |
| `spell_proc` setup, ProcFlags values, off-by-one BasePoints | [`docs/custom-spells/03-procs-and-flags.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/03-procs-and-flags.md) |
| Step-by-step for a new spell | [`docs/custom-spells/04-adding-a-spell.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/04-adding-a-spell.md) |
| Tricky pattern (recursion, target caps, custom NPCs, …) | [`docs/custom-spells/05-complex-spells.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/05-complex-spells.md) |

## Module Structure

```
mod-custom-spells/
├── src/
│   ├── mod_custom_spells_loader.cpp  # Module entry point, registers scripts
│   ├── custom_spells_common.h        # Shared header: enum, constants, includes
│   ├── custom_spells.cpp             # Main: calls per-class registration functions
│   ├── custom_spells_warrior.cpp     # Warrior (Arms/Fury/Prot)
│   ├── custom_spells_paladin.cpp     # Paladin (Holy/Prot/Ret)
│   ├── custom_spells_dk.cpp          # Death Knight (Blood/Frost/Unholy)
│   ├── custom_spells_shaman.cpp      # Shaman (Ele/Enhance/Resto)
│   ├── custom_spells_hunter.cpp      # Hunter (BM/MM/Surv)
│   ├── custom_spells_rogue.cpp       # Rogue (Assa/Combat/Sub)
│   └── custom_spells_druid.cpp       # Druid (Balance/Feral/Resto)
├── conf/
│   └── mod_custom_spells.conf.dist   # Config: CustomSpells.Enable
└── data/sql/db-world/
    └── mod_custom_spells.sql          # spell_script_names, spell_dbc, spell_proc
```

## Workflow: creating a custom spell (step by step)

### Overview: two paths for custom spells

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

### Path A: pure DBC-based spell (no C++)

Suitable for: Damage +X%, Cooldown -Xs, passive stat modifiers, SpellFamilyMask-based buffs.

**Step 1: reserve a spell ID**
- Pick the next free ID from the master plan (see ID block scheme below)
- Document the ID in CLAUDE.md

**Step 2: build the spell_dbc INSERT in SQL**

The `spell_dbc` table overrides the values from `Spell.dbc` at server start. New spell IDs are loaded entirely from the DB. Relevant columns for passive auras:

```sql
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`,
    `CastingTimeIndex`, `DurationIndex`, `RangeIndex`,
    `Effect_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`,
    `EffectAura_1`, `EffectMiscValue_1`,
    `EffectSpellClassMaskA_1`,
    `SpellFamilyName`, `SpellIconID`,
    `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900168,                     -- ID: unique spell ID
 0x10000040,                 -- Attributes: PASSIVE (0x40) + NOT_SHAPESHIFT (0x10000000)
 0x10000000,                 -- AttributesEx3: DEATH_PERSISTENT
 1,                          -- CastingTimeIndex: 1 = instant
 21,                         -- DurationIndex: 21 = permanent (-1)
 1,                          -- RangeIndex: 1 = self
 6,                          -- Effect_1: SPELL_EFFECT_APPLY_AURA
 50,                         -- EffectBasePoints_1: +50% (modifier value)
 1,                          -- ImplicitTargetA_1: TARGET_UNIT_CASTER
 108,                        -- EffectAura_1: SPELL_AURA_ADD_PCT_MODIFIER
 0,                          -- EffectMiscValue_1: SPELLMOD_DAMAGE (0)
 0x400,                      -- EffectSpellClassMaskA_1: target spell FamilyFlags[0]
 4,                          -- SpellFamilyName: 4 = warrior
 132,                        -- SpellIconID: any icon
 'Prot: Revenge Damage',     -- name (English)
 0x003F3F);                  -- Name_Lang_Mask: all locales use enUS
```

**Important DBC attributes:**

| Attribute | Hex value | Meaning |
|----------|----------|-----------|
| `SPELL_ATTR0_PASSIVE` | `0x40` | Spell is invisible, always active |
| `SPELL_ATTR0_NOT_SHAPESHIFT` | `0x10000000` | Stays in all stances |
| `SPELL_ATTR3_DEATH_PERSISTENT` | `0x10000000` | Survives death |

**Common EffectAura values:**

| Aura ID | Name | MiscValue | Use |
|---------|------|-----------|-----------|
| 108 | `ADD_PCT_MODIFIER` | 0=DAMAGE, 11=COOLDOWN, 14=CAST_TIME | percentage spell modifier |
| 107 | `ADD_FLAT_MODIFIER` | 0=DAMAGE, 11=COOLDOWN | flat spell modifier |
| 4 | `DUMMY` | — | marker aura (C++ checks via HasAura) |
| 42 | `PROC_TRIGGER_SPELL` | — | triggers another spell on proc |

**EffectSpellClassMask — identify the target spell:**

The mask must match the SpellFamilyFlags of the target spell. Three 32-bit fields:
- `EffectSpellClassMaskA_1` → SpellFamilyFlags[0]
- `EffectSpellClassMaskB_1` → SpellFamilyFlags[1]
- `EffectSpellClassMaskC_1` → SpellFamilyFlags[2]

> **IMPORTANT**: ALWAYS verify SpellFamilyFlags against your own Spell.dbc, not against online DBs!

**Step 3: insert the SQL into `mod_custom_spells.sql`**

```sql
-- Append at the end of the file
DELETE FROM `spell_dbc` WHERE `ID` = 900168;
INSERT INTO `spell_dbc` (...) VALUES (...);
```

**Step 4: patch the client DBC** (see section below)

**Step 5: build and test the server** (see section below)

---

### Path B: spell with C++ SpellScript

Suitable for: conditional procs, multi-spell triggers, AoE conversion, block procs, custom formulas.

**Step 1: create the DBC entry (like path A)**

C++ spells also need a DBC entry. For marker auras (HasAura check), a DUMMY aura is enough:

```sql
-- Marker aura: no own effect, only for HasAura() check
(900169, 0x10000040, 0, 0, 0x10000000, 1, 21, 1,
 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Revenge AoE', 0x003F3F),
```

For proc auras (triggered through the spell_proc system):
```sql
-- Proc aura: DUMMY effect, proc behavior via spell_proc + C++
(900172, 0x10000040, 0, 0, 0x10000000, 1, 21, 1,
 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block AoE', 0x003F3F),
```

For helper spells (triggered damage/heal spells):
```sql
-- Helper: instant AoE physical damage, TARGET_UNIT_SRC_AREA_ENEMY (22)
(900174, 0x10000000, 0, 0, 0, 1, 0, 1,
 2, 100, 500, 22, 0, 0, 0, 0, 4, 132, 'Block Shield Burst', 0x003F3F),
--       Effect=SCHOOL_DAMAGE(2), DieSides=100(random), BasePoints=500
```

**Step 2: add an enum constant** in `src/custom_spells.cpp`:

```cpp
enum CustomSpellIds
{
    // ... existing IDs ...
    SPELL_PROT_REVENGE_AOE_PASSIVE = 900169,
};
```

**Step 3: create the SpellScript/AuraScript class**

There are 4 main patterns:

**Pattern A: SpellScript with AfterHit (hook on an existing spell)**
```cpp
// Hooked onto Revenge (57823) via spell_script_names
class spell_custom_prot_revenge_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_prot_revenge_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target) return;

        Player* player = caster->ToPlayer();
        if (!player) return;

        // Check the marker aura
        if (!player->HasAura(SPELL_PROT_REVENGE_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Custom logic here
        int32 damage = GetHitDamage();
        caster->CastSpell(target, HELPER_SPELL_ID, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_prot_revenge_aoe::HandleAfterHit);
    }
};
```

**Pattern B: AuraScript with Proc (passive proc aura)**
```cpp
class spell_custom_prot_block_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_prot_block_aoe);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player) return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Check HitMask (block, dodge, parry, crit, etc.)
        if (!(eventInfo.GetHitMask() & PROC_HIT_BLOCK))
            return;

        player->CastSpell(player, HELPER_SPELL_ID, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_prot_block_aoe::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};
```

**Pattern C: AuraScript with CheckProc (filtered procs)**
```cpp
class spell_custom_speedy_bloodthirst : public AuraScript
{
    PrepareAuraScript(spell_custom_speedy_bloodthirst);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo) return false;
        // Only proc on Whirlwind (1680)
        return spellInfo->Id == 1680;
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        if (!player) return;
        player->RemoveSpellCooldown(SPELL_BLOODTHIRST, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_speedy_bloodthirst::CheckProc);
        OnProc += AuraProcFn(spell_custom_speedy_bloodthirst::HandleProc);
    }
};
```

**Pattern D: SpellScript with OnEffectHitTarget (damage override)**
```cpp
class spell_custom_paragon_strike : public SpellScript
{
    PrepareSpellScript(spell_custom_paragon_strike);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        // Custom damage formula
        int32 totalDmg = CalculateCustomDamage();
        SetHitDamage(totalDmg);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(
            spell_custom_paragon_strike::HandleDamage,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
```

**Step 4: register the script** in `AddCustomSpellsScripts()`:

```cpp
void AddCustomSpellsScripts()
{
    // ... existing ones ...
    RegisterSpellScript(spell_custom_prot_revenge_aoe);
};
```

**Step 5: spell_script_names SQL** — links the spell ID with the C++ class:

```sql
-- For your own custom spell IDs:
(900172, 'spell_custom_prot_block_aoe'),
-- For hooks on existing Blizzard spells:
(57823, 'spell_custom_prot_revenge_aoe'),  -- hooked onto Revenge
(47502, 'spell_custom_prot_tc_rend_sunder'), -- hooked onto Thunderclap
```

> **Important**: hooking onto existing spells (57823, 47502, 1680) → the C++ class runs on EVERY cast of that spell. Always check `HasAura()` so the effect is only active when the player has the passive!

**Step 6: spell_proc SQL** (only for proc-based auras):

```sql
DELETE FROM `spell_proc` WHERE `SpellId` = 900172;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`,
    `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
    `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`,
    `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`,
    `Chance`, `Cooldown`, `Charges`) VALUES
(900172,    -- SpellId
 0,         -- SchoolMask (0 = all)
 0,         -- SpellFamilyName (0 = all, 4 = warrior, etc.)
 0, 0, 0,   -- SpellFamilyMask0/1/2 (0 = all spells, C++ filters)
 0x2,       -- ProcFlags: PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK
 0,         -- SpellTypeMask (0 = all)
 0,         -- SpellPhaseMask (0=all, 2=HIT, 4=FINISH)
 0,         -- HitMask (0 = all, C++ checks PROC_HIT_BLOCK)
 0, 0,      -- AttributesMask, DisableEffectsMask
 0,         -- ProcsPerMinute (0 = use Chance instead of PPM)
 100,       -- Chance: 100% (C++ filters additionally)
 1000,      -- Cooldown: 1000ms (ICD)
 0);        -- Charges: 0 = unlimited
```

**Common ProcFlags:** the full corrected table is in [`PROCFLAGS_REFERENCE.md`](./PROCFLAGS_REFERENCE.md) and in [`share-public/docs/custom-spells/03-procs-and-flags.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/03-procs-and-flags.md).

| Flag | Hex | Meaning |
|------|-----|-----------|
| `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` | `0x4` | Own melee auto-attack |
| `PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK` | `0x8` | Enemy melee auto-attack |
| `PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS` | `0x10` | Own melee spell (Bloodthirst etc.) |
| `PROC_FLAG_DONE_PERIODIC` | `0x40000` | Own DoT tick |
| `PROC_FLAG_KILL` | `0x2` | Killed an enemy |
| `PROC_FLAG_TAKEN_DAMAGE` | `0x100000` | Damage taken |
| Combination: melee+spell | `0x14` | auto-attack OR melee spell |

**Common PROC_HIT masks (for C++ eventInfo.GetHitMask()):**

| Mask | Meaning |
|------|-----------|
| `PROC_HIT_BLOCK` | Attack was blocked |
| `PROC_HIT_DODGE` | Attack was dodged |
| `PROC_HIT_PARRY` | Attack was parried |
| `PROC_HIT_CRITICAL` | Critical hit |

---

### Checklist: new custom spell

```
□ Spell ID reserved and documented in CLAUDE.md
□ spell_dbc INSERT in mod_custom_spells.sql
□ (if C++) enum constant in custom_spells.cpp
□ (if C++) SpellScript/AuraScript class implemented
□ (if C++) RegisterSpellScript() in AddCustomSpellsScripts()
□ (if C++) spell_script_names INSERT in mod_custom_spells.sql
□ (if proc) spell_proc INSERT in mod_custom_spells.sql
□ (if helper) helper spell DBC + optional C++ script
□ Client Spell.dbc patched (for tooltips)
□ Build successful (0 errors)
□ Tested in-game
□ CLAUDE.md status updated from "implemented" → "tested"
□ share-public/docs/custom-spells/specs/<spec>.md status updated
```

---

### spell_dbc column reference (most important fields)

The `spell_dbc` table has 257 columns. Here are the most relevant for custom spells:

| Column | Type | Description |
|--------|-----|-------------|
| `ID` | uint | Unique spell ID |
| `Attributes` | uint | Flags: 0x40=PASSIVE, 0x10000000=NOT_SHAPESHIFT |
| `AttributesEx` | uint | Extended flags 1 |
| `AttributesEx2` | uint | Extended flags 2 |
| `AttributesEx3` | uint | Extended flags 3 (0x10000000=DEATH_PERSISTENT) |
| `CastingTimeIndex` | uint | 1=instant, other values → SpellCastTimes.dbc |
| `DurationIndex` | uint | 0=instant, 21=permanent, other → SpellDuration.dbc |
| `RangeIndex` | uint | 1=self, 4=30yd, 6=100yd → SpellRange.dbc |
| `ProcTypeMask` | uint | Proc trigger flags (overridden by spell_proc) |
| `ProcChance` | uint | Proc chance (overridden by spell_proc) |
| `Effect_1/2/3` | uint | 2=SCHOOL_DAMAGE, 6=APPLY_AURA, 3=DUMMY |
| `EffectDieSides_1/2/3` | int | Random range for damage (0=no random) |
| `EffectBasePoints_1/2/3` | int | Base value (damage, modifier %, etc.) |
| `ImplicitTargetA_1/2/3` | uint | 1=SELF, 6=ENEMY, 22=SRC_AREA_ENEMY |
| `EffectRadiusIndex_1/2/3` | uint | 8=5yd, 13=8yd, 14=10yd, 28=30yd |
| `EffectAura_1/2/3` | uint | Aura type: 4=DUMMY, 108=ADD_PCT_MODIFIER |
| `EffectMiscValue_1/2/3` | int | Aura specific: 0=SPELLMOD_DAMAGE, 11=SPELLMOD_COOLDOWN |
| `EffectTriggerSpell_1/2/3` | uint | Spell ID triggered on proc |
| `EffectSpellClassMaskA/B/C_1` | uint | SpellFamilyFlags[0/1/2] of the target spell |
| `SpellFamilyName` | uint | 0=Generic, 4=Warrior, 10=Paladin, 15=DK |
| `SpellClassMask_1/2/3` | uint | SpellFamilyFlags of this spell (for proc matching) |
| `SpellIconID` | uint | Icon ID from SpellIcon.dbc |
| `MaxTargets` | uint | Max targets (0=unlimited) |
| `SchoolMask` | uint | 1=Physical, 2=Holy, 4=Fire, 16=Shadow, 32=Arcane |
| `Name_Lang_enUS` | string | Spell name (English) |
| `Name_Lang_Mask` | uint | 0x003F3F = all locales use enUS |

### SpellFamilyName values

| Value | Class |
|------|--------|
| 0 | Generic |
| 3 | Mage |
| 4 | Warrior |
| 5 | Warlock |
| 6 | Priest |
| 7 | Druid |
| 8 | Rogue |
| 9 | Hunter |
| 10 | Paladin |
| 11 | Shaman |
| 15 | Death Knight |

## DBC Status

> **Curated per-spec view** with status, source links and implementation notes: [`share-public/docs/custom-spells/specs/`](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells/specs). This section stays here as a quick overview in the mod repo.

Spell IDs 900100-900109 (Warrior Arms) exist in `Spell.dbc` and are fully implemented.
Spell IDs 900108-900121 (Warrior Fury) exist in `Spell.dbc` (manually created, pure DBC, no C++). The old IDs 900138-900145 were removed.
Spell IDs 900168-900175 (Warrior Prot + helpers) exist in the `spell_dbc` table and are implemented.
Spell IDs 900200-900210 (Paladin Holy + helpers) exist in the `spell_dbc` table and are implemented.
Spell IDs 900234-900241 (Paladin Prot) exist in the `spell_dbc` table and are implemented.
Spell IDs 900268-900275 (Paladin Ret + helpers) exist in the `spell_dbc` table and are implemented.
Spell IDs 900300-900304 (DK Blood) exist in the `spell_dbc` table and are implemented.
Spell IDs 900333, 900368 (DK Frost + Frost Breath helper) exist in the `spell_dbc` table and are implemented.
NPC 900333 (Frost Wyrm) exists in `creature_template` with AI script `npc_custom_frost_wyrm`.
Spell IDs 900366-900367 (DK Unholy + helper) exist in the `spell_dbc` table and are implemented.

Spell IDs 900400-900408 (Shaman Ele + helpers) exist in the `spell_dbc` table and are implemented.

Spell IDs 900433-900440 (Shaman Enhance + helpers) exist in the `spell_dbc` table and are implemented.
NPC 900436 (Spirit Wolf) exists in `creature_template` for the wolf summon proc.
Spell IDs 900466-900467 (Shaman Resto) exist in the `spell_dbc` table and are implemented.

Spell IDs 900500-900567 (Hunter Shared + BM + MM + Surv + helpers) exist in the `spell_dbc` table and are implemented.

Spell IDs 901000-901073 (Druid Balance + Feral + Resto + helpers) exist in the `spell_dbc` table and are implemented.
NPC 901066 (Healing Treant) exists in `creature_template` for the HoT treant proc.

Spell IDs 900600-900669 (Rogue Assa + Combat + Sub + helpers) exist in the `spell_dbc` table and are implemented.

Spell IDs 900700-900774 (Mage Arcane + Fire + Frost + helpers) exist in the `spell_dbc` table and are implemented.
Spell IDs 900800-900872 (Warlock Affli + Demo + Destro + helpers) exist in the `spell_dbc` table and are implemented.
Spell IDs 900900-900968 (Priest Disc + Holy + Shadow + helpers) exist in the `spell_dbc` table and are implemented.
Spell IDs 901100-901108 (Global Spells + helpers) exist in the `spell_dbc` table and are implemented.

All class blocks are populated. The next free IDs within the existing blocks are tracked in CustomSpells.md (current allocation).

---

### Decision aid: DBC vs. C++

#### Pure DBC (no C++ needed)

| Effect type | DBC aura | DBC MiscValue | Example |
|------------|----------|---------------|----------|
| Damage +X% | `ADD_PCT_MODIFIER` (108) | `SPELLMOD_DAMAGE` (0) | MS +50%, BT +50%, TC +50%, Revenge +50% |
| Cooldown -Xs | `ADD_FLAT_MODIFIER` (107) | `SPELLMOD_COOLDOWN` (11) | MS cd -2s (BasePoints=-2000) |
| Cast time -X% | `ADD_PCT_MODIFIER` (108) | `SPELLMOD_CASTING_TIME` (14) | AB cast -50% |
| Unlimited targets | direct DBC change | `MaxAffectedTargets=0` | WW, Cleave |
| Remove stance req | direct DBC change | `Stances=0` | WW stance removal |

#### C++ required

| Mechanic | Reason for C++ | Examples |
|----------|---------------|-----------|
| Conditional procs | Complex conditions (crit, target count, CD) | 20% crit → Execute, WW 1-target autocast |
| Multi-spell triggers | One proc fires several spells | TC → Rend + 5× Sunder |
| Single→AoE conversion | Custom target selection | Revenge unlimited, Overpower +9 targets |
| Block/dodge/parry procs | HitMask filtering | AoE on block, 10% block → TC |
| Custom damage formulas | non-standard calculation | Paragon Strike (666 + 0.66×AP + PL%) |
| CD manipulation | runtime CD changes | Melee → Bladestorm CD -0.5s |

#### Common pitfalls

1. **SpellFamilyFlags wrong**: ALWAYS verify against your own Spell.dbc, never against online DBs
2. **MaxAffectedTargets=0 globally**: affects ALL players, not only those with the passive → for conditional targets use C++
3. **Proc loop**: helper spells can re-proc → set ICD in spell_proc
4. **spell_script_names missing**: the C++ class is not loaded → spell has no effect
5. **DurationIndex forgotten**: passive aura needs DurationIndex=21 (permanent)
6. **Attributes missing PASSIVE**: without 0x40 the spell is castable instead of permanently active

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
