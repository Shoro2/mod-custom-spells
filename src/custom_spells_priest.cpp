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
#include "Containers.h"
#include "TemporarySummon.h"

// ============================================================
//  Concept revision of 2026-10-08 (share-public custom-spells/09):
//  the ids of the new priest spells and creatures. File-local on
//  purpose - custom_spells_common.h is not touched in this round.
// ============================================================
namespace
{
    // Holy
    constexpr uint32 SPELL_PRI_HOLY_REDEMPTION        = 900938; // marker
    constexpr uint32 SPELL_PRI_HOLY_REDEMPTION_BUFF   = 900939; // +50 % x3
    constexpr uint32 SPELL_PRI_HOLY_REDEMPTION_CD     = 900940; // 5 min
    constexpr uint32 SPELL_PRI_HOLY_NOVA_KINDLE       = 900942; // marker
    constexpr uint32 SPELL_PRI_HOLY_KINDLED_FIRE      = 900943; // +50 % / stack
    constexpr uint32 NPC_REDEMPTION_GUARDIAN          = 900964;

    // Shadow
    constexpr uint32 SPELL_PRI_SHADOW_SOUL_FEAST      = 900972; // marker
    constexpr uint32 SPELL_PRI_SHADOW_SOUL_FEAST_BUFF = 900973; // +1 % Shadow
    constexpr uint32 SPELL_PRI_SHADOW_FREE_FLAY       = 900974;
    constexpr uint32 SPELL_PRI_SHADOW_TENTACLES       = 900975; // marker
    constexpr uint32 SPELL_PRI_SHADOW_TENTACLE_LASH   = 900976;
    constexpr uint32 NPC_LESSER_TENTACLE              = 900999;

    // stock spells
    constexpr uint32 SPELL_SHADOWFORM                 = 15473;
    constexpr uint32 SPELL_SOR_IMMUNITY               = 62371; // see 27795
    constexpr uint32 SPELL_LIGHTWELL_CHARGES          = 59907;
    constexpr uint32 SPELL_LIGHTWELL_RENEW_R1         = 7001;

    // "dmg dependent on paragon lvl": the DK concept's formula
    constexpr int32 PARAGON_DAMAGE_BASE               = 666;
    constexpr int32 PARAGON_DAMAGE_PER_LEVEL          = 5;

    constexpr float HOLY_FIRE_PROC_RANGE              = 30.0f;
    constexpr float ATONEMENT_RANGE                   = 30.0f;
    constexpr uint32 ATONEMENT_PCT                    = 10;
    constexpr float LIGHTWELL_RANGE                   = 20.0f;
    constexpr float LIGHTWELL_HEAL_BELOW_PCT          = 90.0f;
    constexpr int32 REDEMPTION_EXTRA_MS               = 10000;
    constexpr uint32 REDEMPTION_GUARDIAN_GRACE_MS     = 1000;
    constexpr int32 REDEMPTION_HEALTH_PCT             = 50;
    constexpr uint32 REDEMPTION_MANA_PCT              = 50;
    constexpr uint8 SOUL_FEAST_MAX_STACKS             = 100;
    constexpr float FREE_FLAY_RANGE                   = 30.0f;
    constexpr uint32 TENTACLE_LIFETIME_MS             = 15000;
    constexpr uint32 TENTACLE_LIMIT                   = 10;
    constexpr uint32 TENTACLE_FIRST_LASH_MS           = 500;
    constexpr uint32 TENTACLE_LASH_INTERVAL_MS        = 2000;
    constexpr float TENTACLE_LASH_RANGE               = 15.0f;

    int32 ParagonDamage(Player* player)
    {
        return PARAGON_DAMAGE_BASE
            + PARAGON_DAMAGE_PER_LEVEL * int32(GetParagonLevel(player));
    }

    // A random living enemy within `range` of `origin` that is already in
    // combat (an idle mob is never pulled), that the player may attack and
    // sees.
    Unit* SelectRandomEngagedEnemy(WorldObject* origin, Player* player,
        float range)
    {
        std::list<Unit*> enemies;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(origin, player, range);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(origin, enemies, check);
        Cell::VisitObjects(origin, searcher, range);

        enemies.remove_if([player](Unit* enemy)
        {
            return !enemy->IsAlive() || !enemy->IsInCombat()
                || !player->IsValidAttackTarget(enemy)
                || !player->IsWithinLOSInMap(enemy);
        });

        if (enemies.empty())
            return nullptr;

        return Acore::Containers::SelectRandomContainerElement(enemies);
    }

    // The priest and the players of his group, each once.
    std::vector<Player*> PriestAndGroup(Player* priest)
    {
        std::vector<Player*> players = { priest };
        if (Group* group = priest->GetGroup())
            for (GroupReference* ref = group->GetFirstMember(); ref;
                ref = ref->next())
                if (Player* member = ref->GetSource())
                    if (member != priest)
                        players.push_back(member);
        return players;
    }

    // The Lightwell Renew rank of a Lightwell (spell_pri_lightwell), or 0.
    uint32 LightwellRenewFor(uint32 entry)
    {
        switch (entry)
        {
            case 31897: return 7001;    // Lightwell rank 1
            case 31896: return 27873;
            case 31895: return 27874;
            case 31894: return 28276;
            case 31893: return 48084;
            case 31883: return 48085;   // rank 6
            default:    return 0;
        }
    }

    // Spirit of Redemption (900938) between the death and the end of the
    // spirit form, per priest.
    class RedemptionState : public DataMap::Base
    {
    public:
        GuidVector Enemies;     // fought the priest when he died
        ObjectGuid Guardian;
        bool Empowered = false; // this spirit form is the passive's
        bool Resurrect = false; // it ran out: no death, the priest returns
    };

    RedemptionState* GetRedemption(Player* priest)
    {
        return priest->CustomData.GetDefault<RedemptionState>(
            "mod_custom_spells_redemption");
    }
}

// ============================================================
//  PRIEST DISCIPLINE
// ============================================================

// ============================================================
//  900900: Shields explode on breaking/fading
//  Hooked on Power Word: Shield (all ranks via -17).
//  AfterEffectRemove: if the shield expires (fade) or is consumed
//  by damage, the explosion 900903 goes off at the shielded unit's
//  spot. Concept revision 2026-10-08 ("dmg depended on paragon
//  lvl"): the explosion deals 666 + 5 x the priest's Paragon level
//  Holy damage to every enemy within 10 yd (was: the shield's
//  absorption, dealt directly), broken or faded alike.
// ============================================================
class spell_custom_pri_shield_explode : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_shield_explode);

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
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

        if (!g_CustomSpellsEnabled)
            return;

        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();

        // Only explode on expire (fade) or absorb-depleted (enemy spell broke it)
        if (removeMode != AURA_REMOVE_BY_EXPIRE &&
            removeMode != AURA_REMOVE_BY_ENEMY_SPELL &&
            removeMode != AURA_REMOVE_BY_DEFAULT)
            return;

        SpellInfo const* explosion = sSpellMgr->GetSpellInfo(
            SPELL_PRI_DISC_EXPLODE_HELPER);
        if (!explosion)
            return;

        // dest-centred helper (10 yd), aimed at the shielded unit's spot
        SpellCastTargets targets;
        targets.SetDst(*target);
        CustomSpellValues values;
        values.AddSpellMod(SPELLVALUE_BASE_POINT0, ParagonDamage(player));
        player->CastSpell(targets, explosion, &values, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_pri_shield_explode::HandleRemove,
            EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

// ============================================================
//  900902: Weakened Soul lasts only 5 sec
//  Concept: "weakened soul only 5 sec cd". AuraScript on Weakened
//  Soul (6788) itself: whoever's Power Word: Shield put it on
//  whichever unit, when its caster has the passive it lasts 5 sec.
//  (The old hook halved the CASTER's own Weakened Soul after the
//  cast - a shield on anyone else kept the full 15 sec.)
// ============================================================
class spell_custom_pri_weakened_soul_cd : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_weakened_soul_cd);

    void HandleApply(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster || !g_CustomSpellsEnabled
            || !caster->HasAura(SPELL_PRI_DISC_WEAKENED_SOUL_CD))
            return;

        Aura* weakenedSoul = GetAura();
        if (weakenedSoul->GetMaxDuration() > WEAKENED_SOUL_SHORT_MS)
        {
            weakenedSoul->SetMaxDuration(WEAKENED_SOUL_SHORT_MS);
            weakenedSoul->SetDuration(WEAKENED_SOUL_SHORT_MS);
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_custom_pri_weakened_soul_cd::HandleApply, EFFECT_0,
            SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
    }
};

// ============================================================
//  900904: Atonement (2026-10-08)
//  Concept: "heals the lowest % health ally in 30y range by 10% of
//  your damage". Proc aura (spell_proc: melee, ranged, spell and
//  periodic damage done, triggered damage included): 10 % of each hit
//  (after absorbs) heals the injured player of the priest's group -
//  the priest included - with the lowest health percentage within
//  30 yd of the priest. Logged under 900904, no healing bonuses on
//  top; nobody injured, no heal.
// ============================================================
class spell_custom_pri_atonement : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_atonement);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* priest = GetTarget()->ToPlayer();
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!priest || !priest->IsAlive() || !damageInfo
            || !g_CustomSpellsEnabled)
            return;

        uint32 const heal = CalculatePct(damageInfo->GetDamage(), ATONEMENT_PCT);
        if (!heal)
            return;

        Player* lowest = nullptr;
        for (Player* ally : PriestAndGroup(priest))
        {
            if (!ally->IsAlive() || ally->IsFullHealth()
                || !priest->IsWithinDistInMap(ally, ATONEMENT_RANGE))
                continue;

            if (!lowest || ally->GetHealthPct() < lowest->GetHealthPct())
                lowest = ally;
        }

        if (!lowest)
            return;

        HealInfo healInfo(priest, lowest, heal, GetSpellInfo(),
            SPELL_SCHOOL_MASK_HOLY);
        priest->HealBySpell(healInfo);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_pri_atonement::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  End Priest Discipline section
// ============================================================

// ============================================================
//  PRIEST HOLY
// ============================================================

// ============================================================
//  900933: Direct heals have a chance to cast Holy Fire
//  Proc aura (spell_proc: direct priest heals, 50 %, no cooldown):
//  Holy Fire (900934, no facing rule, any range) on ONE random enemy
//  within 30 yd of the healed unit. Concept revision 2026-10-08 (was
//  20 %, every enemy within 10 yd, 3 sec cooldown). Only enemies
//  already in combat are picked: a heal out of combat never pulls an
//  idle mob. No internal cooldown - one heal sets off one Holy Fire
//  at most, so even a Circle of Healing on five allies adds about
//  2.5 casts.
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

        if (!g_CustomSpellsEnabled)
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

        // 900934 = Holy Fire without its facing rule and range, so
        // enemies behind the priest are reached too (bot run 411)
        if (Unit* enemy = SelectRandomEngagedEnemy(healTarget, player,
            HOLY_FIRE_PROC_RANGE))
            player->CastSpell(enemy, SPELL_PRI_HOLY_FIRE_HELPER, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_pri_heal_fire::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  900937: Lightwell: Auto Renew (2026-10-08)
//  Concept: "your well of light auto heals allies players around
//  it". The passive's 1-s pulse: each Lightwell of the priest gives
//  its Lightwell Renew to the most injured player of the priest's
//  group (the priest included) within 20 yd of the well and in its
//  sight who is below 90 % health and has no Lightwell Renew of this
//  priest - one renew per well and pulse, one charge each, exactly
//  what a click does (spell_pri_lightwell: the renew is the priest's
//  aura, the well unsummons at 0 charges), without the click's 5-yd
//  reach. Clicking still works.
// ============================================================
class spell_custom_pri_lightwell_auto : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_lightwell_auto);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Player* priest = GetTarget()->ToPlayer();
        if (!priest || !g_CustomSpellsEnabled)
            return;

        // a copy: a well that runs out of charges leaves m_Controlled
        std::vector<Creature*> wells;
        for (Unit* controlled : priest->m_Controlled)
            if (controlled->IsAlive() && controlled->IsCreature()
                && LightwellRenewFor(controlled->GetEntry()))
                wells.push_back(controlled->ToCreature());

        for (Creature* well : wells)
            Pulse(priest, well);
    }

    void Pulse(Player* priest, Creature* well)
    {
        Aura* charges = well->GetAura(SPELL_LIGHTWELL_CHARGES);
        uint32 const renew = LightwellRenewFor(well->GetEntry());
        if (!charges || !renew)
            return;

        Player* patient = nullptr;
        for (Player* ally : PriestAndGroup(priest))
        {
            if (!ally->IsAlive() || !ally->IsInMap(well)
                || ally->GetHealthPct() >= LIGHTWELL_HEAL_BELOW_PCT
                || !well->IsWithinDistInMap(ally, LIGHTWELL_RANGE)
                || !well->IsWithinLOSInMap(ally)
                || ally->GetAuraOfRankedSpell(SPELL_LIGHTWELL_RENEW_R1,
                    priest->GetGUID()))
                continue;

            if (!patient || ally->GetHealthPct() < patient->GetHealthPct())
                patient = ally;
        }

        if (!patient)
            return;

        priest->AddAura(renew, patient);
        if (charges->ModCharges(-1))
            if (TempSummon* summon = well->ToTempSummon())
                summon->UnSummon();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_pri_lightwell_auto::HandlePeriodic, EFFECT_0,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  900938: Spirit of Redemption: Guardian (2026-10-08)
//  Concept: "on death, summon a guardian that keeps mobs in combat. in
//  your spirit of redemption form, you get +50% haste +50% damage and
//  healing. its duration is extended by 10 seconds. [when] spirit of
//  redemption [ends] you are rezzed and the summon npc despawns (5min
//  cd)". Builds on the core's Spirit of Redemption (talent 20711,
//  Unit::Kill): on a lethal blow the priest becomes the spirit (27827,
//  15 sec) instead of dying, and dies when its aura 176 (27795) ends.
//  With the passive and without the 5-min cooldown 900940:
//  - the passive's death proc (PROC_FLAG_DEATH, before the core's
//    CombatStop) remembers the creatures fighting the priest;
//  - when 27827 lands: +10 sec (25 sec), 900939 (+50 % casting speed,
//    damage and healing) for that time, 900940 (5 min, survives
//    death), and the Redemption Guardian (NPC 900964, a passive decoy
//    with three times a level-80 mob's health) at the priest's spot:
//    every remembered creature is engaged with it, so none resets;
//  - when 27827 runs out the death of 27795 is skipped: the priest
//    stays alive with 50 % health and at least 50 % mana, whatever
//    fought the guardian turns to him, and the guardian despawns.
//  Without the talent nothing happens. A spirit form that ends any
//  other way (logout, .revive) kills as before; the guardian goes.
// ============================================================
class spell_custom_pri_redemption_death : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_redemption_death);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Player* priest = GetTarget()->ToPlayer();
        if (!priest || !g_CustomSpellsEnabled
            || priest->HasAura(SPELL_PRI_HOLY_REDEMPTION_CD))
            return;

        RedemptionState* state = GetRedemption(priest);
        state->Enemies.clear();
        for (auto const& entry : priest->GetCombatManager().GetPvECombatRefs())
            if (Unit* enemy = entry.second->GetOther(priest))
                if (enemy->IsAlive() && enemy->IsCreature())
                    state->Enemies.push_back(enemy->GetGUID());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_pri_redemption_death::HandleProc, EFFECT_0,
            SPELL_AURA_DUMMY);
    }
};

class spell_custom_pri_redemption_form : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_redemption_form);

    void HandleApply(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Player* priest = GetTarget()->ToPlayer();
        if (!priest)
            return;

        RedemptionState* state = GetRedemption(priest);
        GuidVector enemies;
        enemies.swap(state->Enemies);   // the capture belongs to this death
        if (!g_CustomSpellsEnabled
            || !priest->HasAura(SPELL_PRI_HOLY_REDEMPTION)
            || priest->HasAura(SPELL_PRI_HOLY_REDEMPTION_CD))
            return;

        Aura* form = GetAura();
        int32 const duration = form->GetMaxDuration() + REDEMPTION_EXTRA_MS;
        form->SetMaxDuration(duration);
        form->SetDuration(duration);

        if (Aura* empowered = priest->AddAura(SPELL_PRI_HOLY_REDEMPTION_BUFF,
            priest))
        {
            empowered->SetMaxDuration(duration);
            empowered->SetDuration(duration);
        }
        priest->AddAura(SPELL_PRI_HOLY_REDEMPTION_CD, priest);

        state->Empowered = true;
        state->Resurrect = false;
        state->Guardian.Clear();

        TempSummon* guardian = priest->SummonCreature(NPC_REDEMPTION_GUARDIAN,
            *priest, TEMPSUMMON_TIMED_DESPAWN,
            uint32(duration) + REDEMPTION_GUARDIAN_GRACE_MS);
        if (!guardian)
            return;

        guardian->SetCreatorGUID(priest->GetGUID());
        guardian->SetFaction(priest->GetFaction());
        // a player's unit, as the tentacle below: without the flag only a
        // mob hostile to the priest's faction may fight it (the
        // creature-vs-creature rule of Unit::_IsValidAttackTarget), and a
        // neutral mob that fought the priest would reset
        guardian->SetUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED);
        state->Guardian = guardian->GetGUID();

        for (ObjectGuid const& guid : enemies)
        {
            Creature* enemy = ObjectAccessor::GetCreature(*priest, guid);
            if (!enemy || !enemy->IsAlive() || !enemy->IsInMap(priest)
                || !enemy->IsValidAttackTarget(guardian))
                continue;

            enemy->EngageWithTarget(guardian);
            if (enemy->IsAIEnabled && !enemy->GetVictim())
                enemy->AI()->AttackStart(guardian);
        }
    }

    // runs before the core's shapeshift handler takes 27795 away
    void HandleRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Player* priest = GetTarget()->ToPlayer();
        if (!priest)
            return;

        RedemptionState* state = GetRedemption(priest);
        if (state->Empowered && priest->IsAlive()
            && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            state->Resurrect = true;
    }

    void HandleAfterRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Player* priest = GetTarget()->ToPlayer();
        if (!priest)
            return;

        RedemptionState* state = GetRedemption(priest);
        if (!state->Empowered)
            return;

        bool const resurrect = state->Resurrect;
        state->Empowered = false;
        state->Resurrect = false;
        priest->RemoveAurasDueToSpell(SPELL_PRI_HOLY_REDEMPTION_BUFF);

        Creature* guardian = ObjectAccessor::GetCreature(*priest,
            state->Guardian);
        state->Guardian.Clear();

        if (resurrect && priest->IsAlive())
        {
            // what the skipped end of 27795 would have removed
            priest->RemoveAurasDueToSpell(SPELL_SOR_IMMUNITY);
            priest->SetHealth(priest->CountPctFromMaxHealth(
                REDEMPTION_HEALTH_PCT));
            if (uint32 const maxMana = priest->GetMaxPower(POWER_MANA))
            {
                uint32 const floor = CalculatePct(maxMana, REDEMPTION_MANA_PCT);
                if (priest->GetPower(POWER_MANA) < floor)
                    priest->SetPower(POWER_MANA, floor);
            }

            // the fight goes on: what the guardian held turns to the priest
            if (guardian)
                for (auto const& entry
                    : guardian->GetCombatManager().GetPvECombatRefs())
                    if (Unit* enemy = entry.second->GetOther(guardian))
                        if (enemy->IsAlive() && enemy->IsValidAttackTarget(priest))
                            enemy->EngageWithTarget(priest);
        }

        if (guardian)
            guardian->DespawnOrUnsummon();
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_custom_pri_redemption_form::HandleApply, EFFECT_2,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(
            spell_custom_pri_redemption_form::HandleRemove, EFFECT_2,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_pri_redemption_form::HandleAfterRemove, EFFECT_2,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
    }
};

// 27795, the spirit's aura 176: its end kills the priest
// (AuraEffect::HandleSpiritOfRedemption) - skipped for a resurrection
class spell_custom_pri_redemption_no_death : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_redemption_no_death);

    void HandleRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Player* priest = GetTarget()->ToPlayer();
        if (priest && GetRedemption(priest)->Resurrect)
            PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(
            spell_custom_pri_redemption_no_death::HandleRemove, EFFECT_1,
            SPELL_AURA_SPIRIT_OF_REDEMPTION, AURA_EFFECT_HANDLE_REAL);
    }
};

// ============================================================
//  900942: Holy Nova: Kindled Fire (2026-10-08)
//  Concept: "casting holy nova increases the damage of your next
//  holy fire by 50% (stacks 10 times)". Holy Nova (-15237) adds a
//  stack of 900943 (aura 108 SPELLMOD_DAMAGE +50 % on Holy Fire,
//  10 stacks, 30 s); the next Holy Fire (-14914) takes them all and
//  removes them after its cast (its hits are done by then: Holy Fire
//  has no travel time). The direct damage only - the DoT part is
//  "Holy Fire +50 %" (900936).
// ============================================================
class spell_custom_pri_nova_kindle : public SpellScript
{
    PrepareSpellScript(spell_custom_pri_nova_kindle);

    void HandleAfterCast()
    {
        Player* priest = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!priest || !g_CustomSpellsEnabled
            || !priest->HasAura(SPELL_PRI_HOLY_NOVA_KINDLE))
            return;

        if (Aura* kindled = priest->GetAura(SPELL_PRI_HOLY_KINDLED_FIRE))
            kindled->ModStackAmount(1);
        else
            priest->AddAura(SPELL_PRI_HOLY_KINDLED_FIRE, priest);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_pri_nova_kindle::HandleAfterCast);
    }
};

class spell_custom_pri_holy_fire_consume : public SpellScript
{
    PrepareSpellScript(spell_custom_pri_holy_fire_consume);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (caster && caster->HasAura(SPELL_PRI_HOLY_KINDLED_FIRE))
            caster->RemoveAurasDueToSpell(SPELL_PRI_HOLY_KINDLED_FIRE);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(
            spell_custom_pri_holy_fire_consume::HandleAfterCast);
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
//  Concept revision 2026-10-08: the eruption deals 666 + 5 x the
//  priest's Paragon level Shadow damage (was 801-1,000 fixed).
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

        if (!g_CustomSpellsEnabled)
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Shadow AoE centered on the DoT target, Paragon damage
        CastAnchoredBurst(player, target, SPELL_PRI_SHADOW_AOE_HELPER,
            ParagonDamage(player));
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

        if (!g_CustomSpellsEnabled)
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
//  900972 Shadowform: Soul Feast, 900975 Shadowform: Lesser
//  Tentacles (2026-10-08)
//  Concept: "killing an enemy in shadow form infuses his soul an
//  increases your shadow damage by 1% (100 stacks). At 100 stacks auto
//  cast mind flay on a random target every 5 seconds (the triggered
//  cast is not interruptible and doesnt affect the players normal
//  cast during that time)" and "killing an enemy while in shadowform
//  spawns a lesser tentacle on its position (doesnt move, hits
//  small-medium)".
//  The kill (the priest's own killing blow, a creature or a player;
//  no critters or totems) in Shadowform:
//  - Soul Feast: a stack of 900973 (aura 79 +1 % Shadow damage,
//    100 stacks, 60 s, refreshed by each kill). The passive's 5-s
//    pulse: at 100 stacks a random enemy in combat within 30 yd gets
//    900974 - Mind Flay's damage (196 a second for 3 sec, 0.257 spell
//    power, Mind Flay's damage family flags) and its 50 % slow as an
//    aura on the enemy, not a channel: no cast bar, nothing to
//    interrupt, the priest's own casts go on.
//  - Lesser Tentacles: NPC 900999 rises where the enemy fell for
//    15 sec, rooted, and every 2 sec lashes the nearest enemy in combat
//    within 15 yd (900976, 401-600 Shadow). At most 10 at a time per
//    priest. A tentacle's own kills are no kills of the priest (the
//    hook needs the priest's killing blow): no chain of tentacles.
// ============================================================
class custom_pri_shadow_kill_playerscript : public PlayerScript
{
public:
    custom_pri_shadow_kill_playerscript()
        : PlayerScript("custom_pri_shadow_kill_playerscript",
            { PLAYERHOOK_ON_CREATURE_KILL, PLAYERHOOK_ON_PVP_KILL }) { }

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override
    {
        if (killed && !killed->IsTotem() && !killed->IsCritter())
            HandleShadowKill(killer, killed);
    }

    void OnPlayerPVPKill(Player* killer, Player* killed) override
    {
        HandleShadowKill(killer, killed);
    }

private:
    static void HandleShadowKill(Player* priest, Unit* victim)
    {
        if (!priest || !victim || !priest->IsAlive() || !g_CustomSpellsEnabled
            || !priest->HasAura(SPELL_SHADOWFORM))
            return;

        if (priest->HasAura(SPELL_PRI_SHADOW_SOUL_FEAST))
        {
            if (Aura* feast = priest->GetAura(SPELL_PRI_SHADOW_SOUL_FEAST_BUFF))
                feast->ModStackAmount(1);
            else
                priest->AddAura(SPELL_PRI_SHADOW_SOUL_FEAST_BUFF, priest);
        }

        if (priest->HasAura(SPELL_PRI_SHADOW_TENTACLES))
            SpawnTentacle(priest, victim);
    }

    static void SpawnTentacle(Player* priest, Unit* victim)
    {
        std::list<Creature*> tentacles;
        priest->GetCreatureListWithEntryInGrid(tentacles, NPC_LESSER_TENTACLE,
            100.0f);
        uint32 mine = 0;
        for (Creature* tentacle : tentacles)
            if (tentacle->IsAlive()
                && tentacle->GetCreatorGUID() == priest->GetGUID())
                ++mine;
        if (mine >= TENTACLE_LIMIT)
            return;

        if (TempSummon* tentacle = priest->SummonCreature(NPC_LESSER_TENTACLE,
            *victim, TEMPSUMMON_TIMED_DESPAWN, TENTACLE_LIFETIME_MS))
        {
            // owner: its hits count for the priest (loot, the meters)
            tentacle->SetOwnerGUID(priest->GetGUID());
            tentacle->SetCreatorGUID(priest->GetGUID());
            tentacle->SetFaction(priest->GetFaction());
            // a player's unit: it may attack what the priest may attack.
            // A plain summon is a creature, and Unit::_IsValidAttackTarget
            // lets two creatures fight only when one is hostile to the
            // other - a neutral mob the priest fights (the FL dummies,
            // faction 7) was no target, the tentacle never lashed (bot
            // run 479)
            tentacle->SetUnitFlag(UNIT_FLAG_PLAYER_CONTROLLED);
        }
    }
};

class spell_custom_pri_soul_feast : public AuraScript
{
    PrepareAuraScript(spell_custom_pri_soul_feast);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Player* priest = GetTarget()->ToPlayer();
        if (!priest || !priest->IsAlive() || !g_CustomSpellsEnabled)
            return;

        Aura* feast = priest->GetAura(SPELL_PRI_SHADOW_SOUL_FEAST_BUFF);
        if (!feast || feast->GetStackAmount() < SOUL_FEAST_MAX_STACKS)
            return;

        if (Unit* enemy = SelectRandomEngagedEnemy(priest, priest,
            FREE_FLAY_RANGE))
            priest->CastSpell(enemy, SPELL_PRI_SHADOW_FREE_FLAY, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_pri_soul_feast::HandlePeriodic, EFFECT_0,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// NPC 900999: rooted, no melee, no chase, no evade; lashes every 2 sec
struct npc_custom_lesser_tentacle : public ScriptedAI
{
    npc_custom_lesser_tentacle(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        ScriptedAI::InitializeAI();
        me->SetReactState(REACT_PASSIVE);
        me->SetControlled(true, UNIT_STATE_ROOT);
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void UpdateAI(uint32 diff) override
    {
        if (_lashTimer > diff)
        {
            _lashTimer -= diff;
            return;
        }
        _lashTimer = TENTACLE_LASH_INTERVAL_MS;

        if (Unit* target = SelectLashTarget())
        {
            me->SetFacingToObject(target);
            me->CastSpell(target, SPELL_PRI_SHADOW_TENTACLE_LASH, true);
        }
    }

private:
    // the nearest enemy within 15 yd that is already in combat
    Unit* SelectLashTarget() const
    {
        std::list<Unit*> enemies;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(me, me,
            TENTACLE_LASH_RANGE);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(me, enemies, check);
        Cell::VisitObjects(me, searcher, TENTACLE_LASH_RANGE);

        Unit* nearest = nullptr;
        float nearestDist = 0.0f;
        for (Unit* enemy : enemies)
        {
            if (!enemy->IsAlive() || !enemy->IsInCombat()
                || !me->IsValidAttackTarget(enemy)
                || !me->IsWithinLOSInMap(enemy))
                continue;

            float const dist = me->GetDistance(enemy);
            if (!nearest || dist < nearestDist)
            {
                nearest = enemy;
                nearestDist = dist;
            }
        }
        return nearest;
    }

    uint32 _lashTimer = TENTACLE_FIRST_LASH_MS;
};

// ============================================================
//  End Priest Shadow section
// ============================================================

void AddPriestSpellsScripts()
{
    // Priest Discipline
    RegisterSpellScript(spell_custom_pri_shield_explode);
    RegisterSpellScript(spell_custom_pri_weakened_soul_cd);
    RegisterSpellScript(spell_custom_pri_atonement);

    // Priest Holy
    RegisterSpellScript(spell_custom_pri_heal_fire);
    RegisterSpellScript(spell_custom_pri_lightwell_auto);
    RegisterSpellScript(spell_custom_pri_redemption_death);
    RegisterSpellScript(spell_custom_pri_redemption_form);
    RegisterSpellScript(spell_custom_pri_redemption_no_death);
    RegisterSpellScript(spell_custom_pri_nova_kindle);
    RegisterSpellScript(spell_custom_pri_holy_fire_consume);

    // Priest Shadow
    RegisterSpellScript(spell_custom_pri_dot_aoe);
    RegisterSpellScript(spell_custom_pri_dot_spread);
    new custom_pri_shadow_kill_playerscript();
    RegisterSpellScript(spell_custom_pri_soul_feast);
    RegisterCreatureAI(npc_custom_lesser_tentacle);
}
