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
//  WARLOCK AFFLICTION: DoT ticks deal Shadow AoE (900800)
//  Proc aura: when a periodic damage tick lands (any Warlock
//  DoT), X% chance to cast Shadow AoE helper (900803)
//  centered on the DoT target. 2s ICD.
// ============================================================
class spell_custom_wlk_dot_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_dot_aoe);

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

        // Cast Shadow AoE centered on the DoT target
        player->CastSpell(target, SPELL_WLK_AFFL_DOT_AOE_HELPER, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_wlk_dot_aoe::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  WARLOCK AFFLICTION: DoT ticks spread to 2 additional
//  targets (900802). Proc aura: on periodic damage tick,
//  X% chance to copy the ticking DoT to up to 2 nearby
//  enemies that don't already have it.
// ============================================================
class spell_custom_wlk_dot_spread : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_dot_spread);

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

        // Get the spell that caused the periodic damage
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return;

        uint32 dotSpellId = procSpell->Id;

        // Only spread Warlock DoTs (SpellFamilyName = 5)
        if (procSpell->SpellFamilyName != SPELLFAMILY_WARLOCK_ID)
            return;

        // Find up to 2 nearby enemies that don't have this DoT
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, player, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(target, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        uint32 count = 0;
        for (Unit* spreadTarget : targets)
        {
            if (count >= 2)
                break;
            if (!spreadTarget->IsAlive() || !player->IsValidAttackTarget(spreadTarget))
                continue;

            // Skip if target already has this DoT from us
            if (spreadTarget->HasAura(dotSpellId, player->GetGUID()))
                continue;

            // Apply the DoT to the new target
            player->CastSpell(spreadTarget, dotSpellId, true);
            ++count;
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_wlk_dot_spread::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  End Warlock Affliction section
// ============================================================

void AddWarlockSpellsScripts()
{
    // Warlock Affliction
    RegisterSpellScript(spell_custom_wlk_dot_aoe);
    RegisterSpellScript(spell_custom_wlk_dot_spread);
}
