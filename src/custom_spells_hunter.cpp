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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

        // Restore 1 ammo after each shot
        player->StoreNewItemInBestSlots(ammoId, 1);
    }
};

// ============================================================
//  HUNTER: Shared - Multi-Shot unlimited targets (900501)
//  Hooked on Multi-Shot (all ranks via -49048). After hitting
//  the main target, finds ALL additional enemies in 10yd and
//  deals the same damage.
// ============================================================
class spell_custom_hunt_multishot_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_hunt_multishot_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HUNT_MULTISHOT_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        // Find all enemies within 10yd of the main target
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(mainTarget, searcher, 10.0f);
        targets.remove(mainTarget);

        for (Unit* target : targets)
        {
            if (!target->IsAlive() || !caster->IsValidAttackTarget(target))
                continue;

            Unit::DealDamage(caster, target, static_cast<uint32>(damage),
                nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL);
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_hunt_multishot_aoe::HandleAfterHit);
    }
};

// ============================================================
//  HUNTER BM: Pet damage +50% and Pet attack speed +50% (900502/900503)
//  UnitScript: intercepts all pet damage and multiplies by 1.5x
//  if owner has the passive. Also modifies pet attack speed.
// ============================================================
class custom_hunter_pet_unitscript : public UnitScript
{
public:
    custom_hunter_pet_unitscript() : UnitScript("custom_hunter_pet_unitscript") {}

    void OnDamage(Unit* attacker, Unit* /*victim*/, uint32& damage) override
    {
        if (!attacker)
            return;

        // Only for creatures (pets)
        Creature* pet = attacker->ToCreature();
        if (!pet)
            return;

        Unit* owner = pet->GetOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Pet damage +50%
        if (player->HasAura(SPELL_HUNT_BM_PET_DMG_PASSIVE))
            damage = static_cast<uint32>(damage * 1.5f);
    }
};

// ============================================================
//  HUNTER BM: Pet attack speed +50% (900503)
//  PlayerScript: when player summons a pet or updates, apply
//  50% attack speed increase to their pet.
// ============================================================
class custom_hunter_pet_speed_playerscript : public PlayerScript
{
public:
    custom_hunter_pet_speed_playerscript() : PlayerScript("custom_hunter_pet_speed_playerscript") {}

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_HUNT_BM_PET_SPEED_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Throttle: only every 3 seconds
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        static std::unordered_map<ObjectGuid, uint32> s_lastCheck;
        ObjectGuid guid = player->GetGUID();
        if (s_lastCheck.count(guid) && (now - s_lastCheck[guid]) < 3)
            return;
        s_lastCheck[guid] = now;

        Pet* pet = player->GetPet();
        if (!pet || !pet->IsAlive())
            return;

        // Apply 50% faster attack speed (halve the attack time)
        uint32 baseAttack = pet->GetAttackTime(BASE_ATTACK);
        uint32 desired = static_cast<uint32>(pet->GetCreateAttackTime() * 0.5f);
        if (baseAttack != desired)
            pet->SetAttackTime(BASE_ATTACK, desired);
    }
};

// ============================================================
//  HUNTER BM: Pet chance to deal AoE damage (900504)
//  UnitScript: On pet melee hit, 15% chance to cast AoE helper.
// ============================================================
class custom_hunter_pet_aoe_unitscript : public UnitScript
{
public:
    custom_hunter_pet_aoe_unitscript() : UnitScript("custom_hunter_pet_aoe_unitscript") {}

    void OnDamage(Unit* attacker, Unit* victim, uint32& /*damage*/) override
    {
        if (!attacker || !victim || !victim->IsAlive())
            return;

        Creature* pet = attacker->ToCreature();
        if (!pet)
            return;

        Unit* owner = pet->GetOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HUNT_BM_PET_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // 15% chance
        if (!roll_chance_i(15))
            return;

        // Cast AoE damage helper centered on victim
        pet->CastSpell(victim, SPELL_HUNT_BM_PET_AOE_HELPER, true);
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        // Find up to 9 additional enemies within 10yd of target
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 10.0f);
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

            caster->CastCustomSpell(target, SPELL_HUNT_MM_BOUNCE_HELPER,
                &damage, nullptr, nullptr, true);
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Cast Explosive Burst AoE at target
        player->CastSpell(target, SPELL_HUNT_SURV_TRAP_HELPER, true);
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

    // Hunter BM
    new custom_hunter_pet_unitscript();
    new custom_hunter_pet_speed_playerscript();
    new custom_hunter_pet_aoe_unitscript();

    // Hunter MM
    RegisterSpellScript(spell_custom_hunt_autoshot_bounce);
    RegisterSpellScript(spell_custom_hunt_barrage);

    // Hunter Surv
    RegisterSpellScript(spell_custom_hunt_surv_trap_proc);
}
