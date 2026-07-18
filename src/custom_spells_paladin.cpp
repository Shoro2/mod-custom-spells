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
#include "DynamicObject.h"

// Re-entrancy guard for the "Holy Shock always both" pair (passive 900202).
// The damage variant triggers a heal and the heal variant triggers damage;
// both helper spells fall inside the hooked Holy Shock rank chains, so without
// this guard the two scripts re-trigger each other without bound (stack overflow).
static thread_local bool s_hsBothReentry = false;

// ============================================================
//  SPELL 900200: Holy Shock AoE Damage (SpellScript)
//  Hooked on Holy Shock damage spells (48824 etc.).
//  After hitting a hostile target, casts AoE holy damage
//  helper (900208) centered on the target.
//  Only active when player has passive 900200.
// ============================================================
class spell_custom_holy_hs_aoe_dmg : public SpellScript
{
    PrepareSpellScript(spell_custom_holy_hs_aoe_dmg);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_HS_AOE_DMG_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        caster->CastSpell(target, SPELL_HOLY_HS_AOE_DMG_HELPER, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Holy Shock AoE damage on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_holy_hs_aoe_dmg::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900201: Holy Shock AoE Heal (SpellScript)
//  Hooked on Holy Shock heal spells (48825 etc.).
//  After healing a friendly target, casts AoE holy heal
//  helper (900209) centered on the target.
//  Only active when player has passive 900201.
// ============================================================
class spell_custom_holy_hs_aoe_heal : public SpellScript
{
    PrepareSpellScript(spell_custom_holy_hs_aoe_heal);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_HS_AOE_HEAL_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        caster->CastSpell(target, SPELL_HOLY_HS_AOE_HEAL_HELPER, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Holy Shock AoE heal on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_holy_hs_aoe_heal::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900202: Holy Shock Always Both (SpellScript)
//  Hooked on Holy Shock damage (48824) and heal (48825).
//  After hitting a hostile target → also heal nearest ally.
//  After healing a friendly target → also damage nearest enemy.
//  Only active when player has passive 900202.
// ============================================================
class spell_custom_holy_hs_both_dmg : public SpellScript
{
    PrepareSpellScript(spell_custom_holy_hs_both_dmg);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_HS_BOTH_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Skip when this hit came from the paired follow-up cast (avoid recursion)
        if (s_hsBothReentry)
            return;

        // We're on the damage spell → also heal nearest injured ally
        // Find nearest friendly unit within 40yd that's injured
        Unit* healTarget = nullptr;
        float minDist = 40.0f;

        Group* group = player->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || member->GetGUID() == player->GetGUID())
                    continue;
                if (member->GetHealthPct() >= 100.0f)
                    continue;
                float dist = player->GetDistance(member);
                if (dist < minDist)
                {
                    minDist = dist;
                    healTarget = member;
                }
            }
        }

        // Fallback: heal self if no group or no injured members
        if (!healTarget && player->GetHealthPct() < 100.0f)
            healTarget = player;

        if (healTarget)
        {
            s_hsBothReentry = true;
            caster->CastSpell(healTarget, SPELL_HOLY_SHOCK_HEAL_R7, true);
            s_hsBothReentry = false;
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> HS Both: auto-heal {}",
                player->GetName(), healTarget->GetName());
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_holy_hs_both_dmg::HandleAfterHit);
    }
};

class spell_custom_holy_hs_both_heal : public SpellScript
{
    PrepareSpellScript(spell_custom_holy_hs_both_heal);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_HOLY_HS_BOTH_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Skip when this hit came from the paired follow-up cast (avoid recursion)
        if (s_hsBothReentry)
            return;

        // We're on the heal spell → also damage nearest enemy
        Unit* dmgTarget = player->GetVictim();
        if (!dmgTarget || !dmgTarget->IsAlive())
        {
            // Find nearest hostile within 40yd
            dmgTarget = player->SelectNearbyTarget(nullptr, 40.0f);
        }

        if (dmgTarget)
        {
            caster->CastSpell(dmgTarget, SPELL_HOLY_SHOCK_DMG_R7, true);
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> HS Both: auto-damage {}",
                player->GetName(), dmgTarget->GetName());
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_holy_hs_both_heal::HandleAfterHit);
    }
};


// ============================================================
//  SPELL 900239: Avenger's Shield Leaves Consecration
//  Hooked on Avenger's Shield (48827). After hitting each
//  target, casts Consecration at the target's position.
//  Only active when player has passive 900239.
// ============================================================
class spell_custom_pprot_as_consec : public SpellScript
{
    PrepareSpellScript(spell_custom_pprot_as_consec);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_PPROT_AS_CONSEC_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Cast Consecration (triggered) at target's location
        caster->CastSpell(target, SPELL_CONSECRATION_R8, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> AS left Consecration on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_pprot_as_consec::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900240: Judgement → Free Avenger's Shield
//  Hooked on Judgement Damage (54158). After Judgement hits,
//  auto-casts Avenger's Shield at the same target.
//  Only active when player has passive 900240.
// ============================================================
class spell_custom_pprot_judge_as : public SpellScript
{
    PrepareSpellScript(spell_custom_pprot_judge_as);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_PPROT_JUDGE_AS_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        caster->CastSpell(target, SPELL_AVENGERS_SHIELD_R3, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Judgement triggered free AS on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_pprot_judge_as::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900205/900234/900268: Consecration Around You
//  Hooked on Consecration (all ranks via -26573). With one of
//  the marker passives, the static consecration area is removed
//  right after the cast and replaced by self-aura 900211, which
//  carries the cast rank's per-tick damage in its amount.
// ============================================================
class spell_custom_consec_around : public SpellScript
{
    PrepareSpellScript(spell_custom_consec_around);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        bool around = player->HasAura(SPELL_HOLY_CONSEC_AROUND_PASSIVE)
            || player->HasAura(SPELL_PPROT_CONSEC_AROUND_PASSIVE)
            || player->HasAura(SPELL_RET_CONSEC_AROUND_PASSIVE);
        bool heal = player->HasAura(SPELL_HOLY_CONSEC_HEAL_PASSIVE);
        if (!around && !heal)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 tick = GetSpellInfo()->Effects[EFFECT_0].CalcValue(caster);
        if (tick <= 0)
            return;

        // tickers must match the consecration duration the player actually
        // has (900207 extends it via SPELLMOD_DURATION)
        int32 duration = GetSpellInfo()->GetMaxDuration();
        player->ApplySpellMod(GetSpellInfo()->Id, SPELLMOD_DURATION, duration);

        auto applyTicker = [&](uint32 spellId)
        {
            caster->CastCustomSpell(caster, spellId, &tick, nullptr, nullptr, true);
            if (Aura* aura = player->GetAura(spellId))
            {
                if (duration > 0)
                {
                    aura->SetMaxDuration(duration);
                    aura->SetDuration(duration);
                }
            }
        };

        if (around)
        {
            caster->RemoveDynObject(GetSpellInfo()->Id);
            applyTicker(SPELL_CONSEC_MOBILE_AURA);
        }
        if (heal)
            applyTicker(SPELL_CONSEC_MOBILE_HEAL_AURA);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Consecration tickers ({}/tick, "
            "around={}, heal={})",
            player->GetName(), tick, around, heal);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_consec_around::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900211: Mobile Consecration (AuraScript)
//  Periodic dummy self-aura (1s ticks, 8s). Each tick deals the
//  carried amount as holy damage to enemies within 8yd of the
//  paladin, logged as Consecration so the client renders it.
// ============================================================
class spell_custom_mobile_consec : public AuraScript
{
    PrepareAuraScript(spell_custom_mobile_consec);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 amount = aurEff->GetAmount();
        if (amount <= 0)
            return;

        SpellInfo const* consecInfo = sSpellMgr->GetSpellInfo(SPELL_CONSECRATION_R8);
        if (!consecInfo)
            return;

        // Ground visual: a short-lived dynobj tagged with the real Consecration
        // id makes the client draw the circle at the current position each tick
        DynamicObject* visual = new DynamicObject();
        if (!visual->CreateDynamicObject(
            player->GetMap()->GenerateLowGuid<HighGuid::DynamicObject>(),
            player, consecInfo->Id, *player, 8.0f, DYNAMIC_OBJECT_AREA_SPELL))
            delete visual;
        else
            visual->SetDuration(1100);

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(player, player, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(player, targets, check);
        Cell::VisitObjects(player, searcher, 8.0f);

        for (Unit* enemy : targets)
        {
            if (!enemy->IsAlive() || !player->IsValidAttackTarget(enemy))
                continue;

            uint32 damage = player->SpellDamageBonusDone(enemy, consecInfo,
                uint32(amount), DOT, EFFECT_0);
            SpellNonMeleeDamage dmgInfo(player, enemy, consecInfo,
                consecInfo->GetSchoolMask());
            dmgInfo.damage = damage;
            player->DealSpellDamage(&dmgInfo, true);
            player->SendSpellNonMeleeDamageLog(&dmgInfo);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_mobile_consec::HandlePeriodic,
            EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  SPELL 900212: Mobile Consecration Heal (AuraScript)
//  Heal ticker for marker 900204: while Consecration runs, each
//  tick heals allies within 8yd of the paladin for the same
//  per-tick value the damage side uses, logged as Consecration.
// ============================================================
class spell_custom_mobile_consec_heal : public AuraScript
{
    PrepareAuraScript(spell_custom_mobile_consec_heal);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 amount = aurEff->GetAmount();
        if (amount <= 0)
            return;

        SpellInfo const* consecInfo = sSpellMgr->GetSpellInfo(SPELL_CONSECRATION_R8);
        if (!consecInfo)
            return;

        std::list<Unit*> allies;
        Acore::AnyFriendlyUnitInObjectRangeCheck check(player, player, 8.0f);
        Acore::UnitListSearcher<Acore::AnyFriendlyUnitInObjectRangeCheck>
            searcher(player, allies, check);
        Cell::VisitObjects(player, searcher, 8.0f);

        for (Unit* ally : allies)
        {
            if (!ally->IsAlive())
                continue;

            uint32 heal = player->SpellHealingBonusDone(ally, consecInfo,
                uint32(amount), DOT, EFFECT_0);
            HealInfo healInfo(player, ally, heal, consecInfo,
                consecInfo->GetSchoolMask());
            player->HealBySpell(healInfo);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_mobile_consec_heal::HandlePeriodic,
            EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  SPELL 900270: Divine Storm +6 Targets (SpellScript)
//  Hooked on Divine Storm itself (53385 - in this core the
//  weapon damage IS 53385; the heal runs via 54171/54172 in the
//  core's own script, which coexists on the same binding).
//  SPELLMOD_JUMP_TARGETS only extends chains, so the area cap is
//  bypassed like Multi-Shot: once per cast, deal the same damage
//  to up to 6 further enemies within 8yd of the paladin.
// ============================================================
class spell_custom_ret_ds_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_ret_ds_aoe);

    // 53385 hits several targets and AfterHit runs per target - including
    // the caster himself (effects 1+2 are self-targeted value dummies), so
    // the guard may only be consumed by a real enemy hit
    bool _done = false;

    void HandleAfterHit()
    {
        if (_done)
            return;

        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (mainTarget == caster || !player->IsValidAttackTarget(mainTarget))
            return;

        if (!player->HasAura(SPELL_RET_DS_TARGETS_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        _done = true;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(player, player, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(player, targets, check);
        Cell::VisitObjects(player, searcher, 8.0f);
        targets.remove(mainTarget);

        SpellInfo const* spellInfo = GetSpellInfo();

        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= 6)
                break;
            if (!target->IsAlive() || !player->IsValidAttackTarget(target))
                continue;

            SpellNonMeleeDamage dmgInfo(player, target, spellInfo, spellInfo->GetSchoolMask());
            dmgInfo.damage = damage;
            player->DealSpellDamage(&dmgInfo, true);
            player->SendSpellNonMeleeDamageLog(&dmgInfo);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ret_ds_aoe::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900274: Exorcism Buff System (AuraScript)
//  Passive proc aura: when CS, Judgement, or Divine Storm
//  hits an enemy, adds 1 stack of Exorcism buff (900275).
//  Max 10 stacks. Each stack = +50% Exorcism damage (DBC).
//  Stacks are consumed when Exorcism is cast.
// ============================================================
class spell_custom_ret_exorcism_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_ret_exorcism_proc);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        // Only proc on CS (35395), Judgement Damage (54158), or DS (53385)
        uint32 id = spellInfo->Id;
        return id == SPELL_CRUSADER_STRIKE_R6
            || id == SPELL_JUDGEMENT_DAMAGE
            || id == SPELL_DIVINE_STORM;
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* target = GetTarget();
        if (!target)
            return;

        Player* player = target->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Add one stack of the Exorcism buff (max 10)
        player->CastSpell(player, SPELL_RET_EXORCISM_BUFF, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Exorcism buff stack added (now {})",
            player->GetName(), player->GetAuraCount(SPELL_RET_EXORCISM_BUFF));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_ret_exorcism_proc::CheckProc);
        OnProc += AuraProcFn(spell_custom_ret_exorcism_proc::HandleProc);
    }
};

// ============================================================
//  Exorcism Consume: Hooked on Exorcism (48801).
//  After casting Exorcism, consumes all stacks of 900275.
//  Only active when player has the buff.
// ============================================================
class spell_custom_ret_exorcism_consume : public SpellScript
{
    PrepareSpellScript(spell_custom_ret_exorcism_consume);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(SPELL_RET_EXORCISM_BUFF))
        {
            uint32 stacks = caster->GetAuraCount(SPELL_RET_EXORCISM_BUFF);
            caster->RemoveAurasDueToSpell(SPELL_RET_EXORCISM_BUFF);

            if (Player* player = caster->ToPlayer())
                LOG_INFO("module",
                    "mod-custom-spells: Player {} -> Exorcism consumed {} stacks",
                    player->GetName(), stacks);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_ret_exorcism_consume::HandleAfterCast);
    }
};

void AddPaladinSpellsScripts()
{
    // Paladin Holy
    RegisterSpellScript(spell_custom_holy_hs_aoe_dmg);
    RegisterSpellScript(spell_custom_holy_hs_aoe_heal);
    RegisterSpellScript(spell_custom_holy_hs_both_dmg);
    RegisterSpellScript(spell_custom_holy_hs_both_heal);

    // Paladin Prot
    RegisterSpellScript(spell_custom_pprot_as_consec);
    RegisterSpellScript(spell_custom_pprot_judge_as);

    // Shared: Consecration follows the caster (Holy/Prot/Ret markers)
    RegisterSpellScript(spell_custom_consec_around);
    RegisterSpellScript(spell_custom_mobile_consec);
    RegisterSpellScript(spell_custom_mobile_consec_heal);

    // Paladin Ret
    RegisterSpellScript(spell_custom_ret_ds_aoe);
    RegisterSpellScript(spell_custom_ret_exorcism_proc);
    RegisterSpellScript(spell_custom_ret_exorcism_consume);
}
