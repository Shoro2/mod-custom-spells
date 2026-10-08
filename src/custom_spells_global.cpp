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
#include "TemporarySummon.h"

// ============================================================
//  NON-CLASS GLOBAL SPELLS (901100-901199)
//  These passive auras apply to ALL classes at Paragon level.
//  SpellFamilyName = 0 (Generic).
// ============================================================

// ============================================================
//  901100: Cast while moving
//  DBC passive aura using SPELL_AURA_CAST_WHILE_WALKING (330).
//  This is a pure DBC spell â€” no C++ needed. The aura type
//  allows casting all spells (including channels) while moving.
//  Registered here only for completeness; the DBC entry does
//  all the work.
// ============================================================
// (No C++ class needed â€” DBC aura 330 handles it natively)

// ============================================================
//  901101: Kill enemy â†’ heal 5% total HP
//  Proc aura: on PROC_FLAG_KILL, heals the player for 5% of
//  their maximum health via helper spell 901105.
// ============================================================
class spell_custom_global_kill_heal : public AuraScript
{
    PrepareAuraScript(spell_custom_global_kill_heal);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!g_CustomSpellsEnabled)
            return;

        int32 healAmount = static_cast<int32>(player->GetMaxHealth() * 0.05f);
        if (healAmount <= 0)
            return;

        HealInfo healInfo(player, player, uint32(healAmount), GetSpellInfo(),
            SPELL_SCHOOL_MASK_HOLY);
        player->HealBySpell(healInfo);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_global_kill_heal::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  901102: Attacks 25% chance to hit again (Extra Attack)
//  Proc aura: on melee auto attack or melee spell, 25% chance
//  to grant 1 extra attack. Uses SPELL_AURA_ADD_EXTRA_ATTACKS
//  in DBC for the melee part. C++ proc handler prevents
//  recursive procs by checking triggered flag.
// ============================================================
class spell_custom_global_extra_attack : public AuraScript
{
    PrepareAuraScript(spell_custom_global_extra_attack);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Prevent recursive procs: if the triggering event came from a
        // triggered spell (i.e. from an extra attack proc), do not proc again
        if (eventInfo.GetDamageInfo() && eventInfo.GetDamageInfo()->GetSpellInfo()
            && eventInfo.GetDamageInfo()->GetSpellInfo()->Id == SPELL_GLOBAL_EXTRA_ATTACK_HELPER)
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Grant 1 extra melee attack via helper spell (SPELL_EFFECT_ADD_EXTRA_ATTACKS)
        player->CastSpell(player, SPELL_GLOBAL_EXTRA_ATTACK_HELPER, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_global_extra_attack::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  901103: Spells/abilities 10% chance to hit all enemies
//  within 10yd of target.
//  Proc aura: on damage dealt, 10% chance to deal the same
//  damage to all enemies within 10yd via helper spell 901106.
//  1s ICD to prevent DoT-tick spam.
// ============================================================
class spell_custom_global_cleave_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_global_cleave_proc);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!g_CustomSpellsEnabled)
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Get original damage amount
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo)
            return;

        int32 damage = static_cast<int32>(damageInfo->GetDamage());
        if (damage <= 0)
            return;

        // Find all enemies within 10yd of the target
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, player, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(target, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        for (Unit* extra : targets)
        {
            if (!extra->IsAlive() || !player->IsValidAttackTarget(extra))
                continue;

            SpellInfo const* logSpell = damageInfo->GetSpellInfo()
                ? damageInfo->GetSpellInfo() : GetSpellInfo();
            SpellNonMeleeDamage dmgInfo(player, extra, logSpell, damageInfo->GetSchoolMask());
            dmgInfo.damage = damage;
            player->DealSpellDamage(&dmgInfo, true);
            player->SendSpellNonMeleeDamageLog(&dmgInfo);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_global_cleave_proc::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  901104: Avoid attack â†’ counter attack
//  Proc aura: on dodge/parry/block of a melee attack, deal
//  instant physical damage back to the attacker via helper
//  spell 901107. Damage = 50% of player's AP.
// ============================================================
class spell_custom_global_counter_attack : public AuraScript
{
    PrepareAuraScript(spell_custom_global_counter_attack);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->IsAlive())
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Must be an avoided attack: a miss, dodge, parry or block
        // (spell_proc HitMask 0x2074 lets exactly these through)
        uint32 hitMask = eventInfo.GetHitMask();
        if (!(hitMask & (PROC_HIT_MISS | PROC_HIT_DODGE | PROC_HIT_PARRY
            | PROC_HIT_BLOCK | PROC_HIT_FULL_BLOCK)))
            return;

        Unit* attacker = eventInfo.GetActor();
        if (!attacker || !attacker->IsAlive() || attacker == player)
            return;

        // Counter damage = 50% of AP
        int32 counterDamage = static_cast<int32>(
            player->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f);
        if (counterDamage <= 0)
            counterDamage = 100; // minimum floor

        SpellInfo const* spellInfo = GetSpellInfo();
        SpellNonMeleeDamage dmgInfo(player, attacker, spellInfo, SPELL_SCHOOL_MASK_NORMAL);
        dmgInfo.damage = counterDamage;
        player->DealSpellDamage(&dmgInfo, true);
        player->SendSpellNonMeleeDamageLog(&dmgInfo);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_global_counter_attack::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  Minion auras (shared by several classes)
//  A passive of the owner puts an aura on the owner's minions:
//  - 900435 Shaman "Summons +50%": every summon, totem and pet
//  - 900502 Hunter "Pet Damage +50%": the pet
//  - 900503 Hunter "Pet Speed +50%": the pet (+50 % melee haste)
//  - 900836 / 900839 Warlock: the Imp / the Felguard
//  The damage passives used to multiply inside UnitScript::OnDamage,
//  after the combat log line was sent, so the log, the floating
//  numbers and every damage meter showed the old value. An aura
//  (MOD_DAMAGE_PERCENT_DONE 901109, MOD_MELEE_HASTE 901110) works
//  through the core's damage and haste code and shows up everywhere.
//  Kept in sync once a second per owner; the auras are added and
//  removed only here.
// ============================================================
namespace
{
    struct MinionAuraRule
    {
        uint32 passive;
        uint32 aura;
        uint32 entry;   // 0 = every minion of the owner
    };

    MinionAuraRule const MINION_AURA_RULES[] =
    {
        { SPELL_ENH_SUMMON_DMG_PASSIVE,     SPELL_GLOBAL_MINION_DAMAGE, 0 },
        { SPELL_HUNT_BM_PET_DMG_PASSIVE,    SPELL_GLOBAL_MINION_DAMAGE, 0 },
        { SPELL_HUNT_BM_PET_SPEED_PASSIVE,  SPELL_GLOBAL_MINION_HASTE,  0 },
        { SPELL_WLK_DEMO_IMP_FB_DMG,        SPELL_GLOBAL_MINION_DAMAGE,
            NPC_IMP },
        { SPELL_WLK_DEMO_FG_DMG,            SPELL_GLOBAL_MINION_DAMAGE,
            NPC_FELGUARD },
    };

    // Custom summons made with SummonCreature: owned by the player, but
    // neither its pet nor in its controlled list
    uint32 const CUSTOM_MINION_ENTRIES[] =
    {
        NPC_CUSTOM_WOLF, NPC_CUSTOM_TREANT, NPC_LESSER_IMP,
        NPC_LESSER_FELGUARD, NPC_LESSER_VOIDWALKER
    };

    constexpr uint32 MINION_AURA_SYNC_MS = 1000;

    class MinionAuraState : public DataMap::Base
    {
    public:
        uint32 Timer = 0;
        bool Active = false;    // carried an aura at the last sync
    };

    void CollectMinions(Player* player, std::vector<Unit*>& minions)
    {
        for (Unit* controlled : player->m_Controlled)
            if (controlled && controlled->IsInWorld())
                minions.push_back(controlled);

        if (Pet* pet = player->GetPet())
            if (std::find(minions.begin(), minions.end(), pet)
                == minions.end())
                minions.push_back(pet);

        for (uint8 slot = SUMMON_SLOT_TOTEM_FIRE; slot < MAX_TOTEM_SLOT;
            ++slot)
            if (!player->m_SummonSlot[slot].IsEmpty())
                if (Creature* totem = player->GetMap()->GetCreature(
                    player->m_SummonSlot[slot]))
                    minions.push_back(totem);

        for (uint32 entry : CUSTOM_MINION_ENTRIES)
        {
            std::list<Creature*> found;
            player->GetCreatureListWithEntryInGrid(found, entry, 60.0f);
            for (Creature* creature : found)
                if (creature->GetOwnerGUID() == player->GetGUID()
                    || creature->GetCreatorGUID() == player->GetGUID())
                    minions.push_back(creature);
        }
    }
}

class custom_minion_aura_playerscript : public PlayerScript
{
public:
    custom_minion_aura_playerscript()
        : PlayerScript("custom_minion_aura_playerscript") { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (!player || !player->IsInWorld())
            return;

        MinionAuraState* state = player->CustomData.GetDefault<
            MinionAuraState>("mod_custom_spells_minion_auras");
        state->Timer += diff;
        if (state->Timer < MINION_AURA_SYNC_MS)
            return;
        state->Timer = 0;

        bool anyPassive = false;
        for (MinionAuraRule const& rule : MINION_AURA_RULES)
            if (player->HasAura(rule.passive))
                anyPassive = true;
        if (player->HasAura(SPELL_DRST_SUMMON_SCALE_PASSIVE)
            || player->HasAura(SPELL_DRST_SUMMON_HEAL_PASSIVE))
            anyPassive = true;

        // Nothing to add and nothing left to take away: the common case
        if ((!anyPassive || !g_CustomSpellsEnabled) && !state->Active)
            return;

        std::vector<Unit*> minions;
        CollectMinions(player, minions);

        bool active = false;
        for (Unit* minion : minions)
        {
            if (!minion->IsAlive())
                continue;

            for (uint32 aura : { uint32(SPELL_GLOBAL_MINION_DAMAGE),
                uint32(SPELL_GLOBAL_MINION_HASTE) })
            {
                bool wanted = false;
                if (g_CustomSpellsEnabled)
                    for (MinionAuraRule const& rule : MINION_AURA_RULES)
                        if (rule.aura == aura
                            && (!rule.entry || rule.entry == minion->GetEntry())
                            && player->HasAura(rule.passive))
                            wanted = true;

                bool const has = minion->HasAura(aura);
                if (wanted && !has)
                    minion->AddAura(aura, minion);
                else if (!wanted && has)
                    minion->RemoveAurasDueToSpell(aura);
                active |= wanted;
            }
        }
        // 901067 Summons: Healing Power - every summon gains 10 health per
        // point of the druid's healing power (helper 901074, aura 34, its
        // amount kept current). The old code set the summon's maximum to
        // its create health plus the bonus, which threw away the core's
        // stamina scaling (a Force of Nature treant lost 750 health, bot
        // run 426) and never reached the Healing Treant.
        int32 const healthBonus = g_CustomSpellsEnabled
            && player->HasAura(SPELL_DRST_SUMMON_SCALE_PASSIVE)
            ? int32(player->SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL)) * 10
            : 0;
        for (Unit* minion : minions)
        {
            if (!minion->IsAlive())
                continue;

            Aura* bond = minion->GetAura(SPELL_DRST_SUMMON_HEALTH_AURA);
            if (healthBonus <= 0)
            {
                if (bond)
                    minion->RemoveAura(bond);
                continue;
            }

            active = true;
            if (!bond)
                minion->CastCustomSpell(minion, SPELL_DRST_SUMMON_HEALTH_AURA,
                    &healthBonus, nullptr, nullptr, true);
            else if (AuraEffect* effect = bond->GetEffect(EFFECT_0))
                if (effect->GetAmount() != healthBonus)
                    effect->ChangeAmount(healthBonus);
        }
        // 901068 Parting Bloom, the despawn half: every timed summon gets the
        // fuse 901075 (custom_spells_druid.cpp), which ends 0.3 s before the
        // summon's timer - also a summon older than the passive.
        if (g_CustomSpellsEnabled && player->HasAura(SPELL_DRST_SUMMON_HEAL_PASSIVE))
            for (Unit* minion : minions)
            {
                TempSummon* summon = minion->IsAlive() && minion->IsSummon()
                    ? minion->ToTempSummon() : nullptr;
                if (!summon || summon->HasAura(SPELL_DRST_PARTING_BLOOM_FUSE)
                    || (summon->GetSummonType() != TEMPSUMMON_TIMED_DESPAWN
                        && summon->GetSummonType() != TEMPSUMMON_TIMED_OR_DEAD_DESPAWN
                        && summon->GetSummonType() != TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN))
                    continue;

                uint32 const left = summon->GetTimer();
                if (left <= 1000)
                    continue;

                if (Aura* fuse = summon->AddAura(SPELL_DRST_PARTING_BLOOM_FUSE, summon))
                {
                    fuse->SetMaxDuration(int32(left - 300));
                    fuse->SetDuration(int32(left - 300));
                }
                active = true;
            }

        state->Active = active;
    }
};

// ============================================================
//  Mana regeneration by missing mana (shared by three classes)
//  900467 Shaman Resto, 900700 Mage Arcane, 901072 Druid Resto.
//  Concept: "increases your mana regen per missing mana % by 2%":
//  at 50 % mana the regeneration doubles, at 10 % it is 2.8 times
//  as high. The bonus scales the regeneration the core is giving
//  the player right now (casting -> the interrupted rate, as in
//  Player::Regenerate), so a passive with no base regeneration adds
//  nothing. The old code added 2 % of the MISSING mana every 5 sec
//  instead, about 60 mana per 5 sec at half mana.
// ============================================================
namespace
{
    constexpr uint32 MANA_REGEN_PASSIVES[] =
    {
        SPELL_RST_MANA_REGEN_PASSIVE, SPELL_MAGE_ARC_MANA_REGEN,
        SPELL_DRST_MANA_REGEN_PASSIVE
    };

    constexpr float MANA_REGEN_PCT_PER_MISSING_PCT = 2.0f;
    constexpr uint32 MANA_REGEN_TICK_MS = 1000;

    class ManaRegenState : public DataMap::Base
    {
    public:
        uint32 Timer = 0;
        float Carry = 0.0f;     // the fraction of a mana point left over
    };
}

class custom_mana_regen_playerscript : public PlayerScript
{
public:
    custom_mana_regen_playerscript()
        : PlayerScript("custom_mana_regen_playerscript") { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        if (!player || !player->IsAlive() || !g_CustomSpellsEnabled)
            return;

        bool has = false;
        for (uint32 passive : MANA_REGEN_PASSIVES)
            if (player->HasAura(passive))
                has = true;
        if (!has)
            return;

        ManaRegenState* state = player->CustomData.GetDefault<
            ManaRegenState>("mod_custom_spells_mana_regen");
        state->Timer += diff;
        if (state->Timer < MANA_REGEN_TICK_MS)
            return;
        uint32 const elapsed = state->Timer;
        state->Timer = 0;

        uint32 const maxMana = player->GetMaxPower(POWER_MANA);
        uint32 const curMana = player->GetPower(POWER_MANA);
        if (!maxMana || curMana >= maxMana)
            return;

        float const missingPct =
            100.0f * float(maxMana - curMana) / float(maxMana);
        uint16 const field = player->IsUnderLastManaUseEffect()
            ? UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER
            : UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER;
        float const perSecond = player->GetFloatValue(
            field + AsUnderlyingType(POWER_MANA))
            * sWorld->getRate(RATE_POWER_MANA);
        if (perSecond <= 0.0f)
            return;

        float const bonus = perSecond * (elapsed / 1000.0f)
            * MANA_REGEN_PCT_PER_MISSING_PCT * missingPct / 100.0f
            + state->Carry;
        int32 const whole = int32(bonus);
        state->Carry = bonus - float(whole);
        if (whole > 0)
            player->ModifyPower(POWER_MANA, whole);
    }
};

// ============================================================
//  End Non-Class Global section
// ============================================================

void AddGlobalSpellsScripts()
{
    // 901100: Cast while moving - not offered: the 3.3.5 client itself
    // refuses to cast while moving, the server cannot change that
    RegisterSpellScript(spell_custom_global_kill_heal);
    RegisterSpellScript(spell_custom_global_extra_attack);
    RegisterSpellScript(spell_custom_global_cleave_proc);
    RegisterSpellScript(spell_custom_global_counter_attack);

    // Shared mechanics behind class passives
    new custom_minion_aura_playerscript();
    new custom_mana_regen_playerscript();
}
