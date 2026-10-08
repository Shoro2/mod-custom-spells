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
#include "MoveSpline.h"
#include "Pet.h"

// ============================================================
//  Hunter spells of the concept revision of 2026-10-08 (share-public
//  custom-spells/09-concept-revision-20261008.md). File-local: the shared
//  header keeps the ids of the earlier rounds. 900525-900532 are kept free
//  for the second-pet line, which another change builds.
// ============================================================
namespace
{
    enum HunterRevisionSpells
    {
        // ---- Hunter BM (900507-900524) ----
        SPELL_HUNT_BM_PET_HEALTH_PASSIVE    = 900507, // spell_pet_auras -> 900508
        SPELL_HUNT_BM_PET_HEALTH_AURA       = 900508, // on the pet: +100 % health

        // ---- Hunter MM (900537-900565) ----
        SPELL_HUNT_MM_MULTI_SERPENT_PASSIVE = 900537,
        SPELL_HUNT_MM_CHIMERA_AOE_PASSIVE   = 900538, // DBC only (jump targets 10)
        SPELL_HUNT_MM_CHIMERA_DMG_PASSIVE   = 900539, // DBC only
        SPELL_HUNT_MM_RUNNING_AIM_PASSIVE   = 900540,
        SPELL_HUNT_MM_RUNNING_AIM_BUFF      = 900541, // +50 % Aimed Shot per stack, 5

        // ---- Hunter Surv (900568-900599) ----
        SPELL_HUNT_SURV_ALTERNATE_PASSIVE   = 900568,
        SPELL_HUNT_SURV_BLADE_READINESS     = 900569, // +5 % melee weapon damage, 10
        SPELL_HUNT_SURV_SHOT_READINESS      = 900570, // +5 % ranged weapon damage, 10
        SPELL_HUNT_SURV_RAPTOR_AOE_PASSIVE  = 900571, // DBC only (jump targets 10)
        SPELL_HUNT_SURV_RAPTOR_DMG_PASSIVE  = 900572, // DBC only
    };

    // the second-pet feature's marker on every second pet (a Guardian, not a
    // Pet; custom_spells_second_pet.cpp): Beast Cleave counts its hits too
    constexpr uint32 SPELL_SECOND_PET_MARKER         = 900526;

    constexpr uint32 SPELL_SERPENT_STING_R1          = 1978;
    constexpr uint32 SPELL_EXPLOSIVE_TRAP_R1         = 13813;
    constexpr uint32 SPELL_EXPLOSIVE_TRAP_EFFECT_R1  = 13812;

    constexpr uint32 RUNNING_AIM_STACK_MS    = 5000;  // movement per stack
    constexpr uint8  RUNNING_AIM_MAX_STACKS  = 5;
    constexpr float  AIMED_PIERCE_HALF_WIDTH = 2.0f;  // + the struck unit's reach

    // "damage scales with the Paragon level": 666 + 5 x the true level, as
    // the DK concept spells (Bloodworm Burst 900307, Frozen Strike 900342)
    int32 ParagonDamage(Player* player)
    {
        return 666 + 5 * int32(GetParagonLevel(player));
    }

    // The highest rank of a spell chain the player knows (0 = none)
    uint32 HighestKnownRank(Player* player, uint32 firstRank)
    {
        uint32 known = 0;
        for (uint32 rank = firstRank; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
            if (player->HasSpell(rank))
                known = rank;
        return known;
    }

    // The Explosive Trap Effect of the highest Explosive Trap rank the hunter
    // knows; a hunter who never trained it gets the rank its level allows.
    uint32 SelectExplosiveTrapEffect(Player* player)
    {
        uint8 rank = 0;
        if (uint32 trap = HighestKnownRank(player, SPELL_EXPLOSIVE_TRAP_R1))
            rank = sSpellMgr->GetSpellRank(trap);

        if (!rank)
            for (uint32 trap = SPELL_EXPLOSIVE_TRAP_R1; trap;
                trap = sSpellMgr->GetNextSpellInChain(trap))
                if (SpellInfo const* info = sSpellMgr->GetSpellInfo(trap))
                    if (info->SpellLevel <= player->GetLevel())
                        rank = sSpellMgr->GetSpellRank(trap);

        return sSpellMgr->GetSpellWithRank(SPELL_EXPLOSIVE_TRAP_EFFECT_R1,
            std::max<uint8>(rank, 1));
    }
}

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
//  HUNTER BM: 900507 Pet Health +100% needs no code: the passive is a
//  dummy aura listed in spell_pet_auras (900507 -> 900508), so the
//  core's pet-aura path (AuraEffect::HandleAuraDummy -> Unit::AddPetAura,
//  Pet::CastPetAuras at every summon and load) keeps 900508 (aura 133
//  MOD_INCREASE_HEALTH_PERCENT +100, passive) on the hunter's Pet and
//  removes it with the passive. The core reaches Player::GetPet() only.
// ============================================================

// ============================================================
//  HUNTER BM: Pet chance to deal AoE damage (900504)
//  A white melee hit of the hunter's pet has a 15 % chance to set
//  off Beast Cleave (900505) around its target. Only white melee
//  hits count (DealDamage with DIRECT_DAMAGE): the old OnDamage hook
//  also fired on Beast Cleave's own damage, so on a big pack one
//  cleave could set off the next ones.
//  Revision 2026-10-08: the cleave deals 666 + 5 x the hunter's
//  Paragon level Physical (was a fixed 800-1,000), and the hits of the
//  hunter's second pet (a Guardian carrying 900526) count as well.
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
        if (!pet || !(pet->IsPet() || pet->HasAura(SPELL_SECOND_PET_MARKER)))
            return damage;

        Player* player = pet->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!player || !player->HasAura(SPELL_HUNT_BM_PET_AOE_PASSIVE))
            return damage;

        if (roll_chance_i(15))
            CastAnchoredBurst(pet, victim, SPELL_HUNT_BM_PET_AOE_HELPER,
                ParagonDamage(player));
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
//  HUNTER MM: 900537 Multi-Shot applies Serpent Sting
//  Hooked on Multi-Shot (all ranks via -2643): every enemy a Multi-Shot
//  hits (AfterHit runs for hits only, not for misses) gets the highest
//  Serpent Sting rank the hunter knows (a triggered cast: no cost, its
//  own hit roll and travel time). A hunter without Serpent Sting gets
//  nothing.
// ============================================================
class spell_custom_hunt_multishot_serpent : public SpellScript
{
    PrepareSpellScript(spell_custom_hunt_multishot_serpent);

    void HandleAfterHit()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!player || !target || !target->IsAlive() || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_HUNT_MM_MULTI_SERPENT_PASSIVE))
            return;

        if (uint32 sting = HighestKnownRank(player, SPELL_SERPENT_STING_R1))
            player->CastSpell(target, sting, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_hunt_multishot_serpent::HandleAfterHit);
    }
};

// ============================================================
//  HUNTER MM: 900540 Running Aim
//  Concept: while moving you get a stack every 5 seconds; each stack
//  increases Aimed Shot damage by 50 %, max 5; at 5 stacks Aimed Shot
//  pierces its target and damages all enemies in a line within its
//  range.
//  - Every 5 s of movement (client movement flags or a server spline,
//    not on a taxi; standing still pauses the count, it does not reset
//    it) the hunter gains a stack
//    of 900541 (SPELLMOD_DAMAGE +50 % on Aimed Shot per stack, 5
//    stacks, lasts until used).
//  - Every Aimed Shot uses the stacks up (AfterCast: its damage is
//    computed at launch, before that).
//  - At 5 stacks the shot also strikes every enemy in a line from the
//    hunter through its target up to Aimed Shot's range (2 yd either
//    side plus the enemy's reach, in line of sight): each one takes a
//    real Aimed Shot hit of its own (Spell::AddUnitTargetForScript in
//    OnCast, after target selection and before the launch).
// ============================================================
class RunningAimState : public DataMap::Base
{
public:
    uint32 MovingMs = 0;
};

class custom_hunter_running_aim_playerscript : public PlayerScript
{
public:
    custom_hunter_running_aim_playerscript()
        : PlayerScript("custom_hunter_running_aim_playerscript",
            { PLAYERHOOK_ON_UPDATE }) { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        // every player, every update: the movement test first, it is the
        // cheapest; a client sets the movement flags, a server-moved player
        // (a test bot, a charge) runs a spline instead
        if (!player
            || (!player->isMoving() && player->movespline->Finalized())
            || player->IsInFlight()
            || !g_CustomSpellsEnabled || !player->IsAlive()
            || !player->HasAura(SPELL_HUNT_MM_RUNNING_AIM_PASSIVE))
            return;

        RunningAimState* state = player->CustomData.GetDefault<
            RunningAimState>("mod_custom_spells_running_aim");
        if (player->GetAuraCount(SPELL_HUNT_MM_RUNNING_AIM_BUFF)
            >= RUNNING_AIM_MAX_STACKS)
        {
            state->MovingMs = 0;
            return;
        }

        state->MovingMs += diff;
        if (state->MovingMs < RUNNING_AIM_STACK_MS)
            return;

        state->MovingMs -= RUNNING_AIM_STACK_MS;
        player->CastSpell(player, SPELL_HUNT_MM_RUNNING_AIM_BUFF, true);
    }
};

class spell_custom_hunt_mm_aimed_shot : public SpellScript
{
    PrepareSpellScript(spell_custom_hunt_mm_aimed_shot);

    void HandleOnCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* mainTarget = GetExplTargetUnit();
        if (!player || !mainTarget || mainTarget == player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_HUNT_MM_RUNNING_AIM_PASSIVE))
            return;

        Aura* aim = player->GetAura(SPELL_HUNT_MM_RUNNING_AIM_BUFF);
        if (!aim || aim->GetStackAmount() < RUNNING_AIM_MAX_STACKS)
            return;

        float const range = GetSpellInfo()->GetMaxRange(false, player, GetSpell());
        float const angle = player->GetAngle(mainTarget);
        float const dirX = std::cos(angle);
        float const dirY = std::sin(angle);

        uint32 effectMask = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (GetSpellInfo()->Effects[i].IsEffect())
                effectMask |= 1 << i;

        std::list<Unit*> nearby;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(player, player, range);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(player, nearby, check);
        Cell::VisitObjects(player, searcher, range);

        for (Unit* unit : nearby)
        {
            if (unit == mainTarget || !unit->IsAlive()
                || !player->IsValidAttackTarget(unit)
                || !player->IsWithinLOSInMap(unit))
                continue;

            float const dx = unit->GetPositionX() - player->GetPositionX();
            float const dy = unit->GetPositionY() - player->GetPositionY();
            float const along = dx * dirX + dy * dirY;
            float const across = std::fabs(dy * dirX - dx * dirY);
            if (along <= 0.0f || along > range
                || across > AIMED_PIERCE_HALF_WIDTH + unit->GetCombatReach())
                continue;

            GetSpell()->AddUnitTargetForScript(unit, effectMask);
        }
    }

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (player && player->HasAura(SPELL_HUNT_MM_RUNNING_AIM_BUFF))
            player->RemoveAurasDueToSpell(SPELL_HUNT_MM_RUNNING_AIM_BUFF);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_custom_hunt_mm_aimed_shot::HandleOnCast);
        AfterCast += SpellCastFn(spell_custom_hunt_mm_aimed_shot::HandleAfterCast);
    }
};

// ============================================================
//  HUNTER SURV: Explosive trap on physical damage (900566)
//  Concept (revision 2026-10-08): chance to drop an explosion trap on
//  physical damage. Proc passive (spell_proc: melee and ranged, auto
//  attacks and abilities, Physical school, triggered attacks such as
//  Barrage included; 15 %, 2 s cooldown): the highest Explosive Trap
//  Effect rank the hunter knows (SelectExplosiveTrapEffect) goes off
//  at the struck target's spot, exactly as a trap GameObject fires it
//  (GameObject::CastSpell): a world trigger summoned there casts it for
//  its owner, so the damage, the burning ground (its DynamicObject
//  belongs to the hunter, Spell::EffectPersistentAA) and the combat
//  log are the hunter's. The trap hits its target too. Replaces the
//  ranged-only Explosive Burst 900567, which is no longer cast.
// ============================================================
class spell_custom_hunt_surv_trap_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_hunt_surv_trap_proc);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        Unit* target = eventInfo.GetActionTarget();
        return g_CustomSpellsEnabled && damageInfo && damageInfo->GetDamage()
            && (damageInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_NORMAL)
            && target && target->IsAlive();
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        Unit* target = eventInfo.GetActionTarget();
        if (!player || !target || !target->IsInWorld() || !target->IsInMap(player))
            return;

        SpellInfo const* trapEffect = sSpellMgr->GetSpellInfo(
            SelectExplosiveTrapEffect(player));
        if (!trapEffect)
            return;

        Creature* trigger = player->SummonTrigger(target->GetPositionX(),
            target->GetPositionY(), target->GetPositionZ(), 0.0f, 2000, true);
        if (!trigger)
            return;

        trigger->SetOwnerGUID(player->GetGUID());
        if (player->HasUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED))
            trigger->SetUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED);
        trigger->SetImmuneToNPC(false);
        trigger->SetInFront(target);
        trigger->CastSpell(target, trapEffect, true, nullptr, nullptr,
            player->GetGUID());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_hunt_surv_trap_proc::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_hunt_surv_trap_proc::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  HUNTER SURV: 900568 Blade and Shot
//  Concept: dealing physical damage with shot abilities increases your
//  melee damage by 5 % (10 stacks), dealing melee damage consumes it;
//  dealing melee damage increases your physical shot damage by 5 %
//  (10 stacks), dealing physical shot damage consumes it.
//  Proc passive (spell_proc: Physical damage done by melee and ranged
//  attacks and abilities, Auto Shot counted as a shot, triggered ones
//  included):
//  - a ranged hit uses up Shot Readiness 900570 and adds a stack of
//    Blade Readiness 900569;
//  - a melee hit uses up Blade Readiness and adds a stack of Shot
//    Readiness.
//  The two buffs are DBC auras 79 (+5 % Physical per stack) gated to a
//  melee weapon (900569) or a bow/gun/crossbow/thrown weapon (900570):
//  the core applies such an aura to the weapon damage of the attacks
//  that use a fitting weapon only (Unit::UpdateDamagePctDoneMods), so
//  900569 raises white melee hits and melee weapon abilities, 900570
//  Auto Shot and the weapon-based shots. A hit is boosted before the
//  proc that consumes the buff (the proc runs after the damage).
// ============================================================
class spell_custom_hunt_surv_blade_and_shot : public AuraScript
{
    PrepareAuraScript(spell_custom_hunt_surv_blade_and_shot);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        return g_CustomSpellsEnabled && damageInfo && damageInfo->GetDamage()
            && (damageInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_NORMAL);
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* hunter = GetTarget();
        bool const ranged = (eventInfo.GetTypeMask()
            & (PROC_FLAG_DONE_RANGED_AUTO_ATTACK | PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS)) != 0;
        uint32 const consumed = ranged ? SPELL_HUNT_SURV_SHOT_READINESS
            : SPELL_HUNT_SURV_BLADE_READINESS;
        uint32 const gained = ranged ? SPELL_HUNT_SURV_BLADE_READINESS
            : SPELL_HUNT_SURV_SHOT_READINESS;

        hunter->RemoveAurasDueToSpell(consumed);
        // the buff's weapon class gates its bonus, not the stack: a hunter
        // without a melee weapon still builds Blade Readiness
        hunter->CastSpell(hunter, gained, TriggerCastFlags(TRIGGERED_FULL_MASK
            | TRIGGERED_IGNORE_EQUIPPED_ITEM_REQUIREMENT), nullptr, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_hunt_surv_blade_and_shot::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_hunt_surv_blade_and_shot::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  HUNTER: DBC-only passives of the revision (spellmods in
//  mod_custom_spells_d_hunter_druid_rogue.sql):
//  900538 / 900539 Chimera Shot +9 targets (jump targets 10) / +50 %,
//  900571 / 900572 Raptor Strike +9 targets (jump targets 10) / +50 %.
// ============================================================

// ============================================================
//  End Hunter section
// ============================================================

void AddHunterSpellsScripts()
{
    // Hunter Shared
    new custom_hunter_arrows_playerscript();
    RegisterSpellScript(spell_custom_hunt_multishot_aoe);

    // Hunter BM (pet damage/speed: shared minion auras; pet health:
    // spell_pet_auras)
    new custom_hunter_pet_aoe_unitscript();

    // Hunter MM
    RegisterSpellScript(spell_custom_hunt_autoshot_bounce);
    RegisterSpellScript(spell_custom_hunt_barrage);
    RegisterSpellScript(spell_custom_hunt_multishot_serpent);
    new custom_hunter_running_aim_playerscript();
    RegisterSpellScript(spell_custom_hunt_mm_aimed_shot);

    // Hunter Surv
    RegisterSpellScript(spell_custom_hunt_surv_trap_proc);
    RegisterSpellScript(spell_custom_hunt_surv_blade_and_shot);
}
