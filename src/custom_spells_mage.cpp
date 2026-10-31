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
#include "AllSpellScript.h"

// ============================================================
//  Mage spells of the third concept list, wave 2 (share-public
//  custom-spells/records/2026-10-31-third-concept-list.md). File-local
//  helpers and numbers; the ids are in custom_spells_common.h.
// ============================================================
namespace
{
    constexpr uint32 BLINK_IMAGE_MS         = 20000; // "for 20 seconds"
    constexpr uint32 IMAGE_POWER_SYNC_MS    = 500;   // how often the image damage stacks are brought up to date
    constexpr uint8  IMAGE_POWER_MAX_STACKS = 30;    // the aura's CumulativeAura
    constexpr float  BLOCK_LANCE_RANGE      = 25.0f; // "every enemy in 25y range"
    constexpr uint32 BLOCK_LANCE_MAX_TARGETS = 40;   // a tick never casts more than this many lances

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
}

// ============================================================
//  MAGE ARCANE: Arcane Charges stack to 8 (900705) is pure data since
//  2026-10-08: SPELLMOD_MAX_AURA_STACKS +4 on the Arcane Blast debuff
//  (36032). The old AfterCast raised the stack count, and the core's
//  own Arcane Blast script re-applied the debuff right after and
//  clamped it back to 4 (bot run 409).
// ============================================================

// ============================================================
//  MAGE ARCANE: Arcane Explosion generates 1 Arcane Charge
//  (900706). Hooked on Arcane Explosion (all ranks via -42921).
//  After cast, applies 1 stack of Arcane Blast debuff (36032)
//  without consuming existing stacks.
// ============================================================
class spell_custom_mage_ae_charges : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_ae_charges);

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_MAGE_ARC_AE_CHARGES_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // one charge more, up to the stack cap - 4, or 8 with 900705 (its
        // SPELLMOD_MAX_AURA_STACKS raises the debuff's cap in the core)
        player->CastSpell(player, SPELL_ARCANE_BLAST_DEBUFF, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_mage_ae_charges::HandleAfterCast);
    }
};

// ============================================================
//  MAGE ARCANE: Evocation increases spell damage (900707)
//  AuraScript on Evocation (12051). Each periodic tick
//  (every 2s, 4 ticks total) adds one stack of Evocation
//  Power buff (900712) which grants +20% spell damage/stack.
// ============================================================
class spell_custom_mage_evocation_power : public AuraScript
{
    PrepareAuraScript(spell_custom_mage_evocation_power);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        if (!target)
            return;

        Player* player = target->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_MAGE_ARC_EVOC_POWER_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Each Evocation tick adds one stack of spell damage buff
        player->CastSpell(player, SPELL_MAGE_ARC_EVOC_BUFF, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_mage_evocation_power::HandlePeriodic,
            EFFECT_0, SPELL_AURA_OBS_MOD_POWER);
    }
};

// ============================================================
//  MAGE ARCANE: Emergency Mana Shield (900708)
//  Proc aura: when taking damage at or below 30% health,
//  auto-cast Mana Shield (highest rank) and restore all mana.
//  60s internal cooldown.
// ============================================================
class spell_custom_mage_emergency_shield : public AuraScript
{
    PrepareAuraScript(spell_custom_mage_emergency_shield);

    bool CheckProc(ProcEventInfo& /*eventInfo*/)
    {
        Player* player = GetTarget() ? GetTarget()->ToPlayer() : nullptr;
        // The core starts the internal cooldown before HandleProc. Reject
        // ineligible hits here so ordinary damage cannot spend the cooldown.
        return g_CustomSpellsEnabled && player && player->IsAlive()
            && player->GetHealthPct() <= 30.0f;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Preserve the existing inclusive 30% health threshold.
        if (player->GetHealthPct() > 30.0f)
            return;

        // Activate Mana Shield (highest rank)
        player->CastSpell(player, SPELL_MANA_SHIELD_R9, true);

        // Restore all mana
        player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(
            spell_custom_mage_emergency_shield::CheckProc);
        OnEffectProc += AuraEffectProcFn(
            spell_custom_mage_emergency_shield::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  MAGE ARCANE: Targeted Blink (900713)
//  Active spell with ground targeting. Teleports the caster
//  to the selected destination (max 40yd range).
// ============================================================
// ============================================================
//  SPELL 900709: Blink to Target (SpellScript)
//  Hooked on Blink (1953). With the marker passive, Blink
//  teleports to the currently selected unit (up to 30yd, in
//  line of sight) instead of the fixed forward jump.
// ============================================================
class spell_custom_mage_blink_to_target : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_blink_to_target);

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        if (!player->HasAura(SPELL_MAGE_ARC_BLINK_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        Unit* target = ObjectAccessor::GetUnit(*player, player->GetTarget());
        if (!target || target == player)
            return;

        if (!player->IsWithinDistInMap(target, 30.0f)
            || !player->IsWithinLOSInMap(target))
            return;

        player->NearTeleportTo(target->GetPositionX(), target->GetPositionY(),
            target->GetPositionZ(), player->GetOrientation());
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_mage_blink_to_target::HandleAfterCast);
    }
};

class spell_custom_mage_targeted_blink : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_targeted_blink);

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        if (!g_CustomSpellsEnabled)
            return;

        WorldLocation const* dest = GetExplTargetDest();
        if (!dest)
            return;

        player->NearTeleportTo(dest->GetPositionX(), dest->GetPositionY(),
            dest->GetPositionZ(), player->GetOrientation());
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_mage_targeted_blink::HandleAfterCast);
    }
};

// ============================================================
//  MAGE ARCANE: 900720 Blink: Mirror Image
//  List line: "blinking spawns a mirror image for 20seconds on the target
//  location".
//  After Blink (1953, bound as an extra script beside 900709's) and
//  Targeted Blink (900713) a marked mage gets ONE Mirror Image at the spot
//  it arrives at, for 20 s. The images are summoned as the stock Mirror
//  Image spells summon theirs (creature 31216, summon properties 1021): the
//  core's image AI clones the mage, follows it and fights its target.
//  The arrival point: a mage's near teleport (Blink's leap, 900709's hop to
//  the target, 900713's jump) sets Player::GetTeleportDest at once, but the
//  position itself only changes when the client acknowledges it - so the
//  summon waits one update (the casts' teleports are all done by then) and
//  reads the destination while the teleport is pending, the position once
//  it has landed.
// ============================================================
class spell_custom_mage_blink_image : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_blink_image);

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_MAGE_ARC_BLINK_IMAGE_PASSIVE))
            return;

        player->m_Events.AddEventAtOffset([player]()
        {
            if (!player->IsInWorld() || !player->IsAlive()
                || !player->HasAura(SPELL_MAGE_ARC_BLINK_IMAGE_PASSIVE))
                return;

            Position const arrival = player->IsBeingTeleportedNear()
                ? Position(player->GetTeleportDest()) : player->GetPosition();
            player->SummonCreature(NPC_MAGE_MIRROR_IMAGE, arrival.GetPositionX(),
                arrival.GetPositionY(), arrival.GetPositionZ(),
                arrival.GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, BLINK_IMAGE_MS,
                sSummonPropertiesStore.LookupEntry(SUMMON_PROPERTIES_MIRROR_IMAGE));
        }, 1ms);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_mage_blink_image::HandleAfterCast);
    }
};

// ============================================================
//  MAGE ARCANE: 900721 Mirror Images: Strength in Numbers
//  List line: "mirror images damage is increased by 5% for each mirror image
//  alive".
//  Every 0.5 s a marked mage's living Mirror Images (creature 31216, from
//  any source: Mirror Image, Mirror Shield, the Blink image) carry 900722 -
//  aura 79, +5 % damage done per stack - with as many stacks as there are
//  images alive (3 images: +15 % each). A summon or a despawn is therefore
//  followed by the new count within half a second. Without the marker (or
//  with the module off) the aura is taken off again. An image's own splash
//  (900716) copies its boosted hit, so the splash is boosted once more.
// ============================================================
class ImagePowerState : public DataMap::Base
{
public:
    uint32 Timer = 0;
    bool Active = false;   // some image may carry the aura
};

class custom_mage_image_power_playerscript : public PlayerScript
{
public:
    custom_mage_image_power_playerscript()
        : PlayerScript("custom_mage_image_power_playerscript",
            { PLAYERHOOK_ON_UPDATE }) { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (!player || !player->IsInWorld())
            return;

        ImagePowerState* state = player->CustomData.GetDefault<
            ImagePowerState>("mod_custom_spells_image_power");
        state->Timer += diff;
        if (state->Timer < IMAGE_POWER_SYNC_MS)
            return;
        state->Timer = 0;

        bool const wanted = g_CustomSpellsEnabled && player->IsAlive()
            && player->HasAura(SPELL_MAGE_ARC_IMAGE_POWER_PASSIVE);
        // Nothing to give and nothing left to take away: the common case
        if (!wanted && !state->Active)
            return;

        std::vector<Unit*> images;
        for (Unit* controlled : player->m_Controlled)
            if (controlled && controlled->IsAlive()
                && controlled->GetEntry() == NPC_MAGE_MIRROR_IMAGE)
                images.push_back(controlled);

        uint8 const stacks = uint8(std::min<size_t>(images.size(),
            IMAGE_POWER_MAX_STACKS));
        for (Unit* image : images)
        {
            Aura* power = image->GetAura(SPELL_MAGE_ARC_IMAGE_POWER_AURA);
            if (!wanted)
            {
                if (power)
                    image->RemoveAura(power);
                continue;
            }

            if (!power)
                power = image->AddAura(SPELL_MAGE_ARC_IMAGE_POWER_AURA, image);
            if (power && power->GetStackAmount() != stacks)
                power->SetStackAmount(stacks);
        }
        state->Active = wanted && !images.empty();
    }
};

// ============================================================
//  MAGE ARCANE: 900700 Mana Regen is shared with the shaman and the
//  druid: custom_mana_regen_playerscript (custom_spells_global.cpp)
// ============================================================

// ============================================================
//  End Mage Arcane section
// ============================================================

// ============================================================
//  MAGE FIRE: Pyroblast triggers Hot Streak (900738)
//  Hooked on Pyroblast (all ranks via -42891).
//  After each Pyroblast cast, applies Hot Streak (48108)
//  giving a guaranteed instant Pyroblast next cast.
// ============================================================
class spell_custom_mage_pyro_hotstreak : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_pyro_hotstreak);

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_MAGE_FIRE_PYRO_HS_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Only a hard-cast Pyroblast grants Hot Streak: one made instant by
        // Hot Streak would grant the next one, an endless chain of free
        // instant Pyroblasts (bot run 409)
        if (GetSpell()->GetCastTime() <= 0)
            return;

        // Apply Hot Streak buff (instant Pyroblast)
        player->CastSpell(player, SPELL_HOT_STREAK, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_mage_pyro_hotstreak::HandleAfterCast);
    }
};

// ============================================================
//  MAGE FIRE: 900744 Dragon's Breath: Living Bomb
//  List line: "enemies hit by dragon breath also receive living bomb".
//  Hooked on Dragon's Breath (all ranks, -31661): every enemy a Dragon's
//  Breath hits (AfterHit: hits only, not misses) gets the highest Living
//  Bomb rank the mage knows, a triggered cast (no cost, its own hit roll).
//  A mage without Living Bomb gets nothing. (Living Bomb on any number of
//  targets is stock: it carries no single-target attribute.)
// ============================================================
class spell_custom_mage_db_living_bomb : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_db_living_bomb);

    void HandleAfterHit()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!player || !target || !target->IsAlive() || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_MAGE_FIRE_DB_BOMB_PASSIVE))
            return;

        if (uint32 bomb = HighestKnownRank(player, SPELL_MAGE_LIVING_BOMB_R1))
            player->CastSpell(target, bomb, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_mage_db_living_bomb::HandleAfterHit);
    }
};

// ============================================================
//  MAGE FIRE: 900745 Molten Armor: Flamestrike
//  List line: "molten armor has a 5% chance on damage to cast a free
//  flamestrike on the target".
//  Proc passive (spell_proc: damage done by spells, DoT ticks and wand
//  shots, 5 %, 1 s cooldown so one multi-target spell cannot fire it once
//  per target): while a Molten Armor of the mage is up, the highest
//  Flamestrike rank the mage knows goes off, triggered (no cost, no cast
//  time), on the position of the struck target. Flamestrike's own hits and
//  its burning ground never proc it (CheckProc), so it cannot chain.
// ============================================================
class spell_custom_mage_molten_flamestrike : public AuraScript
{
    PrepareAuraScript(spell_custom_mage_molten_flamestrike);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* mage = GetTarget();
        Unit* target = eventInfo.GetActionTarget();
        if (!g_CustomSpellsEnabled || !mage || !target || !target->IsAlive()
            || target == mage)
            return false;

        if (!mage->HasAura(SPELL_MAGE_MOLTEN_ARMOR_R1)
            && !mage->HasAura(SPELL_MAGE_MOLTEN_ARMOR_R2)
            && !mage->HasAura(SPELL_MAGE_MOLTEN_ARMOR_R3))
            return false;

        // Flamestrike (mage flags0 0x4) and its burn must not set off another
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return !(spellInfo && spellInfo->SpellFamilyName == SPELLFAMILY_MAGE
            && (spellInfo->SpellFamilyFlags[0] & 0x4));
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* mage = GetTarget() ? GetTarget()->ToPlayer() : nullptr;
        Unit* target = eventInfo.GetActionTarget();
        if (!mage || !target || !target->IsInWorld())
            return;

        if (uint32 flamestrike = HighestKnownRank(mage, SPELL_MAGE_FLAMESTRIKE_R1))
            mage->CastSpell(target->GetPositionX(), target->GetPositionY(),
                target->GetPositionZ(), flamestrike, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_mage_molten_flamestrike::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_mage_molten_flamestrike::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  End Mage Fire section
// ============================================================

// ============================================================
//  MAGE FROST: Water Elemental is permanent (900770)
//  Hooked on Summon Water Elemental (31687).
//  After summoning, sets the Water Elemental to permanent
//  duration (no auto-despawn).
// ============================================================
class spell_custom_mage_permanent_water_ele : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_permanent_water_ele);

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_MAGE_FROST_PERM_ELE_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Find the Water Elemental among controlled units and make permanent
        for (Unit* controlled : player->m_Controlled)
        {
            if (!controlled || !controlled->IsAlive())
                continue;

            if (controlled->GetEntry() == NPC_WATER_ELEMENTAL)
            {
                if (TempSummon* summon = controlled->ToTempSummon())
                    summon->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN);
            }
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_mage_permanent_water_ele::HandleAfterCast);
    }
};

// ============================================================
//  MAGE FROST: Comet Shower (900771)
//  Active ground-targeted spell. Hits all enemies within 15yd
//  of the target location with Frost Comet impacts.
// ============================================================
class spell_custom_mage_comet_shower : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_comet_shower);

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        if (!g_CustomSpellsEnabled)
            return;

        WorldLocation const* dest = GetExplTargetDest();
        if (!dest)
            return;

        float destX = dest->GetPositionX();
        float destY = dest->GetPositionY();

        // Search radius: distance from player to dest + 15yd comet radius
        float distToDest = player->GetDistance(destX, destY, dest->GetPositionZ());
        float searchRange = distToDest + 16.0f;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(player, player, searchRange);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(player, targets, check);
        Cell::VisitObjects(player, searcher, searchRange);

        for (Unit* target : targets)
        {
            if (!target->IsAlive() || !player->IsValidAttackTarget(target))
                continue;

            // Check if target is within 15yd of the comet impact zone
            float dx = target->GetPositionX() - destX;
            float dy = target->GetPositionY() - destY;
            if ((dx * dx + dy * dy) > 225.0f) // 15yd^2
                continue;

            // Hit each target with a frost comet impact
            player->CastSpell(target, SPELL_MAGE_FROST_COMET_HELPER, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_mage_comet_shower::HandleAfterCast);
    }
};

// ============================================================
//  MAGE FROST: 900775 Ice Block: Ice Lance Barrage / 900776 Ice Block:
//  Detonation
//  List lines: "while in iceblock you auto fire an ice lance to every enemy
//  in 25y range every 0.5 sec" and "when iceblock end it detonates and deal
//  aoe damage dependend on paragon level".
//  An AuraScript on Ice Block (45438), on its stun effect:
//  - apply: a mage with 900775 gets the ticker 900777 (a 0.5-s periodic
//    dummy aura, infinite; Ice Block takes it off again). Each tick the
//    highest Ice Lance rank the mage knows goes off at every enemy within
//    25 yd in line of sight (at most 40 per tick), triggered: free, no
//    cooldown, and ignoring the mage's Ice Block state (a triggered cast
//    skips the stun check; the core's own checks of Ice Lance still apply:
//    the target must be in the mage's front arc, so for each lance the
//    mage is turned toward its target for the instant of the cast and
//    turned back afterwards - a stunned player turns no further, the
//    client is not told).
//  - remove (it ran out, was cancelled or broken; not a death): a mage with
//    900776 detonates: 900778, Frost damage 666 + 5 x the Paragon level to
//    every enemy within 10 yd of the mage.
// ============================================================
class spell_custom_mage_ice_block_barrage : public AuraScript
{
    PrepareAuraScript(spell_custom_mage_ice_block_barrage);

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* mage = GetTarget() ? GetTarget()->ToPlayer() : nullptr;
        if (!mage || !g_CustomSpellsEnabled
            || !mage->HasAura(SPELL_MAGE_FROST_BLOCK_LANCES_PASSIVE))
            return;

        mage->CastSpell(mage, SPELL_MAGE_FROST_BLOCK_TICK, true);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* mage = GetTarget() ? GetTarget()->ToPlayer() : nullptr;
        if (!mage)
            return;

        mage->RemoveAurasDueToSpell(SPELL_MAGE_FROST_BLOCK_TICK);

        AuraApplication const* application = GetTargetApplication();
        if (!g_CustomSpellsEnabled || !mage->IsAlive() || !mage->IsInWorld()
            || !mage->HasAura(SPELL_MAGE_FROST_BLOCK_BURST_PASSIVE)
            || (application && application->GetRemoveMode() == AURA_REMOVE_BY_DEATH))
            return;

        int32 const damage = ParagonDamage(mage);
        mage->CastCustomSpell(mage, SPELL_MAGE_FROST_BLOCK_BURST, &damage, nullptr,
            nullptr, true);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_custom_mage_ice_block_barrage::HandleApply, EFFECT_0,
            SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_mage_ice_block_barrage::HandleRemove, EFFECT_0,
            SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_custom_mage_ice_block_tick : public AuraScript
{
    PrepareAuraScript(spell_custom_mage_ice_block_tick);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Player* mage = GetTarget() ? GetTarget()->ToPlayer() : nullptr;
        if (!mage || !g_CustomSpellsEnabled || !mage->IsAlive() || !mage->IsInWorld()
            || !mage->HasAura(SPELL_MAGE_ICE_BLOCK)
            || !mage->HasAura(SPELL_MAGE_FROST_BLOCK_LANCES_PASSIVE))
            return;

        uint32 const lance = HighestKnownRank(mage, SPELL_MAGE_ICE_LANCE_R1);
        if (!lance)
            return;

        std::list<Unit*> nearby;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(mage, mage, BLOCK_LANCE_RANGE);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(mage, nearby, check);
        Cell::VisitObjects(mage, searcher, BLOCK_LANCE_RANGE);

        float const facing = mage->GetOrientation();
        uint32 count = 0;
        for (Unit* enemy : nearby)
        {
            if (count >= BLOCK_LANCE_MAX_TARGETS)
                break;
            if (!enemy->IsAlive() || !mage->IsValidAttackTarget(enemy)
                || !mage->IsWithinLOSInMap(enemy))
                continue;

            mage->SetOrientation(mage->GetAngle(enemy));
            mage->CastSpell(enemy, lance, true);
            ++count;
        }
        mage->SetOrientation(facing);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_mage_ice_block_tick::HandlePeriodic, EFFECT_0,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  End Mage Frost section
// ============================================================

// ============================================================
//  MAGE ARCANE: 900714 Arcane Overflow
//  Concept: spending 10,000 mana auto-casts an empowered Arcane
//  Explosion and resets the cooldown of Blink. The mana each cast
//  costs (Spell::GetPowerCost at OnPlayerSpellCast) is summed per
//  player; every full 10,000 sets off 900718 (Arcane Explosion at
//  twice the damage, 10 yd round the mage) and resets Blink (1953).
// ============================================================
class ArcaneOverflowState : public DataMap::Base
{
public:
    uint32 Spent = 0;
};

class custom_mage_arcane_overflow_playerscript : public PlayerScript
{
public:
    custom_mage_arcane_overflow_playerscript()
        : PlayerScript("custom_mage_arcane_overflow_playerscript") { }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        // a triggered cast has its cost computed but pays none
        if (!player || !spell || !g_CustomSpellsEnabled || spell->IsTriggered()
            || spell->GetSpellInfo()->PowerType != POWER_MANA
            || spell->GetPowerCost() <= 0
            || !player->HasAura(SPELL_MAGE_ARC_OVERFLOW_PASSIVE))
            return;

        ArcaneOverflowState* state = player->CustomData.GetDefault<
            ArcaneOverflowState>("mod_custom_spells_arcane_overflow");
        state->Spent += uint32(spell->GetPowerCost());
        if (state->Spent < MAGE_OVERFLOW_MANA)
            return;

        state->Spent -= MAGE_OVERFLOW_MANA;
        player->CastSpell(player, SPELL_MAGE_ARC_OVERFLOW_EXPLOSION, true);
        player->RemoveSpellCooldown(SPELL_MAGE_BLINK, true);
    }
};

// ============================================================
//  MAGE ARCANE: 900715 Mirror Shield
//  Concept: getting struck while a shield is up has a chance to
//  spawn a mirror image. Proc (spell_proc: hits taken, 10 %, 6 s
//  cooldown) only while the mage carries an absorb shield of its
//  own (Mana Shield, Ice Barrier, Fire/Frost Ward): one Mirror Image
//  (58831).
// ============================================================
class spell_custom_mage_mirror_shield : public AuraScript
{
    PrepareAuraScript(spell_custom_mage_mirror_shield);

    bool CheckProc(ProcEventInfo& /*eventInfo*/)
    {
        Unit* mage = GetTarget();
        if (!g_CustomSpellsEnabled)
            return false;

        for (AuraType type : { SPELL_AURA_MANA_SHIELD, SPELL_AURA_SCHOOL_ABSORB })
            for (AuraEffect const* shield : mage->GetAuraEffectsByType(type))
                if (shield->GetCasterGUID() == mage->GetGUID())
                    return true;
        return false;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), SPELL_MAGE_MIRROR_IMAGE_ONE, true,
            nullptr, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_mage_mirror_shield::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_mage_mirror_shield::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  MAGE ARCANE: 900716 Mirror Images use area spells
//  Concept: mirror images are using the mage AoEs. Hooked on the
//  images' Frostbolt (59638) and Fire Blast (59637): with the
//  owner's marker, every hit also deals its damage to all other
//  enemies within 8 yd of the target (900717 Frost / 900719 Fire).
// ============================================================
class spell_custom_mage_mirror_splash : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_mirror_splash);

    void HandleAfterHit()
    {
        Unit* image = GetCaster();
        Unit* target = GetHitUnit();
        int32 damage = GetHitDamage();
        if (!image || !target || damage <= 0 || !g_CustomSpellsEnabled
            || image->GetEntry() != NPC_MAGE_MIRROR_IMAGE)
            return;

        Player* owner = image->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner || !owner->HasAura(SPELL_MAGE_ARC_MIRROR_SPLASH_PASSIVE))
            return;

        uint32 const splash = GetSpellInfo()->Id == SPELL_MIRROR_IMAGE_FIRE_BLAST
            ? SPELL_MAGE_ARC_MIRROR_SPLASH_FIRE : SPELL_MAGE_ARC_MIRROR_SPLASH_FROST;
        CastAnchoredBurst(image, target, splash, damage);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_mage_mirror_splash::HandleAfterHit);
    }
};

// ============================================================
//  MAGE FIRE: 900741 Meteor (active, picker)
//  Concept: "meteor spell". Ground-targeted like Comet Shower: every
//  enemy within 8 yd of the aimed point takes the impact (900742,
//  3,500-4,100 Fire) and burns for 600 over 6 sec (900743).
// ============================================================
class spell_custom_mage_meteor : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_meteor);

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        WorldLocation const* dest = GetExplTargetDest();
        if (!player || !dest || !g_CustomSpellsEnabled)
            return;

        float const range = player->GetDistance(*dest) + 9.0f;
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(player, player, range);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(player, targets, check);
        Cell::VisitObjects(player, searcher, range);

        for (Unit* target : targets)
        {
            if (!target->IsAlive() || !player->IsValidAttackTarget(target)
                || target->GetExactDist2d(dest) > 8.0f)
                continue;

            player->CastSpell(target, SPELL_MAGE_FIRE_METEOR_IMPACT, true);
            player->CastSpell(target, SPELL_MAGE_FIRE_METEOR_BURN, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_mage_meteor::HandleAfterCast);
    }
};

// ============================================================
//  MAGE FIRE: 900737 Fire Blast off the GCD
//  The passive's SPELLMOD_GLOBAL_COOLDOWN -1500 is clamped to the
//  core's 1 s minimum (Spell::TriggerGlobalCooldown). An instant spell
//  starts its global cooldown AFTER cast() in Spell::prepare, so an
//  AfterCast hook is too early; OnSpellPrepare runs after it. With the
//  marker, a Fire Blast's own global cooldown is cancelled there.
//  (A Fire Blast inside another spell's global cooldown stays
//  impossible: CheckCast and the client both refuse it.)
// ============================================================
class custom_mage_fire_blast_gcd_allspell : public AllSpellScript
{
public:
    custom_mage_fire_blast_gcd_allspell()
        : AllSpellScript("custom_mage_fire_blast_gcd_allspell",
            { ALLSPELLHOOK_ON_PREPARE }) { }

    void OnSpellPrepare(Spell* /*spell*/, Unit* caster,
        SpellInfo const* spellInfo) override
    {
        if (!caster || !g_CustomSpellsEnabled
            || spellInfo->SpellFamilyName != SPELLFAMILY_MAGE
            || sSpellMgr->GetFirstSpellInChain(spellInfo->Id) != SPELL_FIRE_BLAST_R1)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->HasAura(SPELL_MAGE_FIRE_FBLAST_GCD_PASSIVE))
            return;

        player->GetGlobalCooldownMgr().CancelGlobalCooldown(spellInfo);
    }
};

void AddMageSpellsScripts()
{
    // Mage Arcane
    RegisterSpellScript(spell_custom_mage_ae_charges);
    RegisterSpellScript(spell_custom_mage_evocation_power);
    RegisterSpellScript(spell_custom_mage_emergency_shield);
    RegisterSpellScript(spell_custom_mage_blink_to_target);
    RegisterSpellScript(spell_custom_mage_targeted_blink);
    new custom_mage_arcane_overflow_playerscript();
    RegisterSpellScript(spell_custom_mage_mirror_shield);
    RegisterSpellScript(spell_custom_mage_mirror_splash);
    RegisterSpellScript(spell_custom_mage_blink_image);
    new custom_mage_image_power_playerscript();

    // Mage Fire
    RegisterSpellScript(spell_custom_mage_pyro_hotstreak);
    new custom_mage_fire_blast_gcd_allspell();
    RegisterSpellScript(spell_custom_mage_meteor);
    RegisterSpellScript(spell_custom_mage_db_living_bomb);
    RegisterSpellScript(spell_custom_mage_molten_flamestrike);

    // Mage Frost
    RegisterSpellScript(spell_custom_mage_permanent_water_ele);
    RegisterSpellScript(spell_custom_mage_comet_shower);
    RegisterSpellScript(spell_custom_mage_ice_block_barrage);
    RegisterSpellScript(spell_custom_mage_ice_block_tick);
}
