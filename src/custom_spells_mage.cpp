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

    // Mage Fire
    RegisterSpellScript(spell_custom_mage_pyro_hotstreak);
    new custom_mage_fire_blast_gcd_allspell();
    RegisterSpellScript(spell_custom_mage_meteor);

    // Mage Frost
    RegisterSpellScript(spell_custom_mage_permanent_water_ele);
    RegisterSpellScript(spell_custom_mage_comet_shower);
}
