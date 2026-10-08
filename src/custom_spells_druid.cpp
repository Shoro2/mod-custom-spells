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

void AddDruidSpellsScripts()
{
    // Druid Balance
    RegisterSpellScript(spell_custom_bal_sf_cd_reduce);
    RegisterSpellScript(spell_custom_bal_starfall_stack);
    RegisterSpellScript(spell_custom_bal_starfall_stack_volley);

    // Druid Feral
    RegisterSpellScript(spell_custom_feral_bear_swipe_bleed);
    RegisterSpellScript(spell_custom_feral_cat_swipe_bleed);
    RegisterSpellScript(spell_custom_feral_maul_bleed);

    // Druid Resto
    RegisterSpellScript(spell_custom_drst_hot_treant);
    new custom_druid_summon_heal_unitscript();
    RegisterSpellScript(spell_custom_drst_parting_bloom_fuse);
    new custom_druid_thorns_rejuv_unitscript();
}
