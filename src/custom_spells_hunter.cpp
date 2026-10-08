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
#include "Pet.h"

// ============================================================
//  HUNTER: Shared - Get back arrows (900500)
//  PlayerScript: after every ranged attack, restore the ammo
//  that was consumed. Effectively infinite arrows.
// ============================================================
class custom_hunter_arrows_playerscript : public PlayerScript
{
public:
    custom_hunter_arrows_playerscript() : PlayerScript("custom_hunter_arrows_playerscript") {}

    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_HUNT_ARROWS_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        SpellInfo const* spellInfo = spell->GetSpellInfo();
        if (!spellInfo)
            return;

        // Only ranged auto-attacks and ranged spells consume ammo
        // Check if this spell uses ammo (ranged attacks)
        if (!(spellInfo->Attributes & SPELL_ATTR0_USES_RANGED_SLOT))
            return;

        uint32 ammoId = player->GetUInt32Value(PLAYER_AMMO_ID);
        if (!ammoId)
            return;

        // The core takes one ammo per target hit (Spell::HandleLaunchPhase),
        // after this hook: Multi-Shot on three targets used three. Count
        // now and give back what the shot used once it has launched (the
        // event belongs to the player, so it never outlives it).
        uint32 const before = player->GetItemCount(ammoId);
        player->m_Events.AddEventAtOffset([player, ammoId, before]()
        {
            uint32 const now = player->GetItemCount(ammoId);
            if (now < before)
                player->StoreNewItemInBestSlots(ammoId, before - now);
        }, 1ms);
    }
};

// ============================================================
//  HUNTER: Shared - Multi-Shot unlimited targets (900501)
//  Hooked on Multi-Shot (all ranks via -2643). Multi-Shot is a
//  chain spell (first target + 2 jumps); the script adds every other
//  enemy within 10 yd of the first target to the chain list, so each
//  one takes one real Multi-Shot hit (own damage roll, crits, combat
//  log). The old AfterHit pass dealt the first target's damage to
//  all enemies around it again - Multi-Shot's own 2nd and 3rd target
//  were hit twice.
// ============================================================
class spell_custom_hunt_multishot_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_hunt_multishot_aoe);

    void AddTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetExplTargetUnit();
        if (!caster || !mainTarget || !g_CustomSpellsEnabled)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->HasAura(SPELL_HUNT_MULTISHOT_AOE_PASSIVE))
            return;

        std::list<Unit*> nearby;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(mainTarget, caster,
            10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, nearby, check);
        Cell::VisitObjects(mainTarget, searcher, 10.0f);

        for (Unit* unit : nearby)
        {
            if (unit == mainTarget || !unit->IsAlive()
                || !caster->IsValidAttackTarget(unit))
                continue;
            if (std::find(targets.begin(), targets.end(), unit)
                != targets.end())
                continue;
            targets.push_back(unit);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(
            spell_custom_hunt_multishot_aoe::AddTargets, EFFECT_0,
            TARGET_UNIT_TARGET_ENEMY);
    }
};

// ============================================================
//  HUNTER BM: 900502 Pet Damage +50% / 900503 Pet Speed +50% are
//  minion auras kept by custom_minion_aura_playerscript
//  (custom_spells_global.cpp). The old code multiplied the damage
//  inside OnDamage (invisible in the combat log) and halved the pet's
//  attack time every 3 sec (= +100 % speed, overwriting haste).
// ============================================================

// ============================================================
//  HUNTER BM: Pet chance to deal AoE damage (900504)
//  A white melee hit of the hunter's pet has a 15 % chance to set
//  off Beast Cleave (900505) around its target. Only white melee
//  hits count (DealDamage with DIRECT_DAMAGE): the old OnDamage hook
//  also fired on Beast Cleave's own damage, so on a big pack one
//  cleave could set off the next ones.
// ============================================================
class custom_hunter_pet_aoe_unitscript : public UnitScript
{
public:
    custom_hunter_pet_aoe_unitscript()
        : UnitScript("custom_hunter_pet_aoe_unitscript") { }

    uint32 DealDamage(Unit* attacker, Unit* victim, uint32 damage,
        DamageEffectType damagetype) override
    {
        if (damagetype != DIRECT_DAMAGE || !attacker || !victim
            || !victim->IsAlive() || !g_CustomSpellsEnabled)
            return damage;

        Creature* pet = attacker->ToCreature();
        if (!pet || !pet->IsPet())
            return damage;

        Player* player = pet->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!player || !player->HasAura(SPELL_HUNT_BM_PET_AOE_PASSIVE))
            return damage;

        if (roll_chance_i(15))
            CastAnchoredBurst(pet, victim, SPELL_HUNT_BM_PET_AOE_HELPER);
        return damage;
    }
};

// ============================================================
//  HUNTER MM: Auto Shot bounces +9 targets (900533)
//  Hooked on Auto Shot (75). After hitting main target,
//  bounces to up to 9 additional enemies within 10yd.
// ============================================================
class spell_custom_hunt_autoshot_bounce : public SpellScript
{
    PrepareSpellScript(spell_custom_hunt_autoshot_bounce);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HUNT_MM_AUTOSHOT_BOUNCE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        // Find up to 9 additional enemies within 10yd of target
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(mainTarget, caster, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(mainTarget, searcher, 10.0f);
        targets.remove(mainTarget);

        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= 9)
                break;
            if (!target->IsAlive() || !caster->IsValidAttackTarget(target))
                continue;

            SpellInfo const* spellInfo = GetSpellInfo();
            SpellNonMeleeDamage dmgInfo(caster, target, spellInfo, spellInfo->GetSchoolMask());
            dmgInfo.damage = damage;
            caster->DealSpellDamage(&dmgInfo, true);
            caster->SendSpellNonMeleeDamageLog(&dmgInfo);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_hunt_autoshot_bounce::HandleAfterHit);
    }
};

// ============================================================
//  HUNTER MM: Multi-Shot Barrage (900534)
//  Active spell: 2s duration periodic (ticks every 100ms).
//  Each tick auto-casts Multi-Shot. Applies 50% slow during channel.
//  Implemented as AuraScript on the barrage spell itself.
// ============================================================
class spell_custom_hunt_barrage : public AuraScript
{
    PrepareAuraScript(spell_custom_hunt_barrage);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetTarget();
        if (!caster)
            return;

        // Apply 50% slow while channeling
        caster->CastSpell(caster, SPELL_HUNT_MM_BARRAGE_SLOW, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetTarget();
        if (!caster)
            return;

        // Remove slow when barrage ends
        caster->RemoveAura(SPELL_HUNT_MM_BARRAGE_SLOW);
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Get current target
        Unit* target = ObjectAccessor::GetUnit(*player, player->GetTarget());
        if (!target || !target->IsAlive() || !player->IsValidAttackTarget(target))
            return;

        // Cast Multi-Shot (triggered, no cost/CD)
        player->CastSpell(target, SPELL_MULTISHOT_R6, true);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_custom_hunt_barrage::OnApply,
            EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_custom_hunt_barrage::OnRemove,
            EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_custom_hunt_barrage::HandlePeriodic,
            EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  HUNTER SURV: Chance to drop explosion on damage (900566)
//  Proc passive: on ranged damage dealt, 15% chance to cast
//  Explosive Burst at the target location. 2s ICD.
// ============================================================
class spell_custom_hunt_surv_trap_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_hunt_surv_trap_proc);

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

        // Cast Explosive Burst AoE at target
        CastAnchoredBurst(player, target, SPELL_HUNT_SURV_TRAP_HELPER);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_hunt_surv_trap_proc::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  End Hunter section
// ============================================================

void AddHunterSpellsScripts()
{
    // Hunter Shared
    new custom_hunter_arrows_playerscript();
    RegisterSpellScript(spell_custom_hunt_multishot_aoe);

    // Hunter BM (pet damage/speed: shared minion auras)
    new custom_hunter_pet_aoe_unitscript();

    // Hunter MM
    RegisterSpellScript(spell_custom_hunt_autoshot_bounce);
    RegisterSpellScript(spell_custom_hunt_barrage);

    // Hunter Surv
    RegisterSpellScript(spell_custom_hunt_surv_trap_proc);
}
