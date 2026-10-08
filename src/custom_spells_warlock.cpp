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
#include "DynamicObject.h"

// ============================================================
//  Concept revision of 2026-10-08 (share-public custom-spells/09):
//  the ids of the new warlock spells. File-local on purpose -
//  custom_spells_common.h is not touched in this round; the minion
//  rules in custom_spells_global.cpp repeat the Demonology ones.
// ============================================================
namespace
{
    // Affliction
    constexpr uint32 SPELL_WLK_AFFL_HARVEST_BUFF    = 900805; // +1 % DoT dmg / stack
    constexpr uint32 SPELL_WLK_AFFL_HARVEST_DRAIN   = 900806; // the free Drain Life

    // Demonology
    constexpr uint32 SPELL_WLK_DEMO_MASS_SEDUCTION  = 900850; // marker, +9 targets
    constexpr uint32 SPELL_WLK_DEMO_SEDUCTION_VULN  = 900851; // +25 % damage taken
    constexpr uint32 SPELL_WLK_DEMO_FELHUNTER_AOE   = 900852; // marker

    // Destruction
    constexpr uint32 SPELL_WLK_DEST_HELLFIRE_MOBILE = 900873; // marker
    constexpr uint32 SPELL_WLK_DEST_HELLFIRE_TICKER = 900874;
    constexpr uint32 SPELL_WLK_DEST_ROF_AROUND      = 900875; // marker
    constexpr uint32 SPELL_WLK_DEST_ROF_TICKER      = 900876;
    constexpr uint32 SPELL_WLK_DEST_ROF_DAMAGE      = 900877;

    // on every second pet / demon (custom_spells_second_pet.cpp): a plain
    // Guardian, no Pet - 900835 lets it summon lesser demons too
    constexpr uint32 SPELL_SECOND_PET_MARKER        = 900526;

    // stock spells (first ranks of their chains)
    constexpr uint32 SPELL_SUFFERING_R1             = 17735;
    constexpr uint32 SPELL_HELLFIRE_R1              = 1949;
    constexpr uint32 SPELL_RAIN_OF_FIRE_R1          = 5740;

    // "dmg dependent on paragon lvl": the DK concept's formula
    constexpr int32 PARAGON_DAMAGE_BASE             = 666;
    constexpr int32 PARAGON_DAMAGE_PER_LEVEL        = 5;

    constexpr uint8 HARVEST_MAX_STACKS              = 100;
    constexpr float HARVEST_DRAIN_RANGE             = 30.0f;
    constexpr float FELHUNTER_AOE_RADIUS            = 10.0f;
    constexpr float ROF_VISUAL_RADIUS               = 8.0f;
    constexpr int32 ROF_VISUAL_MS                   = 2100;

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

    // The highest rank of a spell chain that a unit of `level` may use.
    uint32 HighestRankForLevel(uint32 firstRank, uint8 level)
    {
        uint32 best = firstRank;
        for (uint32 rank = firstRank; rank;
            rank = sSpellMgr->GetNextSpellInChain(rank))
            if (SpellInfo const* info = sSpellMgr->GetSpellInfo(rank))
                if (info->SpellLevel <= level)
                    best = rank;
        return best;
    }

    // The spell `id` when it is a rank of the chain that starts with
    // `firstRank` (the tickers carry the rank the warlock cast).
    SpellInfo const* RankOfChain(int32 id, uint32 firstRank)
    {
        if (id <= 0)
            return nullptr;

        SpellInfo const* info = sSpellMgr->GetSpellInfo(uint32(id));
        if (!info || sSpellMgr->GetFirstSpellInChain(info->Id) != firstRank)
            return nullptr;

        return info;
    }

    // Ends a channel that just started, at the warlock's next update: a
    // channel cannot be interrupted from inside its own cast (Spell::
    // IsInterruptable), and a finish - unlike a cancel - sends no
    // "Interrupted". What the channel left goes too: its aura on the
    // warlock and its ground area.
    void EndChannelSoon(Player* player, uint32 spellId)
    {
        player->m_Events.AddEventAtOffset([player, spellId]()
        {
            if (Spell* channel = player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                if (channel->GetSpellInfo()->Id == spellId)
                    player->FinishSpell(CURRENT_CHANNELED_SPELL, true);

            player->RemoveOwnedAura(spellId, player->GetGUID(), 0,
                AURA_REMOVE_BY_CANCEL);
            player->RemoveDynObject(spellId);
        }, 1ms);
    }

    // Hellfire / Rain of Fire without a channel: a self ticker with the
    // channel's duration (duration modifiers and haste included) that
    // carries the cast rank's id, then the channel ends.
    void StartMobileTicker(Player* player, SpellInfo const* channel,
        uint32 tickerId)
    {
        int32 duration = channel->GetMaxDuration();
        if (Aura const* channelAura = player->GetAura(channel->Id,
            player->GetGUID()))
            duration = channelAura->GetMaxDuration();
        else
            player->ApplySpellMod(channel->Id, SPELLMOD_DURATION, duration);

        if (duration <= 0)
            return;

        int32 rank = int32(channel->Id);
        player->CastCustomSpell(player, tickerId, &rank, nullptr, nullptr,
            true);
        if (Aura* ticker = player->GetAura(tickerId))
        {
            ticker->SetMaxDuration(duration);
            ticker->SetDuration(duration);
        }

        EndChannelSoon(player, channel->Id);
    }
}

// ============================================================
//  WARLOCK AFFLICTION: DoT ticks deal Shadow AoE (900800)
//  Proc aura: when a periodic damage tick lands (any Warlock
//  DoT), X% chance to cast Shadow AoE helper (900803)
//  centered on the DoT target (dest-centred since 2026-10-08:
//  the helper used to go off around the warlock). 2s ICD.
//  Concept revision 2026-10-08: the eruption deals 666 + 5 x the
//  warlock's Paragon level Shadow damage (was 801-1,000 fixed).
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

        if (!g_CustomSpellsEnabled)
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Shadow AoE centered on the DoT target, Paragon damage
        CastAnchoredBurst(player, target, SPELL_WLK_AFFL_DOT_AOE_HELPER,
            ParagonDamage(player));
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

        if (!g_CustomSpellsEnabled)
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
//  WARLOCK AFFLICTION: 900804 Withering Harvest (2026-10-08)
//  Concept: "dealing damage with a dot increases your dot damage by
//  1% (100 stacks). At 100 stacks auto cast life leech on a random
//  target every 5 seconds (the triggered cast is not interruptible
//  and doesnt affect the players normal cast during that time)".
//  - Effect 0 (spell_proc: warlock periodic damage, every tick): one
//    stack of 900805 (aura 108 SPELLMOD_DOT +1 % on every warlock
//    spell, 100 stacks, 30 s, refreshed by each tick). A DoT takes the
//    bonus when it is applied (the core snapshots DoT bonuses).
//  - Effect 1 (periodic dummy, 5 s): at 100 stacks the warlock drains
//    a random enemy in combat within 30 yd with 900806 - Drain Life's
//    damage and heal (aura 53, 133 a second for 5 sec, 0.143 spell
//    power, family flags of Drain Life) as an aura on the enemy, not a
//    channel: no cast bar, nothing to interrupt, the warlock's own
//    casts go on.
// ============================================================
class spell_custom_wlk_withering_harvest : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_withering_harvest);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsAlive() || !g_CustomSpellsEnabled
            || !eventInfo.GetDamageInfo())
            return;

        // one stack per tick; the stack also refreshes the 30 s
        if (Aura* harvest = player->GetAura(SPELL_WLK_AFFL_HARVEST_BUFF))
            harvest->ModStackAmount(1);
        else
            player->AddAura(SPELL_WLK_AFFL_HARVEST_BUFF, player);
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsAlive() || !g_CustomSpellsEnabled)
            return;

        Aura* harvest = player->GetAura(SPELL_WLK_AFFL_HARVEST_BUFF);
        if (!harvest || harvest->GetStackAmount() < HARVEST_MAX_STACKS)
            return;

        if (Unit* enemy = SelectRandomEngagedEnemy(player, player,
            HARVEST_DRAIN_RANGE))
            player->CastSpell(enemy, SPELL_WLK_AFFL_HARVEST_DRAIN, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_wlk_withering_harvest::HandleProc, EFFECT_0,
            SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_wlk_withering_harvest::HandlePeriodic, EFFECT_1,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  End Warlock Affliction section
// ============================================================

// ============================================================
//  WARLOCK DEMONOLOGY
// ============================================================

// ============================================================
//  900833: Killing an enemy extends Metamorphosis duration
//  PlayerScript: On kill, if player has Metamorphosis (47241),
//  extend its duration by +5s (capped at 120s).
// ============================================================
class custom_wlk_meta_kill_extend_playerscript : public PlayerScript
{
public:
    custom_wlk_meta_kill_extend_playerscript()
        : PlayerScript("custom_wlk_meta_kill_extend_playerscript") {}

    void OnPlayerCreatureKill(Player* player, Creature* /*victim*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_WLK_DEMO_META_KILL_EXT))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        Aura* meta = player->GetAura(SPELL_METAMORPHOSIS);
        if (!meta)
            return;

        int32 curDuration = meta->GetDuration();
        int32 maxCap = 120000; // 120s cap
        int32 extension = 5000; // +5s per kill

        if (curDuration < maxCap)
        {
            int32 newDuration = std::min(curDuration + extension, maxCap);
            meta->SetDuration(newDuration);
            if (newDuration > meta->GetMaxDuration())
                meta->SetMaxDuration(newDuration);
        }
    }
};

// ============================================================
//  900834: Metamorphosis: Fel Vigor (reworked 2026-10-08)
//  Concept: "while in demon form your immolation heals yourself and
//  your demon get +100% damage" (was Shadow Pulse: a 3-s Shadow AoE
//  plus a self heal). Hooked on Immolation (50590), the damage the
//  Metamorphosis Immolation Aura (50589) triggers every second: while
//  the warlock has the passive and Metamorphosis, each tick's damage,
//  summed over every enemy it hit, heals him once (100 %, logged under
//  900834, no healing bonuses on top). The demons' +100 % damage is
//  minion aura 900845, kept by custom_minion_aura_playerscript
//  (custom_spells_global.cpp) while Metamorphosis lasts.
// ============================================================
class spell_custom_wlk_fel_vigor : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_fel_vigor);

    void HandleAfterHit()
    {
        int32 const damage = GetHitDamage();
        if (damage > 0)
            _damage += uint32(damage);
    }

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !_damage || !player->IsAlive() || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_WLK_DEMO_FEL_VIGOR_PASSIVE)
            || !player->HasAura(SPELL_METAMORPHOSIS))
            return;

        SpellInfo const* vigor = sSpellMgr->GetSpellInfo(
            SPELL_WLK_DEMO_FEL_VIGOR_PASSIVE);
        if (!vigor)
            return;

        HealInfo healInfo(player, player, _damage, vigor,
            SPELL_SCHOOL_MASK_FIRE);
        player->HealBySpell(healInfo);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_wlk_fel_vigor::HandleAfterHit);
        AfterCast += SpellCastFn(spell_custom_wlk_fel_vigor::HandleAfterCast);
    }

private:
    uint32 _damage = 0;   // this tick, every enemy hit
};

// ============================================================
//  900835: Demons have chance to spawn lesser versions
//  UnitScript: on pet melee auto-attack, X% chance to summon
//  a lesser demon (30s duration, reduced stats). Uses NPC
//  entries based on the pet type. 30s ICD via static map.
// ============================================================
class LesserDemonCd : public DataMap::Base
{
public:
    uint32 Last = 0;
};

class custom_wlk_lesser_demon_unitscript : public UnitScript
{
public:
    custom_wlk_lesser_demon_unitscript()
        : UnitScript("custom_wlk_lesser_demon_unitscript") {}

    void OnDamage(Unit* attacker, Unit* victim, uint32& /*damage*/) override
    {
        if (!attacker || !victim)
            return;

        // Must be a pet/minion with an owner - the pet or the second
        // demon (a Guardian carrying 900526); both share the 30 s ICD
        Creature* creature = attacker->ToCreature();
        if (!creature || !(creature->IsPet()
            || creature->HasAura(SPELL_SECOND_PET_MARKER)))
            return;

        Unit* ownerUnit = creature->GetOwner();
        if (!ownerUnit)
            return;

        Player* owner = ownerUnit->ToPlayer();
        if (!owner)
            return;

        if (!owner->HasAura(SPELL_WLK_DEMO_LESSER_SPAWN))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // 30s ICD per player, kept on the player (the old static map was
        // shared by every map thread)
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        LesserDemonCd* cd = owner->CustomData.GetDefault<LesserDemonCd>(
            "mod_custom_spells_lesser_demon");
        if (cd->Last && now - cd->Last < 30)
            return;

        // 10% chance
        if (urand(1, 100) > 10)
            return;

        cd->Last = now;

        // Determine lesser demon NPC based on pet entry
        uint32 petEntry = creature->GetEntry();
        uint32 lesserNpc = 0;

        // Imp family
        if (petEntry == 416)
            lesserNpc = NPC_LESSER_IMP;
        // Felguard family
        else if (petEntry == 17252)
            lesserNpc = NPC_LESSER_FELGUARD;
        // Voidwalker family
        else if (petEntry == 1860)
            lesserNpc = NPC_LESSER_VOIDWALKER;
        else
            lesserNpc = NPC_LESSER_IMP; // fallback

        if (Creature* lesser = owner->SummonCreature(lesserNpc,
            creature->GetPositionX(), creature->GetPositionY(),
            creature->GetPositionZ(), creature->GetOrientation(),
            TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000))
        {
            lesser->SetOwnerGUID(owner->GetGUID());
            lesser->SetCreatorGUID(owner->GetGUID());
            lesser->SetFaction(owner->GetFaction());

            if (victim->IsAlive())
            {
                lesser->Attack(victim, true);
                lesser->GetMotionMaster()->MoveChase(victim);
            }
        }
    }
};

// ============================================================
//  900837: Imp Firebolt +9 targets
//  Hooked on Imp Firebolt (47964). After hitting main target,
//  deals same damage to up to 9 additional enemies within 10yd.
//  (900846 "Firebolt 50% faster", 2026-10-08, is minion aura
//  900847 on the Imp: casting speed +50 %, custom_spells_global.cpp.)
// ============================================================
class spell_custom_wlk_imp_fb_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_imp_fb_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        // Caster is the Imp - check owner
        Unit* ownerUnit = caster->GetOwner();
        if (!ownerUnit)
            return;

        Player* owner = ownerUnit->ToPlayer();
        if (!owner)
            return;

        if (!owner->HasAura(SPELL_WLK_DEMO_IMP_FB_AOE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        // Find up to 9 nearby enemies
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, caster, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(target, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        uint32 count = 0;
        for (Unit* extra : targets)
        {
            if (count >= 9)
                break;
            if (!extra->IsAlive() || !caster->IsValidAttackTarget(extra))
                continue;

            SpellInfo const* spellInfo = GetSpellInfo();
            SpellNonMeleeDamage dmgInfo(caster, extra, spellInfo, spellInfo->GetSchoolMask());
            dmgInfo.damage = damage;
            caster->DealSpellDamage(&dmgInfo, true);
            caster->SendSpellNonMeleeDamageLog(&dmgInfo);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_wlk_imp_fb_aoe::HandleAfterHit);
    }
};

// ============================================================
//  900848 Voidwalker: Void Bulwark (2026-10-08)
//  Concept: "voidwalker: +500% hp and passive cast his aoe taunt every
//  10 sec in combat". The minion manager (custom_spells_global.cpp)
//  keeps 900849 on the warlock's Voidwalkers: effect 0 = aura 133
//  +500 % maximum health (the core keeps the health percentage),
//  effect 1 = this 10-s pulse - in combat the Voidwalker casts the
//  highest Suffering rank for its level, triggered (no cooldown).
// ============================================================
class spell_custom_wlk_void_bulwark : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_void_bulwark);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* voidwalker = GetTarget();
        if (!voidwalker->IsAlive() || !voidwalker->IsInCombat()
            || !g_CustomSpellsEnabled)
            return;

        voidwalker->CastSpell(voidwalker,
            HighestRankForLevel(SPELL_SUFFERING_R1, voidwalker->GetLevel()),
            true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_wlk_void_bulwark::HandlePeriodic, EFFECT_1,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  900850 Succubus: Mass Seduction (2026-10-08)
//  Concept: "succubus: charm now hits +9 targets and their damage take
//  is increased by 25% for 30seconds (stacks 4 times)".
//  +9 targets = the marker itself: SPELLMOD_JUMP_TARGETS value 10 on
//  Seduction's family flag (flags1 0x10000000, the only spell that
//  carries it); a pet's spells take its owner's modifiers
//  (Unit::GetSpellModOwner), as Demonic Power does for the Imp.
//  The debuff: every unit Seduction (6358) lands on gets a stack of
//  900851 (aura 87 +25 % damage taken, 30 s, 4 stacks) from the
//  warlock - AddAura, so neither range nor line of sight of the
//  warlock matter.
// ============================================================
class spell_custom_wlk_mass_seduction : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_mass_seduction);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !target->IsAlive() || !g_CustomSpellsEnabled)
            return;

        Player* owner = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner || !owner->HasAura(SPELL_WLK_DEMO_MASS_SEDUCTION))
            return;

        owner->AddAura(SPELL_WLK_DEMO_SEDUCTION_VULN, target);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_wlk_mass_seduction::HandleAfterHit);
    }
};

// ============================================================
//  900852 Felhunter: Area Spell Lock and Devour Magic (2026-10-08)
//  Operator: "Spell Lock is aoe now, Devour Magic also".
//  Hooked on Spell Lock (-19244) and Devour Magic (-19505): after a
//  cast of the Felhunter (or any demon of a warlock with the passive)
//  the same rank is cast, triggered, on every other unit within 10 yd
//  of the target - enemies when the target was an enemy (Spell Lock's
//  interrupt and silence, Devour Magic's purge), allies when it was an
//  ally (Devour Magic's cleanse). Each unit once; the repeats are
//  triggered casts and spread nothing themselves.
// ============================================================
class spell_custom_wlk_felhunter_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_felhunter_aoe);

    void HandleAfterCast()
    {
        if (GetSpell()->IsTriggered() || !g_CustomSpellsEnabled)
            return;

        Unit* caster = GetCaster();
        Unit* target = GetExplTargetUnit();
        if (!caster || !target)
            return;

        Player* owner = caster->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner || !owner->HasAura(SPELL_WLK_DEMO_FELHUNTER_AOE))
            return;

        bool const friendly = caster->IsFriendlyTo(target);
        std::list<Unit*> units;
        if (friendly)
        {
            Acore::AnyFriendlyUnitInObjectRangeCheck check(target, caster,
                FELHUNTER_AOE_RADIUS);
            Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck>
                searcher(target, units, check);
            Cell::VisitObjects(target, searcher, FELHUNTER_AOE_RADIUS);
        }
        else
        {
            Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, caster,
                FELHUNTER_AOE_RADIUS);
            Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
                searcher(target, units, check);
            Cell::VisitObjects(target, searcher, FELHUNTER_AOE_RADIUS);
        }

        uint32 const spellId = GetSpellInfo()->Id;
        for (Unit* unit : units)
        {
            if (unit == target || unit == caster || !unit->IsAlive())
                continue;

            if (friendly ? !caster->IsValidAssistTarget(unit)
                : !caster->IsValidAttackTarget(unit))
                continue;

            caster->CastSpell(unit, spellId, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_wlk_felhunter_aoe::HandleAfterCast);
    }
};

// ============================================================
//  900838: Felguard AoE unlimited targets
//  Hooked on Felguard Cleave (47994). Uses AfterCast + helper
//  to hit extra targets beyond the DBC MaxTargets cap.
//  FilterTargets records which targets the spell already hits,
//  then AfterCast finds remaining enemies and casts the helper.
// ============================================================
class spell_custom_wlk_fg_unlim : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_fg_unlim);

    void RecordHit()
    {
        if (Unit* target = GetHitUnit())
            _hitTargets.insert(target->GetGUID());
        if (_savedDamage == 0)
            _savedDamage = GetHitDamage();
    }

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* ownerUnit = caster->GetOwner();
        if (!ownerUnit)
            return;

        Player* owner = ownerUnit->ToPlayer();
        if (!owner || !owner->HasAura(SPELL_WLK_DEMO_FG_UNLIM_TARGETS))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Find all enemies in 8yd not already hit by the original Cleave
        std::list<Unit*> enemies;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, enemies, check);
        Cell::VisitObjects(caster, searcher, 8.0f);

        int32 damage = _savedDamage > 0 ? _savedDamage : 500;

        for (Unit* enemy : enemies)
        {
            if (_hitTargets.count(enemy->GetGUID()))
                continue;

            caster->CastCustomSpell(
                SPELL_WLK_DEMO_FG_CLEAVE_HELPER, SPELLVALUE_BASE_POINT0,
                damage, enemy, true);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_custom_wlk_fg_unlim::RecordHit);
        AfterCast += SpellCastFn(
            spell_custom_wlk_fg_unlim::HandleAfterCast);
    }

private:
    GuidUnorderedSet _hitTargets;
    int32 _savedDamage = 0;
};

// ============================================================
//  900840: Sacrificing pet grants ALL pet bonuses
//  Hooked on Demonic Sacrifice (18788). After cast, apply all
//  sacrifice buffs regardless of which pet was sacrificed.
// ============================================================
class spell_custom_wlk_sacrifice_all : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_sacrifice_all);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_WLK_DEMO_SACRIFICE_ALL))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Apply all sacrifice buffs
        static const uint32 sacrificeBuffs[] = {
            SPELL_SACRIFICE_IMP,       // +15% Fire damage
            SPELL_SACRIFICE_VW,        // HP regen
            SPELL_SACRIFICE_SUCCUBUS,  // +15% Shadow damage
            SPELL_SACRIFICE_FELHUNTER  // Mana regen
        };

        for (uint32 buffId : sacrificeBuffs)
        {
            if (!player->HasAura(buffId))
                player->CastSpell(player, buffId, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_wlk_sacrifice_all::HandleAfterCast);
    }
};

// ============================================================
//  End Warlock Demonology section
// ============================================================

// ============================================================
//  WARLOCK DESTRUCTION
// ============================================================

// ============================================================
//  900873 Hellfire: Free Movement (2026-10-08)
//  Concept: "you can now move while using hellfire". The 3.3.5 client
//  cancels a channel on movement, so for a warlock with the passive
//  Hellfire (-1949) is no channel any more: right after the cast the
//  warlock carries ticker 900874 for the channel's duration, and the
//  channel itself ends at his next update (EndChannelSoon). Each
//  second the ticker casts the cast rank's own Hellfire Effect round
//  the warlock - wherever he stands now - and burns him with the
//  rank's own self damage (spell power, absorbs and resistances as
//  the channel's periodic effect). The ticker is a positive aura: a
//  right-click ends Hellfire, as moving did before.
// ============================================================
class spell_custom_wlk_hellfire_mobile : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_hellfire_mobile);

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_WLK_DEST_HELLFIRE_MOBILE))
            return;

        StartMobileTicker(player, GetSpellInfo(),
            SPELL_WLK_DEST_HELLFIRE_TICKER);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_wlk_hellfire_mobile::HandleAfterCast);
    }
};

class spell_custom_wlk_hellfire_ticker : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_hellfire_ticker);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsAlive() || !g_CustomSpellsEnabled)
            return;

        SpellInfo const* hellfire = RankOfChain(aurEff->GetAmount(),
            SPELL_HELLFIRE_R1);
        if (!hellfire)
            return;

        // the enemies: the rank's Hellfire Effect (dest = the caster), as
        // the channel's periodic trigger cast it
        if (uint32 effect = hellfire->Effects[EFFECT_0].TriggerSpell)
            player->CastSpell(player, effect, true, nullptr, aurEff);

        // the warlock: the rank's own burn (the channel's effect 1)
        int32 const base = hellfire->Effects[EFFECT_1].CalcValue(player);
        if (base <= 0)
            return;

        uint32 damage = player->SpellDamageBonusDone(player, hellfire,
            uint32(base), DOT, EFFECT_1);
        damage = player->SpellDamageBonusTaken(player, hellfire, damage, DOT);

        DamageInfo dmgInfo(player, player, damage, hellfire,
            hellfire->GetSchoolMask(), DOT);
        Unit::CalcAbsorbResist(dmgInfo);

        SpellNonMeleeDamage burn(player, player, hellfire,
            hellfire->GetSchoolMask());
        burn.damage = dmgInfo.GetDamage();
        burn.absorb = dmgInfo.GetAbsorb();
        burn.resist = dmgInfo.GetResist();
        player->SendSpellNonMeleeDamageLog(&burn);
        player->DealSpellDamage(&burn, false);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_wlk_hellfire_ticker::HandlePeriodic, EFFECT_0,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  900875 Rain of Fire: Around You (2026-10-08)
//  Concept: "rain of fire is around you for its duration instead on
//  target ground and dont need a channel anymore". Same pattern as
//  Hellfire above: Rain of Fire (-5740) ends its channel and its
//  ground area at once; ticker 900876 lasts the channel's duration
//  and every 2 sec draws the rank's rain at the warlock's feet (a
//  2-s area of the cast rank, visual only) and casts 900877 there:
//  the rank's tick damage (its trigger spell's points, level scaling
//  included) to every enemy within 8 yd of the warlock, Rain of
//  Fire's spell power coefficient and family flags. The aimed point
//  of the cast is ignored.
// ============================================================
class spell_custom_wlk_rof_around : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_rof_around);

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_WLK_DEST_ROF_AROUND))
            return;

        StartMobileTicker(player, GetSpellInfo(), SPELL_WLK_DEST_ROF_TICKER);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_wlk_rof_around::HandleAfterCast);
    }
};

class spell_custom_wlk_rof_ticker : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_rof_ticker);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsAlive() || !g_CustomSpellsEnabled)
            return;

        SpellInfo const* rain = RankOfChain(aurEff->GetAmount(),
            SPELL_RAIN_OF_FIRE_R1);
        if (!rain)
            return;

        SpellInfo const* tick = sSpellMgr->GetSpellInfo(
            rain->Effects[EFFECT_1].TriggerSpell);
        if (!tick)
            return;

        int32 damage = tick->Effects[EFFECT_0].CalcValue(player);
        if (damage <= 0)
            return;

        // the falling fire, drawn where the warlock stands now
        DynamicObject* visual = new DynamicObject();
        if (!visual->CreateDynamicObject(
            player->GetMap()->GenerateLowGuid<HighGuid::DynamicObject>(),
            player, rain->Id, *player, ROF_VISUAL_RADIUS,
            DYNAMIC_OBJECT_AREA_SPELL))
            delete visual;
        else
            visual->SetDuration(ROF_VISUAL_MS);

        player->CastCustomSpell(player, SPELL_WLK_DEST_ROF_DAMAGE, &damage,
            nullptr, nullptr, true, nullptr, aurEff);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_wlk_rof_ticker::HandlePeriodic, EFFECT_0,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  End Warlock Destruction section
// ============================================================

// ============================================================
//  WARLOCK PET AURAS (900836, 900839; 2026-10-08: 900834's +100 %
//  in Metamorphosis, 900846 Imp casting speed, 900848 Voidwalker):
//  minion auras kept by custom_minion_aura_playerscript
//  (custom_spells_global.cpp); the old OnDamage multiplier was
//  invisible in the combat log.
// ============================================================

void AddWarlockSpellsScripts()
{
    // Warlock Affliction
    RegisterSpellScript(spell_custom_wlk_dot_aoe);
    RegisterSpellScript(spell_custom_wlk_dot_spread);
    RegisterSpellScript(spell_custom_wlk_withering_harvest);

    // Warlock Demonology
    new custom_wlk_meta_kill_extend_playerscript();
    RegisterSpellScript(spell_custom_wlk_fel_vigor);
    new custom_wlk_lesser_demon_unitscript();
    RegisterSpellScript(spell_custom_wlk_imp_fb_aoe);
    RegisterSpellScript(spell_custom_wlk_void_bulwark);
    RegisterSpellScript(spell_custom_wlk_mass_seduction);
    RegisterSpellScript(spell_custom_wlk_felhunter_aoe);
    RegisterSpellScript(spell_custom_wlk_fg_unlim);
    RegisterSpellScript(spell_custom_wlk_sacrifice_all);

    // Warlock Destruction (900866-900870 are pure spell data)
    RegisterSpellScript(spell_custom_wlk_hellfire_mobile);
    RegisterSpellScript(spell_custom_wlk_hellfire_ticker);
    RegisterSpellScript(spell_custom_wlk_rof_around);
    RegisterSpellScript(spell_custom_wlk_rof_ticker);
}
