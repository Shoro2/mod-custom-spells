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
//  DRUID BALANCE: Moonfire +9 targets (901000)
//  Hooked on Moonfire (all ranks via -48463). After hitting
//  the main target, finds up to 9 additional enemies in 10yd
//  and applies Moonfire to each.
// ============================================================
class spell_custom_bal_mf_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_bal_mf_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_BAL_MF_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

            // Cast Moonfire on each additional target (triggered, no cost/GCD)
            caster->CastSpell(target, SPELL_MOONFIRE_R14, true);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_bal_mf_aoe::HandleAfterHit);
    }
};

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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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
//  DRUID RESTO: Summons scale with healing power (901067)
//  PlayerScript: periodically check player's healing power
//  and apply it to any owned treants as bonus damage/health.
// ============================================================
class custom_druid_summon_scale_playerscript : public PlayerScript
{
public:
    custom_druid_summon_scale_playerscript() : PlayerScript("custom_druid_summon_scale_playerscript") {}

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_DRST_SUMMON_SCALE_PASSIVE))
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

        // Get player's bonus healing (spell power)
        int32 healPower = player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ALL);

        // Scale all owned summons
        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            Unit* controlled = *itr;
            if (!controlled || !controlled->IsAlive())
                continue;

            Creature* summon = controlled->ToCreature();
            if (!summon)
                continue;

            // Scale health based on healing power (1 spell power = 10 HP)
            uint32 bonusHP = static_cast<uint32>(healPower * 10);
            uint32 baseHP = summon->GetCreateHealth();
            uint32 desiredHP = baseHP + bonusHP;
            if (summon->GetMaxHealth() != desiredHP)
            {
                summon->SetMaxHealth(desiredHP);
                summon->SetHealth(desiredHP);
            }
        }
    }
};

// ============================================================
//  DRUID RESTO: Summons heal on death/despawn (901068)
//  UnitScript: when a creature with a druid owner dies,
//  heal all nearby allies for a burst amount.
// ============================================================
class custom_druid_summon_heal_unitscript : public UnitScript
{
public:
    custom_druid_summon_heal_unitscript() : UnitScript("custom_druid_summon_heal_unitscript") {}

    void OnUnitDeath(Unit* unit) override
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Cast heal helper on self (AoE heal around summon's position)
        summon->CastSpell(summon, SPELL_DRST_TREANT_HEAL_HELPER, true);
    }
};

// ============================================================
//  DRUID RESTO: Thorns has chance to cast Rejuv (901069)
//  UnitScript: when damage is dealt to a player with
//  Thorns and the passive, 20% chance to cast Rejuv on self.
// ============================================================
class custom_druid_thorns_rejuv_unitscript : public UnitScript
{
public:
    custom_druid_thorns_rejuv_unitscript() : UnitScript("custom_druid_thorns_rejuv_unitscript") {}

    void OnDamage(Unit* /*attacker*/, Unit* victim, uint32& /*damage*/) override
    {
        if (!victim || !victim->IsAlive())
            return;

        Player* player = victim->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_DRST_THORNS_REJUV_PASSIVE))
            return;

        // Must have Thorns active
        if (!player->HasAura(SPELL_THORNS_R8))
        {
            // Check all ranks of Thorns (SpellFamilyName=7, flags vary)
            bool hasThorns = false;
            for (auto const& aura : player->GetAppliedAuras())
            {
                SpellInfo const* si = aura.second->GetBase()->GetSpellInfo();
                if (si && si->SpellFamilyName == SPELLFAMILY_DRUID_ID &&
                    si->SpellIconID == 53) // Thorns icon
                {
                    hasThorns = true;
                    break;
                }
            }
            if (!hasThorns)
                return;
        }

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // 20% chance
        if (!roll_chance_i(20))
            return;

        // Cast Rejuvenation on self (triggered)
        player->CastSpell(player, SPELL_REJUV_R15, true);
    }
};

// ============================================================
//  DRUID RESTO: HoTs tick 2x fast + 2x duration (901071)
//  This is a DBC-only approach where possible.
//  However, combining double tick speed with double duration
//  needs two separate DBC auras. We use marker + DBC auras:
//  - One aura halves tick interval (ADD_PCT_MODIFIER on SPELL_AURA_PERIODIC)
//  - One aura doubles duration (ADD_PCT_MODIFIER on duration)
//  Both handled via DBC — no C++ needed for this spell.
// ============================================================
// (No C++ class needed — pure DBC passive)

// ============================================================
//  DRUID RESTO: Mana regen per missing mana% (901072)
//  PlayerScript: same pattern as Shaman Resto (900467).
//  Every 5 seconds, regenerate mana based on missing%.
// ============================================================
class custom_druid_mana_regen_playerscript : public PlayerScript
{
public:
    custom_druid_mana_regen_playerscript() : PlayerScript("custom_druid_mana_regen_playerscript") {}

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_DRST_MANA_REGEN_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Throttle: only every 5 seconds
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        static std::unordered_map<ObjectGuid, uint32> s_lastRegen;
        ObjectGuid guid = player->GetGUID();
        if (s_lastRegen.count(guid) && (now - s_lastRegen[guid]) < 5)
            return;
        s_lastRegen[guid] = now;

        uint32 maxMana = player->GetMaxPower(POWER_MANA);
        uint32 curMana = player->GetPower(POWER_MANA);
        if (maxMana == 0)
            return;

        float missingPct = 100.0f * (1.0f - static_cast<float>(curMana) / maxMana);
        int32 bonus = static_cast<int32>(maxMana * (missingPct * 0.02f) / 100.0f);
        if (bonus > 0)
            player->EnergizeBySpell(player, SPELL_DRST_MANA_REGEN_PASSIVE,
                bonus, POWER_MANA);
    }
};

// ============================================================
//  End Druid section
// ============================================================

void AddDruidSpellsScripts()
{
    // Druid Balance
    RegisterSpellScript(spell_custom_bal_mf_aoe);
    RegisterSpellScript(spell_custom_bal_sf_cd_reduce);

    // Druid Feral
    RegisterSpellScript(spell_custom_feral_bear_swipe_bleed);
    RegisterSpellScript(spell_custom_feral_cat_swipe_bleed);

    // Druid Resto
    RegisterSpellScript(spell_custom_drst_hot_treant);
    new custom_druid_summon_scale_playerscript();
    new custom_druid_summon_heal_unitscript();
    new custom_druid_thorns_rejuv_unitscript();
    new custom_druid_mana_regen_playerscript();
}
