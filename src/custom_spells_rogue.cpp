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

    // Weapon poisons are triggered casts, so the proc row carries
    // PROC_ATTR_TRIGGERED_CAN_PROC - which would let the nova's own Nature
    // damage set off the next nova: never from the helper itself.
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && spellInfo->Id != SPELL_ROG_ASSA_POISON_NOVA_HELPER;
    }

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
        CastAnchoredBurst(player, target, SPELL_ROG_ASSA_POISON_NOVA_HELPER);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_rog_poison_nova::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_rog_poison_nova::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  ROGUE COMBAT: Blade Flurry +9 targets (900636)
//  Blade Flurry's extra hit is the core's proc 22482 (one other
//  enemy near the rogue gets the hit's damage). Hooked on 22482:
//  the same damage goes to up to 8 more enemies within 8 yd of the
//  rogue - the rogue's victim and 22482's own target excluded - so a
//  hit reaches up to 10 enemies. (The old jump-target modifier sat
//  on the Blade Flurry self-aura and changed nothing.)
// ============================================================
class spell_custom_rog_bf_targets : public SpellScript
{
    PrepareSpellScript(spell_custom_rog_bf_targets);

    void HandleAfterHit()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* bfTarget = GetHitUnit();
        if (!player || !bfTarget || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_ROG_COMBAT_BF_TARGETS_PASSIVE))
            return;

        int32 const damage = GetHitDamage();
        if (damage <= 0)
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(player, player, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(player, targets, check);
        Cell::VisitObjects(player, searcher, 8.0f);
        targets.remove(bfTarget);
        targets.remove(player->GetVictim());

        SpellInfo const* spellInfo = GetSpellInfo();
        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= BLADE_FLURRY_EXTRA_TARGETS)
                break;
            if (!target->IsAlive() || !player->IsValidAttackTarget(target))
                continue;

            SpellNonMeleeDamage dmgInfo(player, target, spellInfo,
                spellInfo->GetSchoolMask());
            dmgInfo.damage = damage;
            player->DealSpellDamage(&dmgInfo, true);
            player->SendSpellNonMeleeDamageLog(&dmgInfo);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_rog_bf_targets::HandleAfterHit);
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
    RegisterSpellScript(spell_custom_rog_bf_targets);

    // Rogue Sub
}
