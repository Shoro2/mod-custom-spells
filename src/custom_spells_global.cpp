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
//  NON-CLASS GLOBAL SPELLS (901100-901199)
//  These passive auras apply to ALL classes at Paragon level.
//  SpellFamilyName = 0 (Generic).
// ============================================================

// ============================================================
//  901100: Cast while moving
//  DBC passive aura using SPELL_AURA_CAST_WHILE_WALKING (330).
//  This is a pure DBC spell — no C++ needed. The aura type
//  allows casting all spells (including channels) while moving.
//  Registered here only for completeness; the DBC entry does
//  all the work.
// ============================================================
// (No C++ class needed — DBC aura 330 handles it natively)

// ============================================================
//  901101: Kill enemy → heal 5% total HP
//  Proc aura: on PROC_FLAG_KILL, heals the player for 5% of
//  their maximum health via helper spell 901105.
// ============================================================
class spell_custom_global_kill_heal : public AuraScript
{
    PrepareAuraScript(spell_custom_global_kill_heal);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 healAmount = static_cast<int32>(player->GetMaxHealth() * 0.05f);
        if (healAmount <= 0)
            return;

        player->CastCustomSpell(SPELL_GLOBAL_KILL_HEAL_HELPER,
            SPELLVALUE_BASE_POINT0, healAmount, player, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_global_kill_heal::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  901102: Attacks 25% chance to hit again (Extra Attack)
//  Proc aura: on melee auto attack or melee spell, 25% chance
//  to grant 1 extra attack. Uses SPELL_AURA_ADD_EXTRA_ATTACKS
//  in DBC for the melee part. C++ proc handler prevents
//  recursive procs by checking triggered flag.
// ============================================================
class spell_custom_global_extra_attack : public AuraScript
{
    PrepareAuraScript(spell_custom_global_extra_attack);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Prevent recursive procs: if the triggering event came from a
        // triggered spell (i.e. from an extra attack proc), do not proc again
        if (eventInfo.GetDamageInfo() && eventInfo.GetDamageInfo()->GetSpellInfo()
            && eventInfo.GetDamageInfo()->GetSpellInfo()->Id == SPELL_GLOBAL_EXTRA_ATTACK_HELPER)
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Grant 1 extra melee attack via helper spell (SPELL_EFFECT_ADD_EXTRA_ATTACKS)
        player->CastSpell(player, SPELL_GLOBAL_EXTRA_ATTACK_HELPER, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_global_extra_attack::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  901103: Spells/abilities 10% chance to hit all enemies
//  within 10yd of target.
//  Proc aura: on damage dealt, 10% chance to deal the same
//  damage to all enemies within 10yd via helper spell 901106.
//  1s ICD to prevent DoT-tick spam.
// ============================================================
class spell_custom_global_cleave_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_global_cleave_proc);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Get original damage amount
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo)
            return;

        int32 damage = static_cast<int32>(damageInfo->GetDamage());
        if (damage <= 0)
            return;

        // Find all enemies within 10yd of the target
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, player, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(target, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        for (Unit* extra : targets)
        {
            if (!extra->IsAlive() || !player->IsValidAttackTarget(extra))
                continue;

            player->CastCustomSpell(SPELL_GLOBAL_CLEAVE_HELPER,
                SPELLVALUE_BASE_POINT0, damage, extra, true);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_global_cleave_proc::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  901104: Avoid attack → counter attack
//  Proc aura: on dodge/parry/block of a melee attack, deal
//  instant physical damage back to the attacker via helper
//  spell 901107. Damage = 50% of player's AP.
// ============================================================
class spell_custom_global_counter_attack : public AuraScript
{
    PrepareAuraScript(spell_custom_global_counter_attack);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Must be a dodge, parry, or block
        uint32 hitMask = eventInfo.GetHitMask();
        if (!(hitMask & (PROC_HIT_DODGE | PROC_HIT_PARRY | PROC_HIT_BLOCK)))
            return;

        Unit* attacker = eventInfo.GetActor();
        if (!attacker || !attacker->IsAlive() || attacker == player)
            return;

        // Counter damage = 50% of AP
        int32 counterDamage = static_cast<int32>(
            player->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f);
        if (counterDamage <= 0)
            counterDamage = 100; // minimum floor

        player->CastCustomSpell(SPELL_GLOBAL_COUNTER_HELPER,
            SPELLVALUE_BASE_POINT0, counterDamage, attacker, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_global_counter_attack::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  End Non-Class Global section
// ============================================================

void AddGlobalSpellsScripts()
{
    // 901100: Cast while moving — DBC only, no C++ needed
    RegisterSpellScript(spell_custom_global_kill_heal);
    RegisterSpellScript(spell_custom_global_extra_attack);
    RegisterSpellScript(spell_custom_global_cleave_proc);
    RegisterSpellScript(spell_custom_global_counter_attack);
}
