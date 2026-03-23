/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "Config.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "GridNotifiers.h"
#include "CellImpl.h"
#include "Group.h"
#include "GroupReference.h"
#include "ScriptedCreature.h"
#include "CreatureScript.h"
#include "PetDefines.h"
#include "PlayerScript.h"
#include "GameTime.h"
#include "UnitScript.h"

/*
 * ==========================================================================
 *  mod-custom-spells - Custom Spell Effects Module
 * ==========================================================================
 *
 *  This module uses SpellScripts to override spell effect behavior.
 *  Each custom spell gets its own SpellScript class that hooks into
 *  the spell's DBC effects (e.g. School Damage) and replaces the
 *  damage calculation with custom logic.
 *
 *  HOW TO ADD A NEW CUSTOM SPELL:
 *  1. Define the spell ID as a constant in the enum below.
 *  2. Create a new SpellScript class (see spell_custom_paragon_strike).
 *  3. Register it in AddCustomSpellsScripts() at the bottom.
 */

enum CustomSpellIds
{
    // Custom damage spell: Base 666 + 66% AP + 1% per Paragon level
    SPELL_CUSTOM_PARAGON_STRIKE         = 900106,
    // Each cast reduces Bladestorm (46927) cooldown by 0.5s
    SPELL_CUSTOM_BLADESTORM_CD_REDUCE   = 900107,
    // ---- Warrior Fury (900134-900167) ----
    // Bloody Whirlwind buff: +50% Whirlwind damage per stack, consumed on WW cast
    SPELL_BLOODY_WHIRLWIND_BUFF         = 900139,
    // Bloody Whirlwind passive: procs 900139 on Bloodthirst hit
    SPELL_BLOODY_WHIRLWIND_PASSIVE      = 900140,
    // Speedy Bloodthirst: Whirlwind resets Bloodthirst cooldown
    SPELL_CUSTOM_SPEEDY_BLOODTHIRST     = 900141,
    // WW single-target: passive auras (markers checked via HasAura)
    SPELL_WW_OVERPOWER_PASSIVE          = 900142,
    SPELL_WW_SLAM_PASSIVE               = 900143,
    // WW single-target: boosted damage spells (autocast)
    SPELL_WW_OVERPOWER_BOOSTED          = 900144,
    SPELL_WW_SLAM_BOOSTED               = 900145,

    // ---- Warrior Prot (900168-900199) ----
    SPELL_PROT_REVENGE_DMG_PASSIVE      = 900168, // Revenge +50% damage (DBC-only)
    SPELL_PROT_REVENGE_AOE_PASSIVE      = 900169, // Revenge unlimited targets (C++)
    SPELL_PROT_TC_REND_SUNDER_PASSIVE   = 900170, // TC → Rend + 5× Sunder (C++)
    SPELL_PROT_TC_DMG_PASSIVE           = 900171, // TC +50% damage (DBC-only)
    SPELL_PROT_BLOCK_AOE_PASSIVE        = 900172, // AoE damage on Block (C++)
    SPELL_PROT_BLOCK_TC_PASSIVE         = 900173, // 10% Block → Enhanced TC (C++)
    SPELL_PROT_BLOCK_AOE_DAMAGE         = 900174, // Helper: AoE damage (triggered)
    SPELL_PROT_ENHANCED_THUNDERCLAP     = 900175, // Helper: Enhanced TC (triggered)

    // ---- Paladin Holy (900200-900233) ----
    SPELL_HOLY_HS_AOE_DMG_PASSIVE       = 900200, // Holy Shock AoE damage (C++)
    SPELL_HOLY_HS_AOE_HEAL_PASSIVE      = 900201, // Holy Shock AoE heal (C++)
    SPELL_HOLY_HS_BOTH_PASSIVE          = 900202, // Holy Shock always both (C++)
    SPELL_HOLY_HS_DMG_PASSIVE           = 900203, // Holy Shock +50% (DBC-only)
    SPELL_HOLY_CONSEC_HEAL_PASSIVE      = 900204, // Consecration also heals (C++)
    SPELL_HOLY_CONSEC_AROUND_PASSIVE    = 900205, // Consecration around you (DBC)
    SPELL_HOLY_CONSEC_DMG_PASSIVE       = 900206, // Consecration +50% (DBC-only)
    SPELL_HOLY_CONSEC_DUR_PASSIVE       = 900207, // Consecration +5sec (DBC-only)
    // Helper spells
    SPELL_HOLY_HS_AOE_DMG_HELPER        = 900208, // AoE Holy damage around target
    SPELL_HOLY_HS_AOE_HEAL_HELPER       = 900209, // AoE Holy heal around target
    SPELL_HOLY_CONSEC_HEAL_HELPER       = 900210, // Consecration heal tick helper

    // ---- Paladin Prot (900234-900267) ----
    SPELL_PPROT_CONSEC_AROUND_PASSIVE   = 900234, // Consecration around you (DBC marker)
    SPELL_PPROT_AS_TARGETS_PASSIVE      = 900235, // Avenger's Shield +9 targets (DBC)
    SPELL_PPROT_AS_DMG_PASSIVE          = 900236, // Avenger's Shield +50% (DBC)
    SPELL_PPROT_HS_CHARGES_PASSIVE      = 900237, // Holy Shield charges +99 (DBC)
    SPELL_PPROT_HS_DMG_PASSIVE          = 900238, // Holy Shield +50% (DBC)
    SPELL_PPROT_AS_CONSEC_PASSIVE       = 900239, // AS leaves Consecration (C++)
    SPELL_PPROT_JUDGE_AS_PASSIVE        = 900240, // Judgement → free AS (C++)
    SPELL_PPROT_JUDGE_CD_PASSIVE        = 900241, // Judgement cd -2sec (DBC)

    // ---- Paladin Ret (900268-900299) ----
    SPELL_RET_CONSEC_AROUND_PASSIVE     = 900268, // Consecration around you (DBC marker)
    SPELL_RET_JUDGE_CD_PASSIVE          = 900269, // Judgement cd -2sec (DBC)
    SPELL_RET_DS_TARGETS_PASSIVE        = 900270, // Divine Storm +6 targets (DBC marker)
    SPELL_RET_DS_DMG_PASSIVE            = 900271, // Divine Storm +50% (DBC)
    SPELL_RET_CS_DMG_PASSIVE            = 900272, // Crusader Strike +50% (DBC)
    SPELL_RET_CS_AOE_PASSIVE            = 900273, // Crusader Strike +9 targets (C++)
    SPELL_RET_EXORCISM_PROC_PASSIVE     = 900274, // CS/Judge/DS → Exorcism buff (C++)
    SPELL_RET_EXORCISM_BUFF             = 900275, // Exorcism +50% per stack (DBC buff)

    // ---- DK Blood (900300-900332) ----
    SPELL_DKB_3_RUNE_WEAPONS_PASSIVE    = 900300, // 3 Rune Weapons (C++)
    SPELL_DKB_DOUBLE_CAST_PASSIVE       = 900301, // Rune Weapon double casts (C++)
    SPELL_DKB_HS_DMG_PASSIVE            = 900302, // Heart Strike +50% (DBC)
    SPELL_DKB_HS_AOE_PASSIVE            = 900303, // Heart Strike +9 targets (C++)
    SPELL_DKB_DEATHCOIL_PROC_PASSIVE    = 900304, // Dealing dmg → Death Coil (C++)

    // ---- DK Frost (900333-900365) ----
    SPELL_DKF_FROST_WYRM_PASSIVE        = 900333, // Replace Ghoul with Frost Wyrm (C++)

    // ---- DK Unholy (900366-900399) ----
    SPELL_DKU_DOT_AOE_PASSIVE           = 900366, // DoTs → Shadow AoE proc (C++)
    SPELL_DKU_SHADOW_AOE_HELPER         = 900367, // Shadow AoE damage helper

    // ---- Shaman Ele (900400-900432) ----
    SPELL_ELE_CL_AOE_PASSIVE            = 900400, // Chain Lightning +6 targets + no reduction (C++)
    SPELL_ELE_TOTEM_FOLLOW_PASSIVE      = 900401, // Totems → following creatures (C++)
    SPELL_ELE_RAGNAROS_PASSIVE          = 900402, // Fire Ele → Ragnaros (C++)
    SPELL_ELE_OVERLOAD_LVB_PASSIVE      = 900403, // Lightning Overload + Lava Burst (C++)
    SPELL_ELE_LVB_SPREAD_FS_PASSIVE     = 900404, // Lava Burst spreads Flame Shock (C++)
    SPELL_ELE_FS_RESET_LVB_PASSIVE      = 900405, // Flame Shock ticks → reset LvB CD (C++)
    SPELL_ELE_LVB_TWO_CHARGES_PASSIVE   = 900406, // Lava Burst two charges (C++)
    SPELL_ELE_CC_INSTANT_LVB_PASSIVE    = 900407, // Clearcasting → instant Lava Burst (DBC)
    SPELL_ELE_CL_AOE_HELPER             = 900408, // Chain Lightning AoE damage helper

    // ---- Shaman Enhance (900433-900465) ----
    SPELL_ENH_TOTEM_FOLLOW_PASSIVE      = 900433, // Totems follow (reuse PlayerScript)
    SPELL_ENH_MAELSTROM_AOE_PASSIVE     = 900434, // 5 Maelstrom → summons AoE 5s (C++)
    SPELL_ENH_MAELSTROM_AOE_BUFF        = 900439, // Buff: summons deal AoE on attack 5s
    SPELL_ENH_SUMMON_DMG_PASSIVE        = 900435, // Summons +50% damage (DBC)
    SPELL_ENH_WOLF_SUMMON_PASSIVE       = 900436, // Auto attack → summon wolf (C++)
    SPELL_ENH_WOLF_HASTE_PASSIVE        = 900437, // Spirit Wolves inherit haste (C++)
    SPELL_ENH_WOLF_CL_PASSIVE           = 900438, // Spirit Wolves 5% CL on hit (C++)
    SPELL_ENH_WOLF_AOE_HELPER           = 900440, // Wolf AoE damage helper

    // ---- Shaman Resto (900466-900499) ----
    SPELL_RST_TOTEM_FOLLOW_PASSIVE      = 900466, // Totems follow (reuse PlayerScript)
    SPELL_RST_MANA_REGEN_PASSIVE        = 900467, // Mana regen +2% per missing mana% (C++)
};

// ---- DK constants ----
constexpr uint32 SPELL_DANCING_RUNE_WEAPON  = 49028;
constexpr uint32 SPELL_HEART_STRIKE_R4      = 55050; // Highest rank
constexpr uint32 SPELL_DK_DEATH_COIL_DMG    = 47632; // Death Coil damage effect
constexpr uint32 SPELL_DK_RAISE_DEAD        = 46584;
constexpr uint32 NPC_DK_RUNE_WEAPON         = 27893;
constexpr uint32 NPC_DK_GHOUL               = 26125;
constexpr uint32 NPC_DK_FROST_WYRM          = 900333; // Custom Frost Wyrm creature
constexpr uint32 SPELL_FROST_BREATH         = 900368; // Frost Wyrm cone breath

// ---- Shaman constants ----
constexpr uint32 SPELL_CHAIN_LIGHTNING_R4   = 49271;  // Highest rank CL
constexpr uint32 SPELL_LAVA_BURST_R2        = 51505;  // Highest rank LvB
constexpr uint32 SPELL_FLAME_SHOCK_R9       = 49233;  // Highest rank Flame Shock
constexpr uint32 SPELL_ELEMENTAL_FOCUS      = 16164;  // Clearcasting buff
constexpr uint32 SPELL_OVERLOAD_LB          = 45284;  // Lightning Overload (LB)
constexpr uint32 SPELL_OVERLOAD_CL          = 45297;  // Lightning Overload (CL)
constexpr uint32 SPELL_FIRE_ELEMENTAL_TOTEM = 2894;
// NPC_FIRE_ELEMENTAL = 15438 already defined in PetDefines.h
constexpr uint32 NPC_RAGNAROS_CUSTOM        = 900402; // Custom Ragnaros NPC
constexpr uint32 SPELL_MAELSTROM_WEAPON     = 53817;  // Maelstrom Weapon buff
constexpr uint32 SPELL_FERAL_SPIRIT         = 51533;  // Summon Spirit Wolves
constexpr uint32 NPC_SPIRIT_WOLF            = 29264;  // Spirit Wolf NPC
constexpr uint32 NPC_CUSTOM_WOLF            = 900436; // Custom summoned wolf
constexpr uint32 SPELL_CL_R6               = 49271;  // Chain Lightning rank 6
// Flame Shock SpellFamilyFlags[0] = 0x10000000, SpellFamilyName = 11

// ---- Bloodthirst SpellFamilyFlags ----
// Bloodthirst (23881): SpellFamilyName=4, SpellFamilyFlags[1]=0x00000400 (bit 42)
constexpr uint32 SPELLFAMILY_WARRIOR_ID        = 4;
constexpr uint32 BLOODTHIRST_FAMILY_FLAG1      = 0x00000400;

// ---- Paragon Strike constants ----
constexpr uint32 AURA_PARAGON_LEVEL   = 100000;
constexpr int32  CUSTOM_BASE_DAMAGE   = 666;
constexpr float  CUSTOM_AP_COEFF      = 0.66f;   // 66% of Attack Power
constexpr float  CUSTOM_PARAGON_BONUS = 0.01f;    // +1% damage per Paragon level

// ---- Bladestorm CD Reduction constants ----
constexpr uint32 SPELL_BLADESTORM     = 46924;
constexpr int32  BLADESTORM_CD_REDUCE_MS = -500;  // -0.5 seconds (in ms)

// ---- Speedy Bloodthirst constants ----
constexpr uint32 SPELL_BLOODTHIRST    = 23881;

// ---- Warrior Prot constants ----
constexpr uint32 SPELL_REND_R10       = 47465;
constexpr uint32 SPELL_SUNDER_ARMOR   = 58567;

// ---- Paladin Holy constants ----
// Holy Shock base spell (dummy → routes to dmg/heal)
constexpr uint32 SPELL_HOLY_SHOCK           = 20473;
// Holy Shock damage spell (highest rank: 48824, R1: 25912)
constexpr uint32 SPELL_HOLY_SHOCK_DMG_R7    = 48824;
// Holy Shock heal spell (highest rank: 48825, R1: 25914)
constexpr uint32 SPELL_HOLY_SHOCK_HEAL_R7   = 48825;
// Consecration highest rank
constexpr uint32 SPELL_CONSECRATION_R8      = 48819;
// Avenger's Shield highest rank
constexpr uint32 SPELL_AVENGERS_SHIELD_R3   = 48827;
// Judgement damage spell (always triggered by all Judgement types)
constexpr uint32 SPELL_JUDGEMENT_DAMAGE     = 54158;
// Divine Storm
constexpr uint32 SPELL_DIVINE_STORM        = 53385;
// Crusader Strike (highest rank)
constexpr uint32 SPELL_CRUSADER_STRIKE_R6   = 35395;
// Exorcism (highest rank)
constexpr uint32 SPELL_EXORCISM_R9          = 48801;

// ============================================================
//  SPELL 900106: Paragon Strike (SpellScript)
//  Overrides the School Damage effect with custom calculation:
//  - Base damage: 666
//  - Bonus: 66% Attack Power
//  - +1% total damage per Paragon level (aura 100000 stacks)
// ============================================================
class spell_custom_paragon_strike : public SpellScript
{
    PrepareSpellScript(spell_custom_paragon_strike);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // 66% of melee attack power
        float ap = player->GetTotalAttackPowerValue(BASE_ATTACK);
        float bonusDmg = ap * CUSTOM_AP_COEFF;

        // Paragon level from aura stacks on ID 100000
        uint32 paragonLevel = player->GetAuraCount(AURA_PARAGON_LEVEL);
        float paragonMult = 1.0f + (paragonLevel * CUSTOM_PARAGON_BONUS);

        // Final damage = (base + AP bonus) * paragon multiplier
        int32 totalDmg = static_cast<int32>(
            (CUSTOM_BASE_DAMAGE + bonusDmg) * paragonMult);

        SetHitDamage(totalDmg);

        LOG_INFO("module", "mod-custom-spells: Player {} -> "
            "Paragon Strike {} dmg on {} "
            "(AP: {:.0f}, Paragon Lv: {}, Mult: {:.2f})",
            player->GetName(), totalDmg, target->GetName(),
            ap, paragonLevel, paragonMult);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(
            spell_custom_paragon_strike::HandleDamage,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// ============================================================
//  SPELL 900107: Bladestorm CD Reduction (AuraScript)
//  Passive proc aura: on melee damage dealt, reduces the
//  cooldown of Bladestorm (46927) by 0.5s.
//  DBC: EFFECT_0 = SPELL_EFFECT_APPLY_AURA / SPELL_AURA_DUMMY
//  Proc filtering via spell_proc table (ProcFlags=0x14).
// ============================================================
class spell_custom_bladestorm_cd_reduce : public AuraScript
{
    PrepareAuraScript(spell_custom_bladestorm_cd_reduce);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
        {
            LOG_INFO("module", "mod-custom-spells: 900107 CheckProc -> "
                "NO player, returning false");
            return false;
        }

        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        LOG_INFO("module", "mod-custom-spells: 900107 CheckProc -> "
            "procSpell={}, flags=0x{:X}, hasBladestormCD={}",
            spellInfo ? spellInfo->Id : 0,
            eventInfo.GetTypeMask(),
            player->HasSpellCooldown(SPELL_BLADESTORM) ? "yes" : "no");

        // Only proc when Bladestorm is actually on cooldown
        return player->HasSpellCooldown(SPELL_BLADESTORM);
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        if (!target)
            return;

        Player* player = target->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        LOG_INFO("module", "mod-custom-spells: 900107 HandleProc -> "
            "Player {} reducing Bladestorm CD by {}ms",
            player->GetName(), -BLADESTORM_CD_REDUCE_MS);

        player->ModifySpellCooldown(SPELL_BLADESTORM, BLADESTORM_CD_REDUCE_MS);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(
            spell_custom_bladestorm_cd_reduce::CheckProc);
        OnProc += AuraProcFn(
            spell_custom_bladestorm_cd_reduce::HandleProc);
    }
};

// ============================================================
//  SPELL 900140: Bloody Whirlwind Passive (AuraScript)
//  DBC has the proc aura on EFFECT_1 (not EFFECT_0).
//  Bloodthirst (23881) SpellFamilyFlags[1]=0x00000400 (bit 42).
//  This script explicitly filters procs to only fire on Bloodthirst.
// ============================================================
class spell_custom_bloody_whirlwind_passive : public AuraScript
{
    PrepareAuraScript(spell_custom_bloody_whirlwind_passive);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        // Only proc on Bloodthirst: Warrior family, flags[1] bit 10
        if (spellInfo->SpellFamilyName != SPELLFAMILY_WARRIOR_ID)
            return false;

        if (!(spellInfo->SpellFamilyFlags[1] & BLOODTHIRST_FAMILY_FLAG1))
            return false;

        return true;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_BLOODY_WHIRLWIND_BUFF, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_bloody_whirlwind_passive::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_bloody_whirlwind_passive::HandleProc,
            EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// ============================================================
//  SPELL 1680: Whirlwind - Consume Bloody Whirlwind stacks
//  + Single-target autocast (Overpower / Slam)
//
//  After Whirlwind finishes casting:
//  1. Remove all stacks of the Bloody Whirlwind buff (900139)
//  2. If WW hit exactly 1 target and player has passive 900142/900143,
//     autocast boosted Overpower (900144) / Slam (900145) on the target
// ============================================================
class spell_custom_bloody_whirlwind_consume : public SpellScript
{
    PrepareSpellScript(spell_custom_bloody_whirlwind_consume);

    uint32 _targetCount = 0;
    ObjectGuid _singleTargetGuid;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        _targetCount = targets.size();
        if (_targetCount == 1 && !targets.empty())
            _singleTargetGuid = targets.front()->GetGUID();
    }

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // --- Bloody Whirlwind: consume stacks ---
        if (caster->HasAura(SPELL_BLOODY_WHIRLWIND_BUFF))
        {
            uint32 stacks = caster->GetAuraCount(SPELL_BLOODY_WHIRLWIND_BUFF);
            caster->RemoveAurasDueToSpell(SPELL_BLOODY_WHIRLWIND_BUFF);

            if (Player* player = caster->ToPlayer())
                LOG_INFO("module", "mod-custom-spells: Player {} -> "
                    "Bloody Whirlwind consumed {} stacks on Whirlwind cast",
                    player->GetName(), stacks);
        }

        // --- Single-target autocast ---
        if (_targetCount != 1)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Unit* target = ObjectAccessor::GetUnit(*player, _singleTargetGuid);
        if (!target || !target->IsAlive())
            return;

        if (player->HasAura(SPELL_WW_OVERPOWER_PASSIVE))
        {
            player->CastSpell(target, SPELL_WW_OVERPOWER_BOOSTED, true);
            LOG_INFO("module", "mod-custom-spells: Player {} -> "
                "WW single-target: autocast Overpower on {}",
                player->GetName(), target->GetName());
        }

        if (player->HasAura(SPELL_WW_SLAM_PASSIVE))
        {
            player->CastSpell(target, SPELL_WW_SLAM_BOOSTED, true);
            LOG_INFO("module", "mod-custom-spells: Player {} -> "
                "WW single-target: autocast Slam on {}",
                player->GetName(), target->GetName());
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(
            spell_custom_bloody_whirlwind_consume::FilterTargets,
            EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        AfterCast += SpellCastFn(spell_custom_bloody_whirlwind_consume::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900141: Speedy Bloodthirst (AuraScript)
//  Passive proc aura: when Whirlwind (1680) is cast,
//  resets the cooldown of Bloodthirst (23881).
//  DBC: EFFECT_0 = SPELL_EFFECT_APPLY_AURA / SPELL_AURA_PROC_TRIGGER_SPELL
//  Proc filtering via spell_proc (ProcFlags=0x10, SpellPhaseMask=FINISH).
// ============================================================
class spell_custom_speedy_bloodthirst : public AuraScript
{
    PrepareAuraScript(spell_custom_speedy_bloodthirst);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
        {
            LOG_INFO("module", "mod-custom-spells: 900141 CheckProc -> "
                "NO spell info, returning false");
            return false;
        }

        LOG_INFO("module", "mod-custom-spells: 900141 CheckProc -> "
            "spell={}, typeMask=0x{:X}, spellFamily={}, "
            "flags[0]=0x{:08X} flags[1]=0x{:08X}",
            spellInfo->Id, eventInfo.GetTypeMask(),
            spellInfo->SpellFamilyName,
            spellInfo->SpellFamilyFlags[0], spellInfo->SpellFamilyFlags[1]);

        // Only proc on Whirlwind (1680)
        return spellInfo->Id == 1680;
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player)
        {
            LOG_INFO("module", "mod-custom-spells: 900141 HandleProc -> "
                "NO player, returning");
            return;
        }

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        LOG_INFO("module", "mod-custom-spells: 900141 HandleProc -> "
            "Player {} resetting Bloodthirst CD, hasBTCooldown={}",
            player->GetName(),
            player->HasSpellCooldown(SPELL_BLOODTHIRST) ? "yes" : "no");

        player->RemoveSpellCooldown(SPELL_BLOODTHIRST, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(
            spell_custom_speedy_bloodthirst::CheckProc);
        OnProc += AuraProcFn(
            spell_custom_speedy_bloodthirst::HandleProc);
    }
};

// ============================================================
//  SPELL 900144: WW Boosted Overpower (SpellScript)
//  Autocast by Whirlwind when only 1 target is hit.
//  Multiplies DBC base damage by Paragon bonus: +1% per level.
//  Uses OnHit instead of OnEffectHitTarget to avoid DBC effect
//  type validation (the DBC effect may not be SCHOOL_DAMAGE).
// ============================================================
class spell_custom_ww_overpower : public SpellScript
{
    PrepareSpellScript(spell_custom_ww_overpower);

    void HandleHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        uint32 paragonLevel = player->GetAuraCount(AURA_PARAGON_LEVEL);
        float paragonMult = 1.0f + (paragonLevel * CUSTOM_PARAGON_BONUS);
        int32 damage = static_cast<int32>(GetHitDamage() * paragonMult);
        SetHitDamage(damage);

        LOG_INFO("module", "mod-custom-spells: Player {} -> "
            "WW Overpower {} dmg on {} (Paragon Lv: {}, Mult: {:.2f})",
            player->GetName(), damage, target->GetName(),
            paragonLevel, paragonMult);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_custom_ww_overpower::HandleHit);
    }
};

// ============================================================
//  SPELL 900145: WW Boosted Slam (SpellScript)
//  Autocast by Whirlwind when only 1 target is hit.
//  Multiplies DBC base damage by Paragon bonus: +1% per level.
//  Uses OnHit instead of OnEffectHitTarget to avoid DBC effect
//  type validation (the DBC effect may not be SCHOOL_DAMAGE).
// ============================================================
class spell_custom_ww_slam : public SpellScript
{
    PrepareSpellScript(spell_custom_ww_slam);

    void HandleHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        uint32 paragonLevel = player->GetAuraCount(AURA_PARAGON_LEVEL);
        float paragonMult = 1.0f + (paragonLevel * CUSTOM_PARAGON_BONUS);
        int32 damage = static_cast<int32>(GetHitDamage() * paragonMult);
        SetHitDamage(damage);

        LOG_INFO("module", "mod-custom-spells: Player {} -> "
            "WW Slam {} dmg on {} (Paragon Lv: {}, Mult: {:.2f})",
            player->GetName(), damage, target->GetName(),
            paragonLevel, paragonMult);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_custom_ww_slam::HandleHit);
    }
};

// ============================================================
//  SPELL 900169: Revenge Unlimited Targets (SpellScript)
//  Hooked on Revenge (57823). After hitting the primary target,
//  deals the same damage to all enemies within 8yd of caster.
//  Only active when player has passive aura 900169.
// ============================================================
class spell_custom_prot_revenge_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_prot_revenge_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_PROT_REVENGE_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(caster, searcher, 8.0f);
        targets.remove(mainTarget);

        for (Unit* target : targets)
        {
            if (!target->IsAlive())
                continue;

            Unit::DealDamage(caster, target, static_cast<uint32>(damage),
                nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL);

            LOG_INFO("module",
                "mod-custom-spells: Player {} -> Revenge AoE {} dmg on {}",
                player->GetName(), damage, target->GetName());
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_prot_revenge_aoe::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900170: Thunderclap → Rend + 5× Sunder Armor
//  Hooked on Thunderclap (47502). After hitting each target,
//  applies Rend and 5 stacks of Sunder Armor.
//  Only active when player has passive aura 900170.
// ============================================================
class spell_custom_prot_tc_rend_sunder : public SpellScript
{
    PrepareSpellScript(spell_custom_prot_tc_rend_sunder);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_PROT_TC_REND_SUNDER_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        caster->CastSpell(target, SPELL_REND_R10, true);

        for (uint8 i = 0; i < 5; ++i)
            caster->CastSpell(target, SPELL_SUNDER_ARMOR, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> TC applied Rend + 5x Sunder on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_prot_tc_rend_sunder::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900172: AoE Damage on Block (AuraScript)
//  Passive proc aura: when the warrior blocks an attack,
//  deals AoE physical damage (900174) to nearby enemies.
// ============================================================
class spell_custom_prot_block_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_prot_block_aoe);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        if (!target)
            return;

        Player* player = target->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        if (!(eventInfo.GetHitMask() & PROC_HIT_BLOCK))
            return;

        player->CastSpell(player, SPELL_PROT_BLOCK_AOE_DAMAGE, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Block -> AoE damage triggered",
            player->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_prot_block_aoe::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900173: 10% Block → Enhanced Thunderclap (AuraScript)
//  Passive proc aura: when the warrior blocks an attack,
//  10% chance to cast Enhanced Thunderclap (900175).
// ============================================================
class spell_custom_prot_block_tc : public AuraScript
{
    PrepareAuraScript(spell_custom_prot_block_tc);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        if (!target)
            return;

        Player* player = target->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        if (!(eventInfo.GetHitMask() & PROC_HIT_BLOCK))
            return;

        player->CastSpell(player, SPELL_PROT_ENHANCED_THUNDERCLAP, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Block -> Enhanced TC triggered",
            player->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_prot_block_tc::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900200: Holy Shock AoE Damage (SpellScript)
//  Hooked on Holy Shock damage spells (48824 etc.).
//  After hitting a hostile target, casts AoE holy damage
//  helper (900208) centered on the target.
//  Only active when player has passive 900200.
// ============================================================
class spell_custom_holy_hs_aoe_dmg : public SpellScript
{
    PrepareSpellScript(spell_custom_holy_hs_aoe_dmg);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_HS_AOE_DMG_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        caster->CastSpell(target, SPELL_HOLY_HS_AOE_DMG_HELPER, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Holy Shock AoE damage on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_holy_hs_aoe_dmg::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900201: Holy Shock AoE Heal (SpellScript)
//  Hooked on Holy Shock heal spells (48825 etc.).
//  After healing a friendly target, casts AoE holy heal
//  helper (900209) centered on the target.
//  Only active when player has passive 900201.
// ============================================================
class spell_custom_holy_hs_aoe_heal : public SpellScript
{
    PrepareSpellScript(spell_custom_holy_hs_aoe_heal);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_HS_AOE_HEAL_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        caster->CastSpell(target, SPELL_HOLY_HS_AOE_HEAL_HELPER, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Holy Shock AoE heal on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_holy_hs_aoe_heal::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900202: Holy Shock Always Both (SpellScript)
//  Hooked on Holy Shock damage (48824) and heal (48825).
//  After hitting a hostile target → also heal nearest ally.
//  After healing a friendly target → also damage nearest enemy.
//  Only active when player has passive 900202.
// ============================================================
class spell_custom_holy_hs_both_dmg : public SpellScript
{
    PrepareSpellScript(spell_custom_holy_hs_both_dmg);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_HS_BOTH_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // We're on the damage spell → also heal nearest injured ally
        // Find nearest friendly unit within 40yd that's injured
        Unit* healTarget = nullptr;
        float minDist = 40.0f;

        Group* group = player->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || member->GetGUID() == player->GetGUID())
                    continue;
                if (member->GetHealthPct() >= 100.0f)
                    continue;
                float dist = player->GetDistance(member);
                if (dist < minDist)
                {
                    minDist = dist;
                    healTarget = member;
                }
            }
        }

        // Fallback: heal self if no group or no injured members
        if (!healTarget && player->GetHealthPct() < 100.0f)
            healTarget = player;

        if (healTarget)
        {
            caster->CastSpell(healTarget, SPELL_HOLY_SHOCK_HEAL_R7, true);
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> HS Both: auto-heal {}",
                player->GetName(), healTarget->GetName());
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_holy_hs_both_dmg::HandleAfterHit);
    }
};

class spell_custom_holy_hs_both_heal : public SpellScript
{
    PrepareSpellScript(spell_custom_holy_hs_both_heal);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_HS_BOTH_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // We're on the heal spell → also damage nearest enemy
        Unit* dmgTarget = player->GetVictim();
        if (!dmgTarget || !dmgTarget->IsAlive())
        {
            // Find nearest hostile within 40yd
            dmgTarget = player->SelectNearbyTarget(nullptr, 40.0f);
        }

        if (dmgTarget)
        {
            caster->CastSpell(dmgTarget, SPELL_HOLY_SHOCK_DMG_R7, true);
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> HS Both: auto-damage {}",
                player->GetName(), dmgTarget->GetName());
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_holy_hs_both_heal::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900204: Consecration Also Heals (SpellScript)
//  Hooked on Consecration (48819). On each periodic tick,
//  also casts a heal helper on friendly units in the area.
//  Only active when player has passive 900204.
//
//  Approach: AuraScript on Consecration's periodic aura.
//  OnEffectPeriodic → cast heal helper on caster's location.
// ============================================================
class spell_custom_holy_consec_heal : public AuraScript
{
    PrepareAuraScript(spell_custom_holy_consec_heal);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_CONSEC_HEAL_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Cast AoE heal centered on the Consecration area (caster position
        // at time of cast, but since Consecration is stationary we use
        // the aura owner which is the dynamic object → fallback to caster)
        caster->CastSpell(caster, SPELL_HOLY_CONSEC_HEAL_HELPER, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_holy_consec_heal::HandlePeriodic,
            EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

// ============================================================
//  SPELL 900239: Avenger's Shield Leaves Consecration
//  Hooked on Avenger's Shield (48827). After hitting each
//  target, casts Consecration at the target's position.
//  Only active when player has passive 900239.
// ============================================================
class spell_custom_pprot_as_consec : public SpellScript
{
    PrepareSpellScript(spell_custom_pprot_as_consec);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_PPROT_AS_CONSEC_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Cast Consecration (triggered) at target's location
        caster->CastSpell(target, SPELL_CONSECRATION_R8, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> AS left Consecration on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_pprot_as_consec::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900240: Judgement → Free Avenger's Shield
//  Hooked on Judgement Damage (54158). After Judgement hits,
//  auto-casts Avenger's Shield at the same target.
//  Only active when player has passive 900240.
// ============================================================
class spell_custom_pprot_judge_as : public SpellScript
{
    PrepareSpellScript(spell_custom_pprot_judge_as);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_PPROT_JUDGE_AS_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        caster->CastSpell(target, SPELL_AVENGERS_SHIELD_R3, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Judgement triggered free AS on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_pprot_judge_as::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900273: Crusader Strike +9 Targets (SpellScript)
//  Hooked on Crusader Strike (35395). After hitting primary
//  target, deals same damage to up to 9 additional enemies
//  within 8yd of caster.
//  Only active when player has passive 900273.
// ============================================================
class spell_custom_ret_cs_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_ret_cs_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_RET_CS_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(caster, searcher, 8.0f);
        targets.remove(mainTarget);

        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= 9)
                break;
            if (!target->IsAlive())
                continue;

            Unit::DealDamage(caster, target, static_cast<uint32>(damage),
                nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL);
            ++count;
        }

        if (count > 0)
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> CS AoE hit {} extra targets",
                player->GetName(), count);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ret_cs_aoe::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900274: Exorcism Buff System (AuraScript)
//  Passive proc aura: when CS, Judgement, or Divine Storm
//  hits an enemy, adds 1 stack of Exorcism buff (900275).
//  Max 10 stacks. Each stack = +50% Exorcism damage (DBC).
//  Stacks are consumed when Exorcism is cast.
// ============================================================
class spell_custom_ret_exorcism_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_ret_exorcism_proc);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        // Only proc on CS (35395), Judgement Damage (54158), or DS (53385)
        uint32 id = spellInfo->Id;
        return id == SPELL_CRUSADER_STRIKE_R6
            || id == SPELL_JUDGEMENT_DAMAGE
            || id == SPELL_DIVINE_STORM;
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        if (!target)
            return;

        Player* player = target->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Add one stack of the Exorcism buff (max 10)
        player->CastSpell(player, SPELL_RET_EXORCISM_BUFF, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Exorcism buff stack added (now {})",
            player->GetName(), player->GetAuraCount(SPELL_RET_EXORCISM_BUFF));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_ret_exorcism_proc::CheckProc);
        OnProc += AuraProcFn(spell_custom_ret_exorcism_proc::HandleProc);
    }
};

// ============================================================
//  Exorcism Consume: Hooked on Exorcism (48801).
//  After casting Exorcism, consumes all stacks of 900275.
//  Only active when player has the buff.
// ============================================================
class spell_custom_ret_exorcism_consume : public SpellScript
{
    PrepareSpellScript(spell_custom_ret_exorcism_consume);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(SPELL_RET_EXORCISM_BUFF))
        {
            uint32 stacks = caster->GetAuraCount(SPELL_RET_EXORCISM_BUFF);
            caster->RemoveAurasDueToSpell(SPELL_RET_EXORCISM_BUFF);

            if (Player* player = caster->ToPlayer())
                LOG_INFO("module",
                    "mod-custom-spells: Player {} -> Exorcism consumed {} stacks",
                    player->GetName(), stacks);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_ret_exorcism_consume::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900300: 3 Rune Weapons (SpellScript)
//  Hooked on Dancing Rune Weapon (49028). After cast,
//  summons 2 additional rune weapon creatures.
//  Only active when player has passive 900300.
// ============================================================
class spell_custom_dkb_3_rune_weapons : public SpellScript
{
    PrepareSpellScript(spell_custom_dkb_3_rune_weapons);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_DKB_3_RUNE_WEAPONS_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Summon 2 extra rune weapons by re-casting the summon effect (triggered)
        for (int i = 0; i < 2; ++i)
            player->CastSpell(player, SPELL_DANCING_RUNE_WEAPON, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> 3 Rune Weapons summoned",
            player->GetName());
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_dkb_3_rune_weapons::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900301: Rune Weapon Double Casts (AuraScript)
//  Hooked on Dancing Rune Weapon aura (49028). Overrides
//  the proc to cast each spell twice instead of once.
//  Only active when player has passive 900301.
// ============================================================
class spell_custom_dkb_double_cast : public AuraScript
{
    PrepareAuraScript(spell_custom_dkb_double_cast);

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        // Don't prevent default — let the original DRW proc fire.
        // We just add ONE extra cast (so total = 2).

        Unit* player = eventInfo.GetActor();
        Unit* target = eventInfo.GetActionTarget();
        if (!player || !target || !target->IsAlive())
            return;

        Player* pl = player->ToPlayer();
        if (!pl)
            return;

        if (!pl->HasAura(SPELL_DKB_DOUBLE_CAST_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Find the rune weapon
        Unit* dancingRuneWeapon = nullptr;
        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            if ((*itr)->GetEntry() == NPC_DK_RUNE_WEAPON)
            {
                dancingRuneWeapon = *itr;
                break;
            }
        }

        if (!dancingRuneWeapon)
            return;

        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (procSpell)
        {
            // Cast the spell one extra time from the rune weapon
            dancingRuneWeapon->CastSpell(target, procSpell->Id, true, nullptr, aurEff, dancingRuneWeapon->GetGUID());
        }
        else if (eventInfo.GetDamageInfo())
        {
            // Extra melee hit at half damage (same as base DRW behavior)
            CalcDamageInfo damageInfo;
            player->CalculateMeleeDamage(target, &damageInfo, eventInfo.GetDamageInfo()->GetAttackType());
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                Unit::DealDamageMods(target, damageInfo.damages[i].damage, &damageInfo.damages[i].absorb);
                damageInfo.damages[i].damage /= 2.0f;
            }
            damageInfo.attacker = dancingRuneWeapon;
            dancingRuneWeapon->SendAttackStateUpdate(&damageInfo);
            dancingRuneWeapon->DealMeleeDamage(&damageInfo, true);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_dkb_double_cast::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900303: Heart Strike +9 Targets (SpellScript)
//  Hooked on Heart Strike (55050). After hitting primary
//  target, deals same damage to up to 9 additional enemies.
//  Only active when player has passive 900303.
// ============================================================
class spell_custom_dkb_hs_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_dkb_hs_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_DKB_HS_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(caster, searcher, 8.0f);
        targets.remove(mainTarget);

        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= 9)
                break;
            if (!target->IsAlive())
                continue;

            Unit::DealDamage(caster, target, static_cast<uint32>(damage),
                nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL);
            ++count;
        }

        if (count > 0)
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> HS AoE hit {} extra targets",
                player->GetName(), count);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_dkb_hs_aoe::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900304: Dealing Damage → Death Coil Proc (AuraScript)
//  Passive proc aura: when dealing melee/spell damage,
//  X% chance to auto-cast Death Coil on target.
//  Only active when player has passive 900304.
// ============================================================
class spell_custom_dkb_deathcoil_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_dkb_deathcoil_proc);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Cast Death Coil damage effect (triggered, no cost)
        player->CastSpell(target, SPELL_DK_DEATH_COIL_DMG, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> proc Death Coil on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_dkb_deathcoil_proc::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900333: Replace Ghoul with Frost Wyrm (SpellScript)
//  Hooked on Raise Dead (46584). After cast, despawns the
//  ghoul and summons a custom Frost Wyrm (NPC 900333) instead.
//  The Frost Wyrm has 2× Gargoyle HP and casts Frost Breath.
// ============================================================
class spell_custom_dkf_frost_wyrm : public SpellScript
{
    PrepareSpellScript(spell_custom_dkf_frost_wyrm);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_DKF_FROST_WYRM_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Find and despawn the ghoul
        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            if ((*itr)->GetEntry() == NPC_DK_GHOUL && (*itr)->ToCreature())
            {
                (*itr)->ToCreature()->DespawnOrUnsummon();
                break;
            }
        }

        // Summon Frost Wyrm as guardian (follows + fights)
        if (Creature* wyrm = player->SummonCreature(NPC_DK_FROST_WYRM,
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
            player->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000))
        {
            wyrm->SetOwnerGUID(player->GetGUID());
            wyrm->SetCreatorGUID(player->GetGUID());
            wyrm->SetFaction(player->GetFaction());
            wyrm->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

            if (Unit* victim = player->GetVictim())
            {
                wyrm->Attack(victim, true);
                wyrm->GetMotionMaster()->MoveChase(victim);
            }

            LOG_INFO("module",
                "mod-custom-spells: Player {} -> Frost Wyrm (NPC {}) summoned",
                player->GetName(), NPC_DK_FROST_WYRM);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_dkf_frost_wyrm::HandleAfterCast);
    }
};

// ============================================================
//  NPC 900333: Frost Wyrm CreatureScript
//  AI: Follows owner, auto-casts Frost Breath (900368) on
//  victim. Frost Breath is a 2s cast, cone 20yd, deals
//  5000 base damage + AP scaling, slows targets.
// ============================================================
struct npc_custom_frost_wyrm : public ScriptedAI
{
    npc_custom_frost_wyrm(Creature* creature) : ScriptedAI(creature)
    {
        _breathTimer = 0;
        _selectionTimer = 0;
    }

    void InitializeAI() override
    {
        ScriptedAI::InitializeAI();
        me->SetReactState(REACT_AGGRESSIVE);

        Unit* owner = me->GetOwner();
        if (!owner)
            return;

        if (Unit* victim = owner->GetVictim())
        {
            me->Attack(victim, true);
            me->GetMotionMaster()->MoveChase(victim);
        }
        else
        {
            me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, 0.0f);
        }
    }

    void IsSummonedBy(WorldObject* summoner) override
    {
        if (!summoner || !summoner->ToUnit())
            return;

        Unit* owner = summoner->ToUnit();
        if (Unit* victim = owner->GetVictim())
        {
            me->Attack(victim, true);
            me->GetMotionMaster()->MoveChase(victim);
        }
    }

    void SelectTarget()
    {
        Unit* owner = me->GetOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        // Follow player's target selection
        Unit* selection = player->GetSelectedUnit();
        if (selection && selection != me->GetVictim() && me->IsValidAttackTarget(selection))
        {
            me->GetMotionMaster()->Clear(false);
            AttackStart(selection);
        }
        else if (!me->GetVictim() || !me->GetVictim()->IsAlive())
        {
            me->CombatStop(true);
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, 0.0f);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _selectionTimer += diff;
        if (_selectionTimer >= 1000)
        {
            SelectTarget();
            _selectionTimer = 0;
        }

        if (!UpdateVictim())
            return;

        _breathTimer += diff;

        // Cast Frost Breath every ~4s (2s cast + 2s gap)
        if (_breathTimer >= 4000)
        {
            if (!me->HasUnitState(UNIT_STATE_CASTING))
            {
                // Scale damage with owner's AP
                Unit* owner = me->GetOwner();
                if (owner)
                {
                    float ap = owner->GetTotalAttackPowerValue(BASE_ATTACK);
                    int32 damage = 5000 + static_cast<int32>(ap * 0.5f);
                    me->CastCustomSpell(me->GetVictim(), SPELL_FROST_BREATH,
                        &damage, nullptr, nullptr, false);
                }
                else
                {
                    DoCastVictim(SPELL_FROST_BREATH);
                }
                _breathTimer = 0;
            }
        }

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*who*/) override
    {
        if (me->IsSummon())
            me->ToTempSummon()->UnSummon();
    }

private:
    uint32 _breathTimer;
    uint32 _selectionTimer;
};

// ============================================================
//  SPELL 900368: Frost Breath damage handler (SpellScript)
//  Overrides damage with 5000 base + 50% owner AP scaling.
// ============================================================
class spell_custom_frost_breath : public SpellScript
{
    PrepareSpellScript(spell_custom_frost_breath);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* owner = caster->GetOwner();
        if (!owner)
            return;

        float ap = owner->GetTotalAttackPowerValue(BASE_ATTACK);
        int32 damage = 5000 + static_cast<int32>(ap * 0.5f);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_custom_frost_breath::HandleDamage,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// ============================================================
//  SPELL 900366: DoTs → Shadow AoE Proc (AuraScript)
//  Passive proc aura: when periodic damage ticks,
//  X% chance to cast Shadow AoE (900367) on target.
// ============================================================
class spell_custom_dku_dot_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_dku_dot_aoe);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Cast Shadow AoE helper at target
        player->CastSpell(target, SPELL_DKU_SHADOW_AOE_HELPER, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> DoT proc Shadow AoE on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_dku_dot_aoe::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900400: Chain Lightning +6 targets, no damage reduction
//  Hooked on Chain Lightning (all ranks via -49271).
//  After hitting primary target, casts AoE helper (900408) on
//  up to 6 additional enemies at full damage.
// ============================================================
class spell_custom_ele_cl_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_cl_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_CL_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        // Find 6 additional enemies and deal full CL damage
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 12.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(mainTarget, searcher, 12.0f);
        targets.remove(mainTarget);

        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= 6)
                break;
            if (!target->IsAlive() || !caster->IsValidAttackTarget(target))
                continue;

            caster->CastCustomSpell(target, SPELL_ELE_CL_AOE_HELPER,
                &damage, nullptr, nullptr, true);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ele_cl_aoe::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900401: Totems → Following Creatures (PlayerScript)
//  When player has 900401, totems follow the player instead
//  of being static. Checked every 2 seconds via OnUpdate.
// ============================================================
class custom_totem_follow_playerscript : public PlayerScript
{
public:
    custom_totem_follow_playerscript() : PlayerScript("custom_totem_follow_playerscript") {}

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        // Check for any of the three totem follow passives (Ele/Enh/Resto)
        if (!player->HasAura(SPELL_ELE_TOTEM_FOLLOW_PASSIVE) &&
            !player->HasAura(SPELL_ENH_TOTEM_FOLLOW_PASSIVE) &&
            !player->HasAura(SPELL_RST_TOTEM_FOLLOW_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Throttle: only check every ~2 seconds using game time
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        static std::unordered_map<ObjectGuid, uint32> s_lastCheck;
        ObjectGuid guid = player->GetGUID();
        if (s_lastCheck.count(guid) && (now - s_lastCheck[guid]) < 2)
            return;
        s_lastCheck[guid] = now;

        for (uint8 i = 0; i < MAX_TOTEM_SLOT; ++i)
        {
            if (!player->m_SummonSlot[i])
                continue;

            Creature* totem = player->GetMap()->GetCreature(player->m_SummonSlot[i]);
            if (!totem || !totem->IsAlive())
                continue;

            // Teleport totem near player if too far away
            float dist = totem->GetDistance(player);
            if (dist > 5.0f)
            {
                float x, y, z;
                player->GetClosePoint(x, y, z, totem->GetCombatReach(),
                    PET_FOLLOW_DIST + i * 1.5f, (M_PI / 2) * i);
                totem->NearTeleportTo(x, y, z, player->GetOrientation());
            }
        }
    }
};

// ============================================================
//  SPELL 900402: Fire Elemental → Ragnaros
//  Hooked on Fire Elemental Totem (2894). After cast,
//  replaces the Fire Elemental with a Ragnaros model.
// ============================================================
class spell_custom_ele_ragnaros : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_ragnaros);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_RAGNAROS_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Find the fire elemental and change its display to Ragnaros
        constexpr uint32 RAGNAROS_DISPLAY_ID = 11121;
        constexpr float  RAGNAROS_SCALE      = 0.35f;

        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            if ((*itr)->GetEntry() == NPC_FIRE_ELEMENTAL && (*itr)->ToCreature())
            {
                (*itr)->SetDisplayId(RAGNAROS_DISPLAY_ID);
                (*itr)->SetObjectScale(RAGNAROS_SCALE);
                // Double the health
                (*itr)->SetMaxHealth((*itr)->GetMaxHealth() * 2);
                (*itr)->SetHealth((*itr)->GetMaxHealth());
                LOG_INFO("module",
                    "mod-custom-spells: Player {} -> Fire Elemental → Ragnaros",
                    player->GetName());
                break;
            }
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_ele_ragnaros::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900403: Lightning Overload now also affects Lava Burst
//  Hooked on Lava Burst (all ranks via -51505). After hit,
//  if player has LO talent + this passive, trigger an overload.
// ============================================================
class spell_custom_ele_overload_lvb : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_overload_lvb);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !target->IsAlive())
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_OVERLOAD_LVB_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Check if player has Lightning Overload talent (icon 2018)
        AuraEffect const* loTalent = player->GetDummyAuraEffect(
            SPELLFAMILY_SHAMAN, 2018, EFFECT_0);
        if (!loTalent)
            return;

        // Proc chance = talent amount (11/22/33%)
        // With this passive, doubled chance
        int32 chance = loTalent->GetAmount() * 2;
        if (!roll_chance_i(chance))
            return;

        // Fire Lava Burst overload at half damage
        int32 damage = GetHitDamage() / 2;
        if (damage > 0)
            caster->CastCustomSpell(target, GetSpellInfo()->Id, &damage,
                nullptr, nullptr, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ele_overload_lvb::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900404: Lava Burst spreads Flame Shock
//  Hooked on Lava Burst (all ranks via -51505). After hit,
//  if target has Flame Shock, spread it to nearby enemies.
// ============================================================
class spell_custom_ele_lvb_spread_fs : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_lvb_spread_fs);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_LVB_SPREAD_FS_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Check if target has Flame Shock from this caster
        AuraEffect* fs = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE,
            SPELLFAMILY_SHAMAN, 0x10000000, 0, 0, caster->GetGUID());
        if (!fs)
            return;

        uint32 fsSpellId = fs->GetBase()->GetId();

        // Spread to up to 5 nearby enemies
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        uint32 count = 0;
        for (Unit* t : targets)
        {
            if (count >= 5)
                break;
            if (!t->IsAlive() || !caster->IsValidAttackTarget(t))
                continue;
            // Don't spread if already has FS from this caster
            if (t->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE,
                SPELLFAMILY_SHAMAN, 0x10000000, 0, 0, caster->GetGUID()))
                continue;

            caster->CastSpell(t, fsSpellId, true);
            ++count;
        }

        if (count > 0)
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> LvB spread FS to {} targets",
                player->GetName(), count);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ele_lvb_spread_fs::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900405: Flame Shock ticks → reset Lava Burst CD
//  Proc aura: on periodic damage, chance to reset LvB cooldown.
// ============================================================
class spell_custom_ele_fs_reset_lvb : public AuraScript
{
    PrepareAuraScript(spell_custom_ele_fs_reset_lvb);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Only proc on Flame Shock ticks (SpellFamilyFlags[0] = 0x10000000)
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell || procSpell->SpellFamilyName != SPELLFAMILY_SHAMAN)
            return;
        if (!(procSpell->SpellFamilyFlags[0] & 0x10000000))
            return;

        // Reset Lava Burst cooldown (all ranks)
        player->RemoveSpellCooldown(SPELL_LAVA_BURST_R2, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> FS tick reset LvB CD",
            player->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_ele_fs_reset_lvb::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900406: Lava Burst two charges
//  Hooked on Lava Burst (all ranks via -51505). After cast,
//  if this was the first charge, immediately reset CD.
//  Uses a hidden aura stack to track charges.
// ============================================================
class spell_custom_ele_lvb_charges : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_lvb_charges);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_LVB_TWO_CHARGES_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Check if we have a "charge available" marker
        // Use the passive aura's stack count as charge tracker
        Aura* chargeAura = player->GetAura(SPELL_ELE_LVB_TWO_CHARGES_PASSIVE);
        if (!chargeAura)
            return;

        uint8 stacks = chargeAura->GetStackAmount();
        if (stacks >= 2)
        {
            // Second charge used, set stacks to 1
            chargeAura->SetStackAmount(1);
            // Don't reset CD — it's now on real cooldown
        }
        else
        {
            // First charge used, reset CD immediately and mark
            chargeAura->SetStackAmount(2);
            player->RemoveSpellCooldown(GetSpellInfo()->Id, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_ele_lvb_charges::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900407: Clearcasting → Lava Burst instant
//  Implemented purely via DBC: ADD_PCT_MODIFIER with
//  SPELLMOD_CASTING_TIME = -100% on Lava Burst.
//  This makes LvB always instant when the passive is active.
//  (No separate C++ class needed — DBC handles it.)
// ============================================================

// ============================================================
//  SPELL 900434: 5 Maelstrom stacks → summons empowered AoE 5s
//  Hooked on Maelstrom Weapon (53817). When stacks reach 5,
//  apply a 5s buff (900439) that makes summons deal AoE.
// ============================================================
class spell_custom_enh_maelstrom_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_enh_maelstrom_aoe);

    void HandleStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetStackAmount() < 5)
            return;

        Unit* owner = GetUnitOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ENH_MAELSTROM_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Apply empowerment buff on player (5s duration)
        player->CastSpell(player, SPELL_ENH_MAELSTROM_AOE_BUFF, true);

        // Make all controlled units deal AoE around their targets
        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            Unit* pet = *itr;
            if (pet && pet->IsAlive() && pet->ToCreature())
                pet->CastSpell(pet, SPELL_ENH_WOLF_AOE_HELPER, true);
        }

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Maelstrom 5 stacks → summons empowered",
            player->GetName());
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_custom_enh_maelstrom_aoe::HandleStackChange,
            EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_CHANGE_AMOUNT);
    }
};

// ============================================================
//  SPELL 900436: Auto attacks → chance to summon wolf
//  Proc aura: on melee auto attack, chance to summon a
//  temporary wolf that fights for 15 seconds.
// ============================================================
class spell_custom_enh_wolf_summon : public AuraScript
{
    PrepareAuraScript(spell_custom_enh_wolf_summon);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Summon a temporary wolf (15s)
        if (Creature* wolf = player->SummonCreature(NPC_CUSTOM_WOLF,
            target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
            player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
        {
            wolf->SetOwnerGUID(player->GetGUID());
            wolf->SetCreatorGUID(player->GetGUID());
            wolf->SetFaction(player->GetFaction());
            wolf->Attack(target, true);
            wolf->GetMotionMaster()->MoveChase(target);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_enh_wolf_summon::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900437: Spirit Wolves inherit haste
//  Hooked on Feral Spirit (51533). After cast, apply owner's
//  haste rating to the wolves as attack speed reduction.
// ============================================================
class spell_custom_enh_wolf_haste : public SpellScript
{
    PrepareSpellScript(spell_custom_enh_wolf_haste);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ENH_WOLF_HASTE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Get owner's haste percentage
        float haste = player->GetFloatValue(UNIT_MOD_CAST_SPEED);
        // haste is a multiplier <1 = faster. E.g. 0.8 = 20% haste

        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            if ((*itr)->GetEntry() == NPC_SPIRIT_WOLF && (*itr)->ToCreature())
            {
                Creature* wolf = (*itr)->ToCreature();
                // Apply haste: reduce base attack time
                uint32 baseAttack = wolf->GetAttackTime(BASE_ATTACK);
                uint32 newAttack = static_cast<uint32>(baseAttack * haste);
                wolf->SetAttackTime(BASE_ATTACK, newAttack);

                LOG_INFO("module",
                    "mod-custom-spells: Player {} -> Spirit Wolf haste applied ({}ms → {}ms)",
                    player->GetName(), baseAttack, newAttack);
            }
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_enh_wolf_haste::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900438: Spirit Wolves 5% Chain Lightning on melee hit
//  UnitScript: On any damage dealt, if attacker is a Spirit
//  Wolf and owner has passive, 5% CL proc.
// ============================================================
class custom_wolf_cl_unitscript : public UnitScript
{
public:
    custom_wolf_cl_unitscript() : UnitScript("custom_wolf_cl_unitscript") {}

    void OnDamage(Unit* attacker, Unit* victim, uint32& /*damage*/) override
    {
        if (!attacker || !victim || !victim->IsAlive())
            return;

        Creature* wolf = attacker->ToCreature();
        if (!wolf || wolf->GetEntry() != NPC_SPIRIT_WOLF)
            return;

        Unit* owner = wolf->GetOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ENH_WOLF_CL_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // 5% chance
        if (!roll_chance_i(5))
            return;

        // Cast Chain Lightning from the wolf
        wolf->CastSpell(victim, SPELL_CL_R6, true);
    }
};

// ============================================================
//  SPELL 900467: Mana regen +2% per missing mana %
//  PlayerScript: periodically adjust mana regen based on
//  how much mana is missing.
// ============================================================
class custom_mana_regen_playerscript : public PlayerScript
{
public:
    custom_mana_regen_playerscript() : PlayerScript("custom_mana_regen_playerscript") {}

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_RST_MANA_REGEN_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Throttle: only every 5 seconds
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        static std::unordered_map<ObjectGuid, uint32> s_lastRegen;
        ObjectGuid guid = player->GetGUID();
        if (s_lastRegen.count(guid) && (now - s_lastRegen[guid]) < 5)
            return;
        s_lastRegen[guid] = now;

        uint32 maxMana = player->GetMaxPower(POWER_MANA);
        uint32 curMana = player->GetPower(POWER_MANA);
        if (maxMana == 0)
            return;

        // Calculate missing mana percentage (0-100)
        float missingPct = 100.0f * (1.0f - static_cast<float>(curMana) / maxMana);
        // Regen bonus = missingPct * 2% of max mana, applied per 5s tick
        int32 bonus = static_cast<int32>(maxMana * (missingPct * 0.02f) / 100.0f);
        // Scale to per-5s (this runs every 5s)
        if (bonus > 0)
            player->EnergizeBySpell(player, SPELL_RST_MANA_REGEN_PASSIVE,
                bonus, POWER_MANA);
    }
};

// ============================================================
//  Extend totem follow PlayerScript to also check 900433 + 900466
// ============================================================

void AddCustomSpellsScripts()
{
    RegisterSpellScript(spell_custom_paragon_strike);
    RegisterSpellScript(spell_custom_bladestorm_cd_reduce);
    RegisterSpellScript(spell_custom_bloody_whirlwind_passive);
    RegisterSpellScript(spell_custom_bloody_whirlwind_consume);
    RegisterSpellScript(spell_custom_speedy_bloodthirst);
    RegisterSpellScript(spell_custom_ww_overpower);
    RegisterSpellScript(spell_custom_ww_slam);

    // Warrior Prot
    RegisterSpellScript(spell_custom_prot_revenge_aoe);
    RegisterSpellScript(spell_custom_prot_tc_rend_sunder);
    RegisterSpellScript(spell_custom_prot_block_aoe);
    RegisterSpellScript(spell_custom_prot_block_tc);

    // Paladin Holy
    RegisterSpellScript(spell_custom_holy_hs_aoe_dmg);
    RegisterSpellScript(spell_custom_holy_hs_aoe_heal);
    RegisterSpellScript(spell_custom_holy_hs_both_dmg);
    RegisterSpellScript(spell_custom_holy_hs_both_heal);
    RegisterSpellScript(spell_custom_holy_consec_heal);

    // Paladin Prot
    RegisterSpellScript(spell_custom_pprot_as_consec);
    RegisterSpellScript(spell_custom_pprot_judge_as);

    // Paladin Ret
    RegisterSpellScript(spell_custom_ret_cs_aoe);
    RegisterSpellScript(spell_custom_ret_exorcism_proc);
    RegisterSpellScript(spell_custom_ret_exorcism_consume);

    // DK Blood
    RegisterSpellScript(spell_custom_dkb_3_rune_weapons);
    RegisterSpellScript(spell_custom_dkb_double_cast);
    RegisterSpellScript(spell_custom_dkb_hs_aoe);
    RegisterSpellScript(spell_custom_dkb_deathcoil_proc);

    // DK Frost
    RegisterSpellScript(spell_custom_dkf_frost_wyrm);
    RegisterCreatureAI(npc_custom_frost_wyrm);
    RegisterSpellScript(spell_custom_frost_breath);

    // DK Unholy
    RegisterSpellScript(spell_custom_dku_dot_aoe);

    // Shaman Elemental
    RegisterSpellScript(spell_custom_ele_cl_aoe);
    new custom_totem_follow_playerscript();
    RegisterSpellScript(spell_custom_ele_ragnaros);
    RegisterSpellScript(spell_custom_ele_overload_lvb);
    RegisterSpellScript(spell_custom_ele_lvb_spread_fs);
    RegisterSpellScript(spell_custom_ele_fs_reset_lvb);
    RegisterSpellScript(spell_custom_ele_lvb_charges);
    // 900407: Clearcasting → LvB instant is DBC-only (no C++ needed)

    // Shaman Enhancement
    RegisterSpellScript(spell_custom_enh_maelstrom_aoe);
    RegisterSpellScript(spell_custom_enh_wolf_summon);
    RegisterSpellScript(spell_custom_enh_wolf_haste);
    new custom_wolf_cl_unitscript();
    // 900433/900466: Totem follow reuses custom_totem_follow_playerscript (checks all 3 passives)
    // 900435: Summons +50% damage is DBC-only

    // Shaman Resto
    new custom_mana_regen_playerscript();
    // 900466: Totem follow reuses custom_totem_follow_playerscript
}
