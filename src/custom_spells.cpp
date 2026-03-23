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
    // Bloody Whirlwind buff: +50% Whirlwind damage per stack, consumed on WW cast
    SPELL_BLOODY_WHIRLWIND_BUFF         = 900115,
    // Bloody Whirlwind passive: procs 900115 on Bloodthirst hit
    SPELL_BLOODY_WHIRLWIND_PASSIVE      = 900116,
    // Speedy Bloodthirst: Whirlwind resets Bloodthirst cooldown
    SPELL_CUSTOM_SPEEDY_BLOODTHIRST     = 900117,
    // WW single-target: passive auras (markers checked via HasAura)
    SPELL_WW_OVERPOWER_PASSIVE          = 900118,
    SPELL_WW_SLAM_PASSIVE               = 900119,
    // WW single-target: boosted damage spells (autocast)
    SPELL_WW_OVERPOWER_BOOSTED          = 900120,
    SPELL_WW_SLAM_BOOSTED               = 900121,

    // ---- Warrior Prot (900122-900129) ----
    SPELL_PROT_REVENGE_DMG_PASSIVE      = 900122, // Revenge +50% damage (DBC-only)
    SPELL_PROT_REVENGE_AOE_PASSIVE      = 900123, // Revenge unlimited targets (C++)
    SPELL_PROT_TC_REND_SUNDER_PASSIVE   = 900124, // TC → Rend + 5× Sunder (C++)
    SPELL_PROT_TC_DMG_PASSIVE           = 900125, // TC +50% damage (DBC-only)
    SPELL_PROT_BLOCK_AOE_PASSIVE        = 900126, // AoE damage on Block (C++)
    SPELL_PROT_BLOCK_TC_PASSIVE         = 900127, // 10% Block → Enhanced TC (C++)
    SPELL_PROT_BLOCK_AOE_DAMAGE         = 900128, // Helper: AoE damage (triggered)
    SPELL_PROT_ENHANCED_THUNDERCLAP     = 900129, // Helper: Enhanced TC (triggered)

    // ---- Paladin Holy (900150-900161) ----
    SPELL_HOLY_HS_AOE_DMG_PASSIVE       = 900150, // Holy Shock AoE damage (C++)
    SPELL_HOLY_HS_AOE_HEAL_PASSIVE      = 900151, // Holy Shock AoE heal (C++)
    SPELL_HOLY_HS_BOTH_PASSIVE          = 900152, // Holy Shock always both (C++)
    SPELL_HOLY_HS_DMG_PASSIVE           = 900153, // Holy Shock +50% (DBC-only)
    SPELL_HOLY_CONSEC_HEAL_PASSIVE      = 900154, // Consecration also heals (C++)
    SPELL_HOLY_CONSEC_AROUND_PASSIVE    = 900155, // Consecration around you (DBC)
    SPELL_HOLY_CONSEC_DMG_PASSIVE       = 900156, // Consecration +50% (DBC-only)
    SPELL_HOLY_CONSEC_DUR_PASSIVE       = 900157, // Consecration +5sec (DBC-only)
    // Helper spells
    SPELL_HOLY_HS_AOE_DMG_HELPER        = 900158, // AoE Holy damage around target
    SPELL_HOLY_HS_AOE_HEAL_HELPER       = 900159, // AoE Holy heal around target
    SPELL_HOLY_CONSEC_HEAL_HELPER       = 900160, // Consecration heal tick helper

    // ---- Paladin Prot (900161-900168) ----
    SPELL_PPROT_CONSEC_AROUND_PASSIVE   = 900161, // Consecration around you (DBC marker)
    SPELL_PPROT_AS_TARGETS_PASSIVE      = 900162, // Avenger's Shield +9 targets (DBC)
    SPELL_PPROT_AS_DMG_PASSIVE          = 900163, // Avenger's Shield +50% (DBC)
    SPELL_PPROT_HS_CHARGES_PASSIVE      = 900164, // Holy Shield charges +99 (DBC)
    SPELL_PPROT_HS_DMG_PASSIVE          = 900165, // Holy Shield +50% (DBC)
    SPELL_PPROT_AS_CONSEC_PASSIVE       = 900166, // AS leaves Consecration (C++)
    SPELL_PPROT_JUDGE_AS_PASSIVE        = 900167, // Judgement → free AS (C++)
    SPELL_PPROT_JUDGE_CD_PASSIVE        = 900168, // Judgement cd -2sec (DBC)

    // ---- Paladin Ret (900169-900176) ----
    SPELL_RET_CONSEC_AROUND_PASSIVE     = 900169, // Consecration around you (DBC marker)
    SPELL_RET_JUDGE_CD_PASSIVE          = 900170, // Judgement cd -2sec (DBC)
    SPELL_RET_DS_TARGETS_PASSIVE        = 900171, // Divine Storm +6 targets (DBC marker)
    SPELL_RET_DS_DMG_PASSIVE            = 900172, // Divine Storm +50% (DBC)
    SPELL_RET_CS_DMG_PASSIVE            = 900173, // Crusader Strike +50% (DBC)
    SPELL_RET_CS_AOE_PASSIVE            = 900174, // Crusader Strike +9 targets (C++)
    SPELL_RET_EXORCISM_PROC_PASSIVE     = 900175, // CS/Judge/DS → Exorcism buff (C++)
    SPELL_RET_EXORCISM_BUFF             = 900176, // Exorcism +50% per stack (DBC buff)
};

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
//  SPELL 900116: Bloody Whirlwind Passive (AuraScript)
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
//  1. Remove all stacks of the Bloody Whirlwind buff (900115)
//  2. If WW hit exactly 1 target and player has passive 900118/900119,
//     autocast boosted Overpower (900120) / Slam (900121) on the target
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
//  SPELL 900117: Speedy Bloodthirst (AuraScript)
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
            LOG_INFO("module", "mod-custom-spells: 900117 CheckProc -> "
                "NO spell info, returning false");
            return false;
        }

        LOG_INFO("module", "mod-custom-spells: 900117 CheckProc -> "
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
            LOG_INFO("module", "mod-custom-spells: 900117 HandleProc -> "
                "NO player, returning");
            return;
        }

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        LOG_INFO("module", "mod-custom-spells: 900117 HandleProc -> "
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
//  SPELL 900120: WW Boosted Overpower (SpellScript)
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
//  SPELL 900121: WW Boosted Slam (SpellScript)
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
//  SPELL 900123: Revenge Unlimited Targets (SpellScript)
//  Hooked on Revenge (57823). After hitting the primary target,
//  deals the same damage to all enemies within 8yd of caster.
//  Only active when player has passive aura 900123.
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
//  SPELL 900124: Thunderclap → Rend + 5× Sunder Armor
//  Hooked on Thunderclap (47502). After hitting each target,
//  applies Rend and 5 stacks of Sunder Armor.
//  Only active when player has passive aura 900124.
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
//  SPELL 900126: AoE Damage on Block (AuraScript)
//  Passive proc aura: when the warrior blocks an attack,
//  deals AoE physical damage (900128) to nearby enemies.
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
//  SPELL 900127: 10% Block → Enhanced Thunderclap (AuraScript)
//  Passive proc aura: when the warrior blocks an attack,
//  10% chance to cast Enhanced Thunderclap (900129).
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
//  SPELL 900150: Holy Shock AoE Damage (SpellScript)
//  Hooked on Holy Shock damage spells (48824 etc.).
//  After hitting a hostile target, casts AoE holy damage
//  helper (900158) centered on the target.
//  Only active when player has passive 900150.
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
//  SPELL 900151: Holy Shock AoE Heal (SpellScript)
//  Hooked on Holy Shock heal spells (48825 etc.).
//  After healing a friendly target, casts AoE holy heal
//  helper (900159) centered on the target.
//  Only active when player has passive 900151.
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
//  SPELL 900152: Holy Shock Always Both (SpellScript)
//  Hooked on Holy Shock damage (48824) and heal (48825).
//  After hitting a hostile target → also heal nearest ally.
//  After healing a friendly target → also damage nearest enemy.
//  Only active when player has passive 900152.
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
//  SPELL 900154: Consecration Also Heals (SpellScript)
//  Hooked on Consecration (48819). On each periodic tick,
//  also casts a heal helper on friendly units in the area.
//  Only active when player has passive 900154.
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
//  SPELL 900166: Avenger's Shield Leaves Consecration
//  Hooked on Avenger's Shield (48827). After hitting each
//  target, casts Consecration at the target's position.
//  Only active when player has passive 900166.
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
//  SPELL 900167: Judgement → Free Avenger's Shield
//  Hooked on Judgement Damage (54158). After Judgement hits,
//  auto-casts Avenger's Shield at the same target.
//  Only active when player has passive 900167.
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
//  SPELL 900174: Crusader Strike +9 Targets (SpellScript)
//  Hooked on Crusader Strike (35395). After hitting primary
//  target, deals same damage to up to 9 additional enemies
//  within 8yd of caster.
//  Only active when player has passive 900174.
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
//  SPELL 900175: Exorcism Buff System (AuraScript)
//  Passive proc aura: when CS, Judgement, or Divine Storm
//  hits an enemy, adds 1 stack of Exorcism buff (900176).
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
//  After casting Exorcism, consumes all stacks of 900176.
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
}
