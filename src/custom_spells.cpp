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
}
