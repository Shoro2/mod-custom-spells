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
//  Druid spells of the concept revision of 2026-10-08 (share-public
//  custom-spells/09-concept-revision-20261008.md). File-local: the
//  shared header keeps the ids of the earlier rounds.
// ============================================================
namespace
{
    enum DruidRevisionSpells
    {
        // ---- Balance (901006-901032) ----
        SPELL_BAL_LUNAR_FRENZY_PASSIVE     = 901006,
        SPELL_BAL_LUNAR_FRENZY_BUFF        = 901007, // +1 % damage and haste, 100

        // ---- Feral tank (901036-901048) ----
        SPELL_FERAL_BEAR_SWIPE_DMG_PASSIVE = 901036, // DBC only
        SPELL_FERAL_THORNS_WOUND_PASSIVE   = 901037,
        SPELL_FERAL_THORNS_WOUND_DAMAGE    = 901038,
        SPELL_FERAL_URSINE_PASSIVE         = 901039,
        SPELL_FERAL_URSINE_BUFF            = 901040, // +1 % maximum health, 100
        SPELL_FERAL_URSINE_QUAKE           = 901041,

        // ---- Feral DPS (901052-901065) ----
        SPELL_FERAL_CAT_SWIPE_DMG_PASSIVE  = 901052, // DBC only
        SPELL_FERAL_FRONTAL_PASSIVE        = 901053, // custom_spells_rogue.cpp
        SPELL_FERAL_BERSERK_PASSIVE        = 901054, // custom_spells_rogue.cpp
        SPELL_FERAL_BERSERK_SPEED          = 901055,
        SPELL_FERAL_BERSERK_BURST          = 901056,
    };

    constexpr uint32 SPELL_DRUID_STARFIRE_R1 = 2912;
    constexpr uint32 SPELL_DRUID_BARKSKIN    = 22812;
    constexpr uint32 SPELL_DRUID_BERSERK     = 50334;

    constexpr uint8  LUNAR_FRENZY_MAX_STACKS = 100;
    constexpr uint8  URSINE_MAX_STACKS       = 100;
    constexpr uint32 URSINE_RAGE_PER_QUAKE   = 1000;  // 100 rage, stored in tenths
    constexpr uint32 THORNS_WOUND_CD_MS      = 5000;  // per target

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

    // Adds a stack to a stacking buff the unit keeps on itself and renews
    // its duration WITHOUT restarting its periodic effect. A new cast on
    // the buff restarts the timer of every periodic effect that is not a
    // damage over time (Aura::ModStackAmount -> RefreshTimers ->
    // AuraEffect::CalculatePeriodic): a buff renewed more often than its
    // period (a tank hit several times a second, kills in quick
    // succession) would never tick.
    void AddStackKeepTicking(Unit* unit, uint32 spellId, AuraEffect const* triggeredBy)
    {
        Aura* aura = unit->GetAura(spellId, unit->GetGUID());
        if (!aura)
        {
            unit->CastSpell(unit, spellId, true, nullptr, triggeredBy);
            return;
        }

        if (aura->GetStackAmount() < aura->GetSpellInfo()->CalcMaxAuraStacks(unit))
            aura->SetStackAmount(aura->GetStackAmount() + 1);
        aura->RefreshDuration();
    }
}

// ============================================================
//  DRUID BALANCE: Spell damage reduces Starfall CD (901004)
//  Proc passive: on spell damage dealt, reduce Starfall CD
//  by 1 second. 100% chance, no ICD (CD reduction per hit).
// ============================================================
class spell_custom_bal_sf_cd_reduce : public AuraScript
{
    PrepareAuraScript(spell_custom_bal_sf_cd_reduce);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        // Only on spell damage (not melee)
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        // Must be druid spell family
        return spellInfo->SpellFamilyName == SPELLFAMILY_DRUID_ID;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
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

        // Reduce Starfall cooldown by 1 second
        player->ModifySpellCooldown(SPELL_STARFALL_R2, -1000);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_bal_sf_cd_reduce::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_bal_sf_cd_reduce::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  DRUID FERAL: Swipe Bear applies bleed (901033)
//  Hooked on Swipe Bear (all ranks via -48562). After hitting
//  each target, applies a bleed DoT.
// ============================================================
class spell_custom_feral_bear_swipe_bleed : public SpellScript
{
    PrepareSpellScript(spell_custom_feral_bear_swipe_bleed);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_FERAL_BEAR_SWIPE_BLEED))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Apply bleed DoT to target
        caster->CastSpell(target, SPELL_FERAL_BEAR_SWIPE_BLEED_DOT, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_feral_bear_swipe_bleed::HandleAfterHit);
    }
};

// ============================================================
//  DRUID FERAL: Swipe Cat applies bleed (901049)
//  Hooked on Swipe Cat (62078). After hitting each target,
//  applies a bleed DoT.
// ============================================================
class spell_custom_feral_cat_swipe_bleed : public SpellScript
{
    PrepareSpellScript(spell_custom_feral_cat_swipe_bleed);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_FERAL_CAT_SWIPE_BLEED))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Apply bleed DoT to target
        caster->CastSpell(target, SPELL_FERAL_CAT_SWIPE_BLEED_DOT, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_feral_cat_swipe_bleed::HandleAfterHit);
    }
};

// ============================================================
//  DRUID RESTO: HoTs chance to summon Force of Nature (901066)
//  Proc passive: on periodic healing done, 5% chance to
//  summon a custom Treant at the healed target's location.
// ============================================================
class spell_custom_drst_hot_treant : public AuraScript
{
    PrepareAuraScript(spell_custom_drst_hot_treant);

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
        if (!target)
            target = caster;

        // Summon a treant at the target's location (30s duration)
        if (Creature* treant = player->SummonCreature(NPC_CUSTOM_TREANT,
            target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
            player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000))
        {
            treant->SetOwnerGUID(player->GetGUID());
            treant->SetCreatorGUID(player->GetGUID());
            treant->SetFaction(player->GetFaction());

            // If there's a hostile target nearby, attack it
            Unit* victim = player->GetVictim();
            if (victim && victim->IsAlive())
            {
                treant->Attack(victim, true);
                treant->GetMotionMaster()->MoveChase(victim);
            }
            else
            {
                treant->GetMotionMaster()->MoveFollow(player, 3.0f, M_PI / 4);
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_drst_hot_treant::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  DRUID RESTO: 901067 Summons scale with healing power lives in the
//  shared minion manager (custom_spells_global.cpp, helper 901074).
// ============================================================

// ============================================================
//  DRUID RESTO: Summons heal on death/despawn (901068)
//  UnitScript: when a creature with a druid owner dies,
//  heal all nearby allies for a burst amount.
// ============================================================
class custom_druid_summon_heal_unitscript : public UnitScript
{
public:
    custom_druid_summon_heal_unitscript() : UnitScript("custom_druid_summon_heal_unitscript") {}

    void OnUnitDeath(Unit* unit, Unit* /*killer*/) override
    {
        if (!unit)
            return;

        Creature* summon = unit->ToCreature();
        if (!summon)
            return;

        Unit* owner = summon->GetOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_DRST_SUMMON_HEAL_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Cast heal helper on self (AoE heal around summon's position)
        summon->CastSpell(summon, SPELL_DRST_TREANT_HEAL_HELPER, true);
    }
};

// The despawn half of 901068: the shared minion manager
// (custom_spells_global.cpp) gives every timed summon of a marked druid
// the fuse 901075, which ends 0.3 s before the summon would despawn;
// its expiry heals (death removes the fuse without the expiry mode, so
// a killed summon heals once, through the death path above).
class spell_custom_drst_parting_bloom_fuse : public AuraScript
{
    PrepareAuraScript(spell_custom_drst_parting_bloom_fuse);

    void HandleRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        Unit* summon = GetTarget();
        Player* owner = summon->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!summon->IsAlive() || !owner || !g_CustomSpellsEnabled
            || !owner->HasAura(SPELL_DRST_SUMMON_HEAL_PASSIVE))
            return;

        summon->CastSpell(summon, SPELL_DRST_TREANT_HEAL_HELPER, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_drst_parting_bloom_fuse::HandleRemove, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// ============================================================
//  DRUID BALANCE: 901005 Starfall stacks up to 10
//  Concept: "starfall now stacks up to 10". Casting Starfall while it
//  runs adds a stack (the recast refreshes it; the stack count is set
//  right after), and every second each stack calls down its own
//  volley: the aura's periodic trigger (53198 for rank 4) fires once
//  more per extra stack.
// ============================================================
class spell_custom_bal_starfall_stack : public SpellScript
{
    PrepareSpellScript(spell_custom_bal_starfall_stack);

    uint8 _stacks = 0;

    void HandleBeforeCast()
    {
        Aura* starfall = GetCaster()->GetAura(GetSpellInfo()->Id,
            GetCaster()->GetGUID());
        _stacks = starfall ? starfall->GetStackAmount() : 0;
    }

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || !_stacks || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_BAL_SF_STACKS_PASSIVE))
            return;

        if (Aura* starfall = player->GetAura(GetSpellInfo()->Id, player->GetGUID()))
            starfall->SetStackAmount(std::min<uint8>(_stacks + 1, 10));
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_custom_bal_starfall_stack::HandleBeforeCast);
        AfterCast += SpellCastFn(spell_custom_bal_starfall_stack::HandleAfterCast);
    }
};

class spell_custom_bal_starfall_stack_volley : public AuraScript
{
    PrepareAuraScript(spell_custom_bal_starfall_stack_volley);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        uint8 const stacks = GetStackAmount();
        Unit* target = GetTarget();
        if (stacks < 2 || !g_CustomSpellsEnabled
            || !target->HasAura(SPELL_BAL_SF_STACKS_PASSIVE))
            return;

        uint32 const volley = GetSpellInfo()->Effects[EFFECT_0].TriggerSpell;
        for (uint8 i = 1; i < stacks; ++i)
            target->CastSpell(target, volley, true, nullptr, aurEff);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_bal_starfall_stack_volley::HandlePeriodic, EFFECT_0,
            SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// ============================================================
//  DRUID RESTO: Thorns has chance to cast Rejuv (901069)
//  Concept: "thorns has a chance to cast rejuv on targets". A melee
//  hit against any unit that carries the druid's Thorns (damage
//  shield, druid family flags0 0x100) has a 20 % chance to make the
//  druid cast Rejuvenation on that unit, at most once every 3 sec
//  per unit. (The old hook only covered the druid itself, on any
//  damage, while it had anyone's Thorns.)
// ============================================================
class ThornsRejuvCd : public DataMap::Base
{
public:
    uint32 NextAllowedMs = 0;
};

class custom_druid_thorns_rejuv_unitscript : public UnitScript
{
public:
    custom_druid_thorns_rejuv_unitscript()
        : UnitScript("custom_druid_thorns_rejuv_unitscript") { }

    uint32 DealDamage(Unit* attacker, Unit* victim, uint32 damage,
        DamageEffectType damagetype) override
    {
        if (damagetype != DIRECT_DAMAGE || !attacker || !victim
            || !victim->IsAlive() || !g_CustomSpellsEnabled)
            return damage;

        AuraEffect const* thorns = victim->GetAuraEffect(
            SPELL_AURA_DAMAGE_SHIELD, SPELLFAMILY_DRUID, SPELLFAMILYFLAG_THORNS,
            0, 0);
        if (!thorns)
            return damage;

        Unit* druid = thorns->GetCaster();
        if (!druid || !druid->IsAlive()
            || !druid->HasAura(SPELL_DRST_THORNS_REJUV_PASSIVE))
            return damage;

        uint32 const now = GameTime::GetGameTimeMS().count();
        ThornsRejuvCd* cd = victim->CustomData.GetDefault<ThornsRejuvCd>(
            "mod_custom_spells_thorns_rejuv_cd");
        if (now < cd->NextAllowedMs || !roll_chance_i(20))
            return damage;
        cd->NextAllowedMs = now + 3000;

        druid->CastSpell(victim, SPELL_REJUV_R15, true);
        return damage;
    }
};

// ============================================================
//  DRUID RESTO: HoTs tick 2x fast + 2x duration (901071)
//  This is a DBC-only approach where possible.
//  However, combining double tick speed with double duration
//  needs two separate DBC auras. We use marker + DBC auras:
//  - One aura halves tick interval (ADD_PCT_MODIFIER on SPELL_AURA_PERIODIC)
//  - One aura doubles duration (ADD_PCT_MODIFIER on duration)
//  Both handled via DBC - no C++ needed for this spell.
// ============================================================
// (No C++ class needed - pure DBC passive)

// ============================================================
//  DRUID RESTO: 901072 Mana Regen is shared with the shaman and the
//  mage: custom_mana_regen_playerscript (custom_spells_global.cpp)
// ============================================================

// ============================================================
//  End Druid section
// ============================================================

// ============================================================
//  DRUID FERAL TANK: 901035 Maul on a bleeding target
//  Concept: casting Maul on a bleeding target deals extra damage.
//  Hooked on Maul (all ranks via -6807): +50 % damage against a
//  target in the bleeding aura state (any bleed, the druid's own
//  Lacerate, Rake, Rip or Swipe bleed 901034 included).
// ============================================================
class spell_custom_feral_maul_bleed : public SpellScript
{
    PrepareSpellScript(spell_custom_feral_maul_bleed);

    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !g_CustomSpellsEnabled)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->HasAura(SPELL_FERAL_BEAR_MAUL_BLEED_PASSIVE))
            return;

        if (!target->HasAuraState(AURA_STATE_BLEEDING))
            return;

        SetHitDamage(GetHitDamage() * 3 / 2);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_custom_feral_maul_bleed::HandleOnHit);
    }
};

// ============================================================
//  DRUID BALANCE: 901002 Starfall area +50 % (revision 2026-10-08,
//  was "+9 targets") is DBC only: SPELLMOD_RADIUS +50 % on the volley
//  trigger's 30-yd search (53196-53198, flags2 0x100) and on the stars'
//  5-yd splash (50294, 53188-53190, flags1 0x800000).
// ============================================================

// ============================================================
//  DRUID BALANCE: 901006 Lunar Frenzy
//  Concept: while in Moonkin Form killing an enemy enrages you,
//  increasing your damage and haste by 1 % (100 stacks); at 100 stacks
//  a free instant Starfire at a random enemy in range every 3 sec.
//  - Kill proc (spell_proc PROC_FLAG_KILL; any killing blow of the
//    druid, a critter included): in Moonkin Form a stack of 901007
//    (+1 % damage done, +1 % casting speed per stack, 100 stacks, 60 s,
//    each kill renews it without restarting its 3-s tick).
//  - 901007 ticks every 3 s: at 100 stacks, in combat, in caster or
//    Moonkin Form, the highest Starfire rank the druid knows goes off,
//    triggered (instant, free, it does not touch the druid's own cast),
//    at a random enemy within Starfire's range, in line of sight and in
//    front of the druid (a triggered cast still checks facing).
// ============================================================
class spell_custom_bal_lunar_frenzy : public AuraScript
{
    PrepareAuraScript(spell_custom_bal_lunar_frenzy);

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* druid = GetTarget();
        if (!g_CustomSpellsEnabled || !druid->IsAlive()
            || druid->GetShapeshiftForm() != FORM_MOONKIN)
            return;

        AddStackKeepTicking(druid, SPELL_BAL_LUNAR_FRENZY_BUFF, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_bal_lunar_frenzy::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_custom_bal_lunar_frenzy_volley : public AuraScript
{
    PrepareAuraScript(spell_custom_bal_lunar_frenzy_volley);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Player* druid = GetTarget()->ToPlayer();
        if (!druid || GetStackAmount() < LUNAR_FRENZY_MAX_STACKS
            || !g_CustomSpellsEnabled || !druid->IsAlive() || !druid->IsInCombat()
            || !druid->HasAura(SPELL_BAL_LUNAR_FRENZY_PASSIVE))
            return;

        ShapeshiftForm const form = druid->GetShapeshiftForm();
        if (form != FORM_NONE && form != FORM_MOONKIN)
            return;

        uint32 const starfire = HighestKnownRank(druid, SPELL_DRUID_STARFIRE_R1);
        SpellInfo const* starfireInfo = starfire
            ? sSpellMgr->GetSpellInfo(starfire) : nullptr;
        if (!starfireInfo)
            return;

        float const range = starfireInfo->GetMaxRange(false, druid);
        std::list<Unit*> nearby;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(druid, druid, range);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(druid, nearby, check);
        Cell::VisitObjects(druid, searcher, range);

        std::vector<Unit*> candidates;
        for (Unit* unit : nearby)
            if (unit->IsAlive() && druid->IsValidAttackTarget(unit)
                && druid->HasInArc(float(M_PI), unit)
                && druid->IsWithinLOSInMap(unit))
                candidates.push_back(unit);

        if (candidates.empty())
            return;

        druid->CastSpell(Acore::Containers::SelectRandomContainerElement(candidates),
            starfire, true, nullptr, aurEff);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_bal_lunar_frenzy_volley::HandlePeriodic, EFFECT_2,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  DRUID FERAL TANK: 901036 Swipe (Bear) +50 % is DBC only.
// ============================================================

// ============================================================
//  DRUID FERAL TANK: 901037 Thorns: Open Wounds
//  Concept: bleeding targets that get damaged by Thorns open their
//  wound, dealing physical damage dependent on the Paragon level to
//  them, 5 sec cooldown per target.
//  Thorns strikes back when a white melee hit lands on its carrier
//  (Unit::DealMeleeDamage, the damage-shield pass). This hook sees that
//  hit (DealDamage with DIRECT_DAMAGE and damage): when the attacker is
//  bleeding (AURA_STATE_BLEEDING) and the victim carries Thorns of a
//  marked druid, the druid deals 901038 to the attacker - 666 + 5 x its
//  Paragon level Physical, a bleed (armor does not reduce it) - at most
//  once every 5 s per attacker. The cast waits for the druid's next
//  update: the hook runs inside the attacker's own melee swing.
// ============================================================
class ThornsWoundCd : public DataMap::Base
{
public:
    uint32 NextAllowedMs = 0;
};

class custom_druid_thorns_wound_unitscript : public UnitScript
{
public:
    custom_druid_thorns_wound_unitscript()
        : UnitScript("custom_druid_thorns_wound_unitscript") { }

    uint32 DealDamage(Unit* attacker, Unit* victim, uint32 damage,
        DamageEffectType damagetype) override
    {
        if (damagetype != DIRECT_DAMAGE || !damage || !attacker || !victim
            || !attacker->IsAlive() || !g_CustomSpellsEnabled
            || !attacker->HasAuraState(AURA_STATE_BLEEDING))
            return damage;

        AuraEffect const* thorns = victim->GetAuraEffect(
            SPELL_AURA_DAMAGE_SHIELD, SPELLFAMILY_DRUID, SPELLFAMILYFLAG_THORNS,
            0, 0);
        if (!thorns)
            return damage;

        Unit* caster = thorns->GetCaster();
        Player* druid = caster ? caster->ToPlayer() : nullptr;
        if (!druid || !druid->IsAlive()
            || !druid->HasAura(SPELL_FERAL_THORNS_WOUND_PASSIVE))
            return damage;

        uint32 const now = GameTime::GetGameTimeMS().count();
        ThornsWoundCd* cd = attacker->CustomData.GetDefault<ThornsWoundCd>(
            "mod_custom_spells_thorns_wound_cd");
        if (now < cd->NextAllowedMs)
            return damage;
        cd->NextAllowedMs = now + THORNS_WOUND_CD_MS;

        ObjectGuid const attackerGuid = attacker->GetGUID();
        int32 const amount = ParagonDamage(druid);
        druid->m_Events.AddEventAtOffset([druid, attackerGuid, amount]()
        {
            Unit* bleeding = ObjectAccessor::GetUnit(*druid, attackerGuid);
            if (!bleeding || !bleeding->IsAlive() || !druid->IsAlive()
                || !druid->IsValidAttackTarget(bleeding))
                return;

            int32 bp0 = amount;
            druid->CastCustomSpell(bleeding, SPELL_FERAL_THORNS_WOUND_DAMAGE,
                &bp0, nullptr, nullptr, true);
        }, 1ms);
        return damage;
    }
};

// ============================================================
//  DRUID FERAL TANK: 901039 Ursine Bulwark
//  Concept: getting hit while in Bear Form increases your health by 1 %
//  (100 stacks); while at 100 stacks you have Barkskin active and
//  spending 100 rage triggers an AoE around you (Paragon damage).
//  - Hits taken (spell_proc: melee, ranged and harmful spells that hit,
//    triggered ones included) in Bear or Dire Bear Form add a stack of
//    901040 (+1 % maximum health per stack, 100 stacks, 30 s, each hit
//    renews it without restarting its 1-s tick).
//  - 901040 ticks every second: at 100 stacks it keeps Barkskin (22812)
//    on the druid, its duration lifted to what is left of 901040.
//  - At 100 stacks every 100 rage the druid's own abilities cost (a
//    triggered cast pays none) sets off 901041: 666 + 5 x the Paragon
//    level Physical to every enemy within 8 yd of the druid.
// ============================================================
class spell_custom_feral_ursine_bulwark : public AuraScript
{
    PrepareAuraScript(spell_custom_feral_ursine_bulwark);

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* druid = GetTarget();
        ShapeshiftForm const form = druid->GetShapeshiftForm();
        if (!g_CustomSpellsEnabled || !druid->IsAlive()
            || (form != FORM_BEAR && form != FORM_DIREBEAR))
            return;

        AddStackKeepTicking(druid, SPELL_FERAL_URSINE_BUFF, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_feral_ursine_bulwark::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_custom_feral_ursine_barkskin : public AuraScript
{
    PrepareAuraScript(spell_custom_feral_ursine_barkskin);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* druid = GetTarget();
        if (GetStackAmount() < URSINE_MAX_STACKS || !g_CustomSpellsEnabled
            || !druid->IsAlive() || !druid->HasAura(SPELL_FERAL_URSINE_PASSIVE))
            return;

        int32 const left = GetDuration();
        Aura* barkskin = druid->GetAura(SPELL_DRUID_BARKSKIN, druid->GetGUID());
        if (!barkskin)
            barkskin = druid->AddAura(SPELL_DRUID_BARKSKIN, druid);
        if (barkskin && left > 0 && barkskin->GetDuration() < left)
        {
            barkskin->SetMaxDuration(left);
            barkskin->SetDuration(left);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_feral_ursine_barkskin::HandlePeriodic, EFFECT_1,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

class UrsineRageState : public DataMap::Base
{
public:
    uint32 Spent = 0;
};

class custom_druid_ursine_rage_playerscript : public PlayerScript
{
public:
    custom_druid_ursine_rage_playerscript()
        : PlayerScript("custom_druid_ursine_rage_playerscript",
            { PLAYERHOOK_ON_SPELL_CAST }) { }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!player || !spell || spell->IsTriggered() || !g_CustomSpellsEnabled
            || spell->GetSpellInfo()->PowerType != POWER_RAGE
            || spell->GetPowerCost() <= 0
            || !player->HasAura(SPELL_FERAL_URSINE_PASSIVE))
            return;

        Aura* bulwark = player->GetAura(SPELL_FERAL_URSINE_BUFF);
        if (!bulwark || bulwark->GetStackAmount() < URSINE_MAX_STACKS)
            return;

        UrsineRageState* state = player->CustomData.GetDefault<
            UrsineRageState>("mod_custom_spells_ursine_rage");
        state->Spent += uint32(spell->GetPowerCost());
        while (state->Spent >= URSINE_RAGE_PER_QUAKE)
        {
            state->Spent -= URSINE_RAGE_PER_QUAKE;
            int32 amount = ParagonDamage(player);
            player->CastCustomSpell(player, SPELL_FERAL_URSINE_QUAKE, &amount,
                nullptr, nullptr, true);
        }
    }
};

// ============================================================
//  DRUID FERAL DPS: 901052 Swipe (Cat) +50 % is DBC only.
//  901053 Frontal Assault (the "must be behind the target" abilities -
//  Shred, Ravage - from the front) and 901054 Berserk: Unleashed (+100 %
//  movement speed 901055 while Berserk lasts, and every 5-combo-point
//  finisher meanwhile unleashes 901056: 666 + 5 x the Paragon level
//  Physical to every enemy within 8 yd) use the shared implementations
//  in custom_spells_rogue.cpp, registered below.
// ============================================================

void AddDruidSpellsScripts()
{
    // Druid Balance
    RegisterSpellScript(spell_custom_bal_sf_cd_reduce);
    RegisterSpellScript(spell_custom_bal_starfall_stack);
    RegisterSpellScript(spell_custom_bal_starfall_stack_volley);
    RegisterSpellScript(spell_custom_bal_lunar_frenzy);
    RegisterSpellScript(spell_custom_bal_lunar_frenzy_volley);

    // Druid Feral
    RegisterSpellScript(spell_custom_feral_bear_swipe_bleed);
    RegisterSpellScript(spell_custom_feral_cat_swipe_bleed);
    RegisterSpellScript(spell_custom_feral_maul_bleed);
    new custom_druid_thorns_wound_unitscript();
    RegisterSpellScript(spell_custom_feral_ursine_bulwark);
    RegisterSpellScript(spell_custom_feral_ursine_barkskin);
    new custom_druid_ursine_rage_playerscript();
    AddFrontalAttackMarker(SPELL_FERAL_FRONTAL_PASSIVE);
    AddComboFrenzyRule(SPELL_DRUID_BERSERK, SPELL_FERAL_BERSERK_PASSIVE,
        SPELL_FERAL_BERSERK_SPEED, SPELL_FERAL_BERSERK_BURST);

    // Druid Resto
    RegisterSpellScript(spell_custom_drst_hot_treant);
    new custom_druid_summon_heal_unitscript();
    RegisterSpellScript(spell_custom_drst_parting_bloom_fuse);
    new custom_druid_thorns_rejuv_unitscript();
}
