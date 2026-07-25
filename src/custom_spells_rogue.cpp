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

        if (!g_CustomSpellsEnabled)
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
//  End Rogue section
// ============================================================

void AddRogueSpellsScripts()
{
    // Rogue Assa
    RegisterSpellScript(spell_custom_rog_poison_nova);

    // Rogue Combat

    // Rogue Sub
}
