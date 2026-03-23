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
//  PRIEST DISCIPLINE
// ============================================================

// ============================================================
//  900900: Shields explode on breaking/fading
//  Hooked on Power Word: Shield (all ranks via -48066).
//  OnRemove: if shield expires (fade) or is consumed by damage
//  (absorb depleted), cast AoE explosion centered on the
//  shielded target. Damage scales with remaining/absorbed
//  shield amount.
// ============================================================
class spell_custom_pri_shield_explode : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_shield_explode);

    void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_PRI_DISC_SHIELD_EXPLODE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();

        // Only explode on expire (fade) or absorb-depleted (enemy spell broke it)
        if (removeMode != AURA_REMOVE_BY_EXPIRE &&
            removeMode != AURA_REMOVE_BY_ENEMY_SPELL &&
            removeMode != AURA_REMOVE_BY_DEFAULT)
            return;

        // Calculate explosion damage based on the original absorb amount
        // aurEff->GetAmount() = remaining absorb; GetBaseAmount() = original
        int32 originalAbsorb = aurEff->GetBaseAmount();
        int32 remainingAbsorb = aurEff->GetAmount();

        // Use the absorbed amount (original - remaining) if shield was broken,
        // or the original amount if it expired with absorb remaining
        int32 explosionDamage;
        if (removeMode == AURA_REMOVE_BY_ENEMY_SPELL)
            explosionDamage = originalAbsorb; // fully consumed → full damage
        else
            explosionDamage = std::max(originalAbsorb / 2, remainingAbsorb); // expired → at least 50%

        if (explosionDamage <= 0)
            return;

        // Cast AoE helper with custom damage centered on shield target
        player->CastCustomSpell(SPELL_PRI_DISC_EXPLODE_HELPER, SPELLVALUE_BASE_POINT0,
            explosionDamage, target, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_pri_shield_explode::HandleRemove,
            EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

// ============================================================
//  End Priest Discipline section
// ============================================================

// ============================================================
//  PRIEST HOLY
// ============================================================

// ============================================================
//  900933: Direct heals have chance to cast Holy Fire on enemies
//  Proc aura: on positive spell hit (direct heal), 20% chance
//  to cast Holy Fire on all enemies within 10yd of the heal
//  target. Filters out HoTs by checking SpellPhaseMask for
//  HIT phase (not periodic). 3s ICD.
// ============================================================
class spell_custom_pri_heal_fire : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_heal_fire);

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

        // Must be a Priest spell (SpellFamilyName = 6)
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell || procSpell->SpellFamilyName != SPELLFAMILY_PRIEST_ID)
            return;

        // Filter: only direct heals (not periodic/HoTs)
        // Check that it's a healing spell with direct component
        bool isDirect = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (procSpell->Effects[i].Effect == SPELL_EFFECT_HEAL ||
                procSpell->Effects[i].Effect == SPELL_EFFECT_HEAL_PCT)
            {
                isDirect = true;
                break;
            }
        }
        if (!isDirect)
            return;

        Unit* healTarget = eventInfo.GetActionTarget();
        if (!healTarget)
            return;

        // Find all enemies within 10yd of the heal target
        std::list<Unit*> enemies;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(healTarget, player, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(healTarget, enemies, check);
        Cell::VisitObjects(healTarget, searcher, 10.0f);

        for (Unit* enemy : enemies)
        {
            if (!enemy->IsAlive() || !player->IsValidAttackTarget(enemy))
                continue;

            player->CastSpell(enemy, SPELL_HOLY_FIRE_R11, true);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_pri_heal_fire::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  End Priest Holy section
// ============================================================

// ============================================================
//  PRIEST SHADOW
// ============================================================

// ============================================================
//  900966: DoT ticks deal Shadow AoE (same pattern as Warlock
//  900800). Proc aura: on periodic damage tick from Priest
//  DoTs, 20% chance to cast Shadow AoE helper (900968)
//  centered on the DoT target. 2s ICD.
// ============================================================
class spell_custom_pri_dot_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_dot_aoe);

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
        player->CastSpell(target, SPELL_PRI_SHADOW_AOE_HELPER, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_pri_dot_aoe::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  900967: DoT ticks spread to 2 additional targets
//  (same pattern as Warlock 900802). On periodic damage tick,
//  15% chance to copy the ticking DoT to up to 2 nearby
//  enemies that don't already have it. 3s ICD.
// ============================================================
class spell_custom_pri_dot_spread : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_dot_spread);

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

        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return;

        uint32 dotSpellId = procSpell->Id;

        // Only spread Priest DoTs (SpellFamilyName = 6)
        if (procSpell->SpellFamilyName != SPELLFAMILY_PRIEST_ID)
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

            if (spreadTarget->HasAura(dotSpellId, player->GetGUID()))
                continue;

            player->CastSpell(spreadTarget, dotSpellId, true);
            ++count;
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_pri_dot_spread::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  End Priest Shadow section
// ============================================================

void AddPriestSpellsScripts()
{
    // Priest Discipline
    RegisterSpellScript(spell_custom_pri_shield_explode);

    // Priest Holy
    RegisterSpellScript(spell_custom_pri_heal_fire);

    // Priest Shadow
    RegisterSpellScript(spell_custom_pri_dot_aoe);
    RegisterSpellScript(spell_custom_pri_dot_spread);
}
