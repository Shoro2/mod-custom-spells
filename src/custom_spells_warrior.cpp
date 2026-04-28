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


// NOTE: Warrior Fury spells (900108-900121) are defined purely in
// Spell.dbc (manually created). No C++ scripts needed — all effects
// are handled via DBC passive auras, proc triggers, and spell_proc.
// The old 900138-900145 C++ implementations have been removed.

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

        SpellInfo const* spellInfo = GetSpellInfo();

        for (Unit* target : targets)
        {
            if (!target->IsAlive() || !caster->IsValidAttackTarget(target))
                continue;

            SpellNonMeleeDamage dmgInfo(caster, target, spellInfo->Id, spellInfo->GetSchoolMask());
            dmgInfo.damage = damage;
            caster->DealSpellDamage(&dmgInfo, true);
            caster->SendSpellNonMeleeDamageLog(&dmgInfo);

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

    // Warrior Prot
    RegisterSpellScript(spell_custom_prot_revenge_aoe);
    RegisterSpellScript(spell_custom_prot_tc_rend_sunder);
    RegisterSpellScript(spell_custom_prot_block_aoe);
    RegisterSpellScript(spell_custom_prot_block_tc);
}
