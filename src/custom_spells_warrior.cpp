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

#include "custom_spells_common.h"

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
            EFFECT_0, SPELL_AURA_DUMMY);
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


void AddWarriorSpellsScripts()
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
}
