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
//  ROGUE ASSA: Poison Nova proc (900603)
//  Proc passive: on poison damage dealt, 15% chance to cast
//  Poison Nova AoE at the target. 3s ICD.
// ============================================================
class spell_custom_rog_poison_nova : public AuraScript
{
    PrepareAuraScript(spell_custom_rog_poison_nova);

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

        // Cast Poison Nova AoE centered on target
        player->CastSpell(target, SPELL_ROG_ASSA_POISON_NOVA_HELPER, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_rog_poison_nova::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  ROGUE COMBAT: Sinister Strike +9 targets (900634)
//  Hooked on Sinister Strike (all ranks via -48638). After
//  hitting the main target, deals same damage to up to 9
//  additional enemies in 8yd melee range.
// ============================================================
class spell_custom_rog_ss_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_rog_ss_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ROG_COMBAT_SS_AOE_PASSIVE))
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
            if (!target->IsAlive() || !caster->IsValidAttackTarget(target))
                continue;

            caster->CastCustomSpell(target, SPELL_ROG_COMBAT_SS_AOE_HELPER,
                &damage, nullptr, nullptr, true);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_rog_ss_aoe::HandleAfterHit);
    }
};

// ============================================================
//  ROGUE SUB: Hemorrhage +9 targets (900668)
//  Hooked on Hemorrhage (all ranks via -48660). After hitting
//  the main target, deals same damage to up to 9 additional
//  enemies in 8yd melee range.
// ============================================================
class spell_custom_rog_hemo_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_rog_hemo_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ROG_SUB_HEMO_AOE_PASSIVE))
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
            if (!target->IsAlive() || !caster->IsValidAttackTarget(target))
                continue;

            caster->CastCustomSpell(target, SPELL_ROG_SUB_HEMO_AOE_HELPER,
                &damage, nullptr, nullptr, true);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_rog_hemo_aoe::HandleAfterHit);
    }
};

// ============================================================
//  End Rogue section
// ============================================================

void AddRogueSpellsScripts()
{
    // Rogue Assa
    RegisterSpellScript(spell_custom_rog_poison_nova);

    // Rogue Combat
    RegisterSpellScript(spell_custom_rog_ss_aoe);

    // Rogue Sub
    RegisterSpellScript(spell_custom_rog_hemo_aoe);
}
