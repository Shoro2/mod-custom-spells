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

// Re-entrancy guard for the Lava Burst hooks (overload / charges / spread-FS).
// The Lightning-Overload-on-Lava-Burst effect (900403) recasts Lava Burst; this
// guard stops that triggered copy from re-running the Lava Burst custom scripts,
// which would otherwise cascade into extra casts and cooldown resets.
static thread_local bool s_lvbReentry = false;

namespace
{
    constexpr int32 PARAGON_DAMAGE_BASE      = 666;
    constexpr int32 PARAGON_DAMAGE_PER_LEVEL = 5;

    // third concept list 2026-10-31 (wave 2)
    // 900445 Ghost Wolf: how long after the form ends the next hit still
    // erupts. A cancelled Ghost Wolf is followed by the spell or swing that
    // caused the cancel: Lightning Bolt's 2-2.5 s cast and its missile take
    // up to 3.5 s, so the window is 4 s.
    constexpr int32 GHOST_ERUPTION_WINDOW_MS = 4000;
    // 900468: "a random target in range" = a random enemy in combat within
    // this distance of the shaman (Lightning Bolt's own range)
    constexpr float HEAL_STRIKE_RANGE        = 30.0f;
    // family 11 flags of the four hooked heals
    constexpr uint32 FAMILYFLAG_CHAIN_HEAL   = 0x100;   // flags[0]; Healing Wave 0x40, Lesser Healing Wave 0x80
    constexpr uint32 FAMILYFLAG2_RIPTIDE     = 0x10;    // flags[2]

    // 666 + 5 x the shaman's Paragon level, the amount of the Nature bursts.
    int32 ParagonDamage(Player* player)
    {
        return PARAGON_DAMAGE_BASE
            + PARAGON_DAMAGE_PER_LEVEL * int32(GetParagonLevel(player));
    }

    // The highest rank of a spell chain the player knows (0: none). The
    // argument is the chain's first rank.
    uint32 HighestKnownRank(Player* player, uint32 firstRank)
    {
        uint32 known = 0;
        for (uint32 rank = firstRank; rank;
            rank = sSpellMgr->GetNextSpellInChain(rank))
            if (player->HasSpell(rank))
                known = rank;
        return known;
    }

    // A random living enemy within `range` of `origin` that is already in
    // combat (an idle mob is never pulled), that the player may attack and
    // sees (the priest's and the warlock's pick).
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
}

// ============================================================
//  SPELL 900401: Totems -> Following Creatures (PlayerScript)
//  When player has 900401, totems follow the player instead
//  of being static. Checked every 2 seconds via OnUpdate.
// ============================================================
class custom_totem_follow_playerscript : public PlayerScript
{
public:
    custom_totem_follow_playerscript() : PlayerScript("custom_totem_follow_playerscript") {}

    void OnPlayerLogout(Player* player) override
    {
        if (player)
            _lastCheck.erase(player->GetGUID());
    }

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        // Check for any of the three totem follow passives (Ele/Enh/Resto)
        if (!player->HasAura(SPELL_ELE_TOTEM_FOLLOW_PASSIVE) &&
            !player->HasAura(SPELL_ENH_TOTEM_FOLLOW_PASSIVE) &&
            !player->HasAura(SPELL_RST_TOTEM_FOLLOW_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Throttle: only check every ~2 seconds using game time
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        ObjectGuid guid = player->GetGUID();
        if (_lastCheck.count(guid) && (now - _lastCheck[guid]) < 2)
            return;
        _lastCheck[guid] = now;

        for (uint8 i = 0; i < MAX_TOTEM_SLOT; ++i)
        {
            if (!player->m_SummonSlot[i])
                continue;

            Creature* totem = player->GetMap()->GetCreature(player->m_SummonSlot[i]);
            if (!totem || !totem->IsAlive())
                continue;

            // Teleport totem near player if too far away
            float dist = totem->GetDistance(player);
            if (dist > 5.0f)
            {
                float x, y, z;
                player->GetClosePoint(x, y, z, totem->GetCombatReach(),
                    PET_FOLLOW_DIST + i * 1.5f, (M_PI / 2) * i);
                totem->NearTeleportTo(x, y, z, player->GetOrientation());
            }
        }
    }

private:
    std::unordered_map<ObjectGuid, uint32> _lastCheck;
};

// ============================================================
//  SPELL 900402: Fire Elemental -> Ragnaros
//  Hooked on the Fire Elemental Totem's own spell (32982), which
//  the totem casts once it stands and which summons the Fire
//  Elemental (NPC 15438). The shaman's Fire Elemental Totem cast
//  (2894) ends before that, so the elemental cannot be found there
//  (the old hook on 2894 never saw one). The elemental takes on
//  Ragnaros' model and has twice the health.
// ============================================================
class spell_custom_ele_ragnaros : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_ragnaros);

    void HandleAfterCast()
    {
        Unit* totem = GetCaster();
        if (!totem || !g_CustomSpellsEnabled)
            return;

        Player* player = totem->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!player || !player->HasAura(SPELL_ELE_RAGNAROS_PASSIVE))
            return;

        constexpr uint32 RAGNAROS_DISPLAY_ID = 11121;
        constexpr float  RAGNAROS_SCALE      = 0.35f;

        std::list<Creature*> elementals;
        totem->GetCreatureListWithEntryInGrid(elementals, NPC_FIRE_ELEMENTAL,
            20.0f);
        for (Creature* elemental : elementals)
        {
            if (!elemental->IsAlive()
                || elemental->GetDisplayId() == RAGNAROS_DISPLAY_ID)
                continue;

            ObjectGuid const owner = elemental->GetCharmerOrOwnerGUID();
            ObjectGuid const creator = elemental->GetCreatorGUID();
            if (owner != player->GetGUID() && owner != totem->GetGUID()
                && creator != player->GetGUID()
                && creator != totem->GetGUID())
                continue;

            elemental->SetDisplayId(RAGNAROS_DISPLAY_ID);
            elemental->SetNativeDisplayId(RAGNAROS_DISPLAY_ID);
            elemental->SetObjectScale(RAGNAROS_SCALE);
            elemental->SetMaxHealth(elemental->GetMaxHealth() * 2);
            elemental->SetHealth(elemental->GetMaxHealth());
            break;
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_ele_ragnaros::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900403: Lightning Overload now also affects Lava Burst
//  Hooked on Lava Burst (all ranks via -51505). After hit,
//  if player has LO talent + this passive, trigger an overload.
// ============================================================
class spell_custom_ele_overload_lvb : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_overload_lvb);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !target->IsAlive())
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_OVERLOAD_LVB_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Skip the overload copy itself: it travels, so its hit lands after
        // the re-entry guard below has long been reset (copies overloaded
        // again: 13-23 hits from 10 casts, bot run 435). A copy carries the
        // passive as its triggering aura.
        if (s_lvbReentry || (GetSpell()->GetTriggeredByAuraSpellInfo()
            && GetSpell()->GetTriggeredByAuraSpellInfo()->Id
                == SPELL_ELE_OVERLOAD_LVB_PASSIVE))
            return;

        // Check if player has Lightning Overload talent (icon 2018)
        AuraEffect const* loTalent = player->GetDummyAuraEffect(
            SPELLFAMILY_SHAMAN, 2018, EFFECT_0);
        if (!loTalent)
            return;

        // Proc chance = talent amount (11/22/33%)
        // With this passive, doubled chance
        int32 chance = loTalent->GetAmount() * 2;
        if (!roll_chance_i(chance))
            return;

        // Fire Lava Burst overload at half damage
        int32 damage = GetHitDamage() / 2;
        if (damage > 0)
        {
            s_lvbReentry = true;
            caster->CastCustomSpell(target, GetSpellInfo()->Id, &damage,
                nullptr, nullptr, true, nullptr,
                player->GetAuraEffect(SPELL_ELE_OVERLOAD_LVB_PASSIVE, EFFECT_0));
            s_lvbReentry = false;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ele_overload_lvb::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900404: Lava Burst spreads Flame Shock
//  Hooked on Lava Burst (all ranks via -51505). After hit,
//  if target has Flame Shock, spread it to nearby enemies.
// ============================================================
class spell_custom_ele_lvb_spread_fs : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_lvb_spread_fs);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_LVB_SPREAD_FS_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Skip the overload copy's triggered Lava Burst (avoid cascade)
        if (s_lvbReentry)
            return;

        // Check if target has Flame Shock from this caster
        AuraEffect* fs = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE,
            SPELLFAMILY_SHAMAN, 0x10000000, 0, 0, caster->GetGUID());
        if (!fs)
            return;

        uint32 fsSpellId = fs->GetBase()->GetId();

        // Spread to up to 5 nearby enemies
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, caster, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        uint32 count = 0;
        for (Unit* t : targets)
        {
            if (count >= 5)
                break;
            if (!t->IsAlive() || !caster->IsValidAttackTarget(t))
                continue;
            // Don't spread if already has FS from this caster
            if (t->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE,
                SPELLFAMILY_SHAMAN, 0x10000000, 0, 0, caster->GetGUID()))
                continue;

            caster->CastSpell(t, fsSpellId, true);
            ++count;
        }

        if (count > 0)
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> LvB spread FS to {} targets",
                player->GetName(), count);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ele_lvb_spread_fs::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900405: Flame Shock ticks -> reset Lava Burst CD
//  Proc aura: on periodic damage, chance to reset LvB cooldown.
// ============================================================
class spell_custom_ele_fs_reset_lvb : public AuraScript
{
    PrepareAuraScript(spell_custom_ele_fs_reset_lvb);

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

        // Only proc on Flame Shock ticks (SpellFamilyFlags[0] = 0x10000000)
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell || procSpell->SpellFamilyName != SPELLFAMILY_SHAMAN)
            return;
        if (!(procSpell->SpellFamilyFlags[0] & 0x10000000))
            return;

        // Reset the Lava Burst cooldown of every rank the shaman knows (a
        // level-80 shaman casts rank 2, 60043; resetting rank 1 alone - the
        // old code - left the real cooldown running)
        for (uint32 rankId = sSpellMgr->GetFirstSpellInChain(SPELL_LAVA_BURST_R1);
            rankId; rankId = sSpellMgr->GetNextSpellInChain(rankId))
            if (player->HasSpell(rankId))
                player->RemoveSpellCooldown(rankId, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> FS tick reset LvB CD",
            player->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_ele_fs_reset_lvb::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900406: Lava Burst two charges
//  Hooked on Lava Burst (all ranks via -51505). After cast,
//  if this was the first charge, immediately reset CD.
//  Uses a hidden aura stack to track charges.
// ============================================================
class spell_custom_ele_lvb_charges : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_lvb_charges);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_LVB_TWO_CHARGES_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Skip the overload copy's triggered Lava Burst (don't touch charges)
        if (s_lvbReentry)
            return;

        // Check if we have a "charge available" marker
        // Use the passive aura's stack count as charge tracker
        Aura* chargeAura = player->GetAura(SPELL_ELE_LVB_TWO_CHARGES_PASSIVE);
        if (!chargeAura)
            return;

        uint8 stacks = chargeAura->GetStackAmount();
        if (stacks >= 2)
        {
            // Second charge used, set stacks to 1
            chargeAura->SetStackAmount(1);
            // Don't reset CD - it's now on real cooldown
        }
        else
        {
            // First charge used, reset CD immediately and mark
            chargeAura->SetStackAmount(2);
            player->RemoveSpellCooldown(GetSpellInfo()->Id, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_ele_lvb_charges::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900407: Clearcasting -> Lava Burst instant
//  Concept: "while clearcasting your lava burst is instant". The
//  shaman's Clearcasting (16246, from Elemental Focus) carries a
//  hidden helper aura (900409: Lava Burst cast time -100 %) while
//  the passive is learned; both leave together. The old row made
//  every Lava Burst instant, Clearcasting or not.
// ============================================================
class spell_custom_ele_cc_instant_lvb : public AuraScript
{
    PrepareAuraScript(spell_custom_ele_cc_instant_lvb);

    void HandleApply(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (target && g_CustomSpellsEnabled
            && target->HasAura(SPELL_ELE_CC_INSTANT_LVB_PASSIVE))
            target->CastSpell(target, SPELL_ELE_CC_INSTANT_LVB_AURA, true);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
            target->RemoveAurasDueToSpell(SPELL_ELE_CC_INSTANT_LVB_AURA);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_custom_ele_cc_instant_lvb::HandleApply, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_ele_cc_instant_lvb::HandleRemove, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }
};

// ============================================================
//  SPELL 900434: 5 Maelstrom stacks -> empowered summons, 5 sec
//  Hooked on Maelstrom Weapon (53817). When the stacks reach 5,
//  the shaman gets Maelstrom Fury (900439, 5 sec); while it lasts,
//  every melee hit of one of the shaman's summons sets off Spirit
//  Howl (900440) around that summon - see
//  custom_enh_summon_melee_unitscript below. (The old code fired a
//  single burst per summon at the moment of the 5th stack.)
// ============================================================
class spell_custom_enh_maelstrom_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_enh_maelstrom_aoe);

    void HandleStackChange(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        if (GetStackAmount() < 5)
            return;

        Player* player = GetUnitOwner() ? GetUnitOwner()->ToPlayer()
            : nullptr;
        if (!player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_ENH_MAELSTROM_AOE_PASSIVE))
            return;

        player->CastSpell(player, SPELL_ENH_MAELSTROM_AOE_BUFF, true);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(
            spell_custom_enh_maelstrom_aoe::HandleStackChange, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_CHANGE_AMOUNT);
    }
};

// ============================================================
//  SPELL 900436: Auto attacks -> chance to summon wolf
//  Proc aura: on melee auto attack, chance to summon a
//  temporary wolf that fights for 15 seconds.
// ============================================================
class spell_custom_enh_wolf_summon : public AuraScript
{
    PrepareAuraScript(spell_custom_enh_wolf_summon);

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

        // Summon a temporary wolf (15s), as the shaman's guardian
        if (Creature* wolf = player->SummonCreature(NPC_CUSTOM_WOLF,
            target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
            player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000,
            sSummonPropertiesStore.LookupEntry(SUMMON_PROPERTIES_PROC_GUARDIAN)))
        {
            wolf->SetOwnerGUID(player->GetGUID());
            wolf->SetCreatorGUID(player->GetGUID());
            wolf->SetFaction(player->GetFaction());
            wolf->Attack(target, true);
            wolf->GetMotionMaster()->MoveChase(target);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_enh_wolf_summon::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900437: Spirit Wolves inherit haste
//  Hooked on Feral Spirit (51533). After cast, apply owner's
//  haste rating to the wolves as attack speed reduction.
// ============================================================
class spell_custom_enh_wolf_haste : public SpellScript
{
    PrepareSpellScript(spell_custom_enh_wolf_haste);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ENH_WOLF_HASTE_PASSIVE))
            return;

        if (!g_CustomSpellsEnabled)
            return;

        // Get owner's haste percentage
        float haste = player->GetFloatValue(UNIT_MOD_CAST_SPEED);
        // haste is a multiplier <1 = faster. E.g. 0.8 = 20% haste

        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            if ((*itr)->GetEntry() == NPC_SPIRIT_WOLF && (*itr)->ToCreature())
            {
                Creature* wolf = (*itr)->ToCreature();
                // Apply haste: reduce base attack time
                uint32 baseAttack = wolf->GetAttackTime(BASE_ATTACK);
                uint32 newAttack = static_cast<uint32>(baseAttack * haste);
                wolf->SetAttackTime(BASE_ATTACK, newAttack);

                LOG_INFO("module",
                    "mod-custom-spells: Player {} -> Spirit Wolf haste applied ({}ms -> {}ms)",
                    player->GetName(), baseAttack, newAttack);
            }
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_enh_wolf_haste::HandleAfterCast);
    }
};

// ============================================================
//  SPELLS 900438 / 900434: melee hits of the shaman's summons
//  - 900438: a Spirit Wolf's melee hit has a 5 % chance to cast
//    Chain Lightning at its target.
//  - 900434: while the shaman has Maelstrom Fury (900439), each
//    melee hit of one of its summons sets off Spirit Howl (900440)
//    around that summon, at most once a second per summon.
//  Only white melee hits count (DealDamage with DIRECT_DAMAGE): the
//  old OnDamage hook also fired on the summons' own spell damage,
//  so a Chain Lightning could set off the next one.
// ============================================================
class SummonHowlCd : public DataMap::Base
{
public:
    uint32 NextAllowedMs = 0;
};

class custom_enh_summon_melee_unitscript : public UnitScript
{
public:
    custom_enh_summon_melee_unitscript()
        : UnitScript("custom_enh_summon_melee_unitscript") { }

    uint32 DealDamage(Unit* attacker, Unit* victim, uint32 damage,
        DamageEffectType damagetype) override
    {
        if (damagetype != DIRECT_DAMAGE || !attacker || !victim
            || !victim->IsAlive() || !g_CustomSpellsEnabled)
            return damage;

        Creature* summon = attacker->ToCreature();
        if (!summon)
            return damage;

        Player* owner = summon->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner || owner->getClass() != CLASS_SHAMAN)
            return damage;

        if (summon->GetEntry() == NPC_SPIRIT_WOLF
            && owner->HasAura(SPELL_ENH_WOLF_CL_PASSIVE) && roll_chance_i(5))
            summon->CastSpell(victim, SPELL_CL_R6, true);

        if (owner->HasAura(SPELL_ENH_MAELSTROM_AOE_BUFF))
        {
            uint32 const now = GameTime::GetGameTimeMS().count();
            SummonHowlCd* cd = summon->CustomData.GetDefault<SummonHowlCd>(
                "mod_custom_spells_howl_cd");
            if (now >= cd->NextAllowedMs)
            {
                cd->NextAllowedMs = now + 1000;
                summon->CastSpell(summon, SPELL_ENH_WOLF_AOE_HELPER, true);
            }
        }
        return damage;
    }
};

// ============================================================
//  SHAMAN ELE: 900415 Elemental Resonance
//  Concept: dealing Lightning damage increases your Fire damage,
//  dealing Fire damage increases your Lightning damage, 2 % per
//  stack, 5 stacks each. Proc (spell_proc: harmful spells and
//  periodic damage done): Nature damage -> 900416 (+2 % Fire, 10 s),
//  Fire damage -> 900417 (+2 % Nature, 10 s).
// ============================================================
class spell_custom_ele_resonance : public AuraScript
{
    PrepareAuraScript(spell_custom_ele_resonance);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        return damageInfo && damageInfo->GetDamage() && eventInfo.GetSpellInfo()
            && g_CustomSpellsEnabled;
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        uint32 const school = eventInfo.GetSpellInfo()->GetSchoolMask();
        Unit* shaman = GetTarget();
        if (school & SPELL_SCHOOL_MASK_NATURE)
            shaman->CastSpell(shaman, SPELL_ELE_RESONANCE_FIRE_BUFF, true,
                nullptr, aurEff);
        if (school & SPELL_SCHOOL_MASK_FIRE)
            shaman->CastSpell(shaman, SPELL_ELE_RESONANCE_NATURE_BUFF, true,
                nullptr, aurEff);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_ele_resonance::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_ele_resonance::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SHAMAN ELE: 900418 Lightning Shield casts Chain Lightning
//  Hooked on the Lightning Shield damage spells (all ranks via
//  -26364): each discharge has a 20 % chance to send the highest
//  Chain Lightning rank the shaman knows at the attacker (free,
//  triggered).
// ============================================================
class spell_custom_ele_ls_chain_lightning : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_ls_chain_lightning);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !target->IsAlive() || !g_CustomSpellsEnabled)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->HasAura(SPELL_ELE_LS_CHAIN_LIGHTNING_PASSIVE))
            return;

        if (!roll_chance_i(20))
            return;

        uint32 chainLightning = 0;
        for (uint32 rank = SPELL_CHAIN_LIGHTNING_R1; rank;
            rank = sSpellMgr->GetNextSpellInChain(rank))
            if (player->HasSpell(rank))
                chainLightning = rank;
        if (chainLightning)
            player->CastSpell(target, chainLightning, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ele_ls_chain_lightning::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900445: Ghost Wolf in combat (AuraScript on Ghost Wolf 2645)
//  List line (third concept list, 2026-10-31): "your movementspeed is
//  increased by additional 100% for 3 seconds when entering ghost wolf
//  in combat. also your next hit that makes you exit ghost form, erupts
//  in lightning dealing aoe damage dependent on your paragon level".
//  Mechanism:
//   - When the shaman (passive 900445) turns into Ghost Wolf while in
//     combat, it gets Storm Rush (900446: +100 % movement speed on top of
//     the form's +40 %, 3 s - the run speed takes the highest speed aura
//     (SPELL_AURA_MOD_INCREASE_SPEED), so the aura carries the 140 that
//     Ghost Wolf and the bonus make together) and the hidden Storm Charge
//     (900447, no end, one charge). Entering the form out of combat arms
//     nothing.
//   - Ghost Wolf has no attack of its own: it ends when the client
//     cancels the form to cast, or the shaman cancels it. When the form
//     ends with the charge up, the charge gets a 4-s window instead (a
//     cancelled form is followed by the cast or swing that caused it; the
//     window covers a 2.5-s Lightning Bolt and its missile). A death
//     takes the charge away.
//   - The window's first hit that deals damage - melee, ability or
//     spell, triggered strikes included - erupts: see
//     spell_custom_enh_ghost_charge below. The charge is a spell_proc
//     aura with one charge, so the core takes it away with that proc.
// ============================================================
class spell_custom_enh_ghost_wolf : public AuraScript
{
    PrepareAuraScript(spell_custom_enh_ghost_wolf);

    void HandleApply(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !g_CustomSpellsEnabled || !player->IsInCombat()
            || !player->HasAura(SPELL_ENH_GHOST_RUSH_PASSIVE))
            return;

        player->CastSpell(player, SPELL_ENH_GHOST_RUSH_BUFF, true);
        player->CastSpell(player, SPELL_ENH_GHOST_CHARGE_AURA, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Ghost Wolf in combat: Storm Rush, charge up",
            player->GetName());
    }

    void HandleRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        Aura* charge = player->GetAura(SPELL_ENH_GHOST_CHARGE_AURA);
        if (!charge)
            return;

        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
        {
            player->RemoveAurasDueToSpell(SPELL_ENH_GHOST_CHARGE_AURA);
            return;
        }

        charge->SetMaxDuration(GHOST_ERUPTION_WINDOW_MS);
        charge->SetDuration(GHOST_ERUPTION_WINDOW_MS);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_custom_enh_ghost_wolf::HandleApply, EFFECT_0,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_enh_ghost_wolf::HandleRemove, EFFECT_0,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
    }
};

// ============================================================
//  SPELL 900447: Storm Charge (AuraScript, the proc side of 900445)
//  Hidden aura (spell_proc: damage done by melee, melee abilities and
//  spells, triggered hits included; one charge). Only a hit while the
//  shaman is NOT in Ghost Wolf form counts - that is the "next hit that
//  makes you exit ghost form": the charge is consumed by the core when
//  the proc is taken, and the eruption (900448: Nature, 10 yd, 666 + 5 x
//  Paragon level) strikes round the hit unit, which is left out (it took
//  the hit).
// ============================================================
class spell_custom_enh_ghost_charge : public AuraScript
{
    PrepareAuraScript(spell_custom_enh_ghost_charge);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        if (!g_CustomSpellsEnabled)
            return false;

        Player* player = GetTarget()->ToPlayer();
        Unit* victim = eventInfo.GetActionTarget();
        if (!player || !victim || victim == player || !victim->IsAlive()
            || player->GetShapeshiftForm() == FORM_GHOSTWOLF)
            return false;

        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (spellInfo && spellInfo->Id == SPELL_ENH_GHOST_ERUPTION)
            return false;

        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        return damageInfo && (damageInfo->GetDamage() + damageInfo->GetAbsorb())
            && player->IsValidAttackTarget(victim);
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        Unit* victim = eventInfo.GetActionTarget();
        if (!player || !victim)
            return;

        CastAnchoredBurst(player, victim, SPELL_ENH_GHOST_ERUPTION,
            ParagonDamage(player));

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Ghost Wolf eruption round {}",
            player->GetName(), victim->GetName());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_enh_ghost_charge::CheckProc);
        OnEffectProc += AuraEffectProcFn(
            spell_custom_enh_ghost_charge::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900468: Heals launch Lightning Bolt / Chain Lightning / Flame Shock
//  List line: "Casting Lesser Healing Wave/Healing waves launches a free
//  Lightning Bolt at a random target in range. Casting Chain Heal
//  launches a free Chain Lightning at a random target in range. Riptide
//  applies Flame Shock at a random target in range."
//  Hooked on Healing Wave (-331), Lesser Healing Wave (-8004), Chain
//  Heal (-1064) and Riptide (-61295). After the cast the shaman casts
//  the highest rank it knows of Lightning Bolt (Healing Wave, Lesser
//  Healing Wave), Chain Lightning (Chain Heal) or Flame Shock (Riptide)
//  at ONE random enemy that is already in combat, within 30 yd of the
//  shaman and in its line of sight (a heal never pulls an idle mob),
//  triggered: free, instant. The strike is not one of the hooked
//  spells, so it cannot set the next one off.
//  Only active when the shaman has passive 900468.
// ============================================================
class spell_custom_rst_heal_strikes : public SpellScript
{
    PrepareSpellScript(spell_custom_rst_heal_strikes);

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_RST_HEAL_STRIKES_PASSIVE))
            return;

        SpellInfo const* heal = GetSpellInfo();
        uint32 firstRank;
        if (heal->SpellFamilyFlags[2] & FAMILYFLAG2_RIPTIDE)
            firstRank = SPELL_FLAME_SHOCK_R1;
        else if (heal->SpellFamilyFlags[0] & FAMILYFLAG_CHAIN_HEAL)
            firstRank = SPELL_CHAIN_LIGHTNING_R1;
        else
            firstRank = SPELL_LIGHTNING_BOLT_R1;

        uint32 const strike = HighestKnownRank(player, firstRank);
        if (!strike)
            return;

        if (Unit* enemy = SelectRandomEngagedEnemy(player, player,
            HEAL_STRIKE_RANGE))
        {
            player->CastSpell(enemy, strike, true);

            LOG_INFO("module",
                "mod-custom-spells: Player {} -> heal {} launches {} at {}",
                player->GetName(), heal->Id, strike, enemy->GetName());
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_rst_heal_strikes::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900469: Earth Shield Spikes (AuraScript on Earth Shield, -974)
//  List line: "earth shield get spikes, damaging the attacker (nature
//  dmg, scaling with paragon lvl)".
//  A second script on the Earth Shield aura, beside the core's
//  spell_sha_earth_shield. It hooks the same proc - the hit on the
//  shield's holder that makes the shield heal (the core script lets it
//  through once per 3.5 s) - and adds a spike: the shaman who cast the
//  shield (passive 900469) strikes the attacker with 900471 (Nature,
//  666 + 5 x the shaman's Paragon level), triggered. It has NO CheckProc
//  on purpose: a false from a second script would also cancel the
//  shield's heal. An attacker that is not the shaman's enemy is spared.
// ============================================================
class spell_custom_rst_earth_spikes : public AuraScript
{
    PrepareAuraScript(spell_custom_rst_earth_spikes);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Player* shaman = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* attacker = eventInfo.GetActor();
        if (!shaman || !attacker || !g_CustomSpellsEnabled
            || !shaman->IsAlive() || attacker == GetTarget()
            || !shaman->HasAura(SPELL_RST_EARTH_SPIKES_PASSIVE)
            || !attacker->IsAlive() || !shaman->IsValidAttackTarget(attacker))
            return;

        int32 damage = ParagonDamage(shaman);
        shaman->CastCustomSpell(attacker, SPELL_RST_EARTH_SPIKES_DAMAGE, &damage,
            nullptr, nullptr, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Earth Shield spikes {} dmg on {}",
            shaman->GetName(), damage, attacker->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_rst_earth_spikes::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900470: Direct heals add an Earth Shield charge (SpellScript)
//  List line: "Your direct heal spells increase the stacks of your
//  shields on the target by 1".
//  Hooked on the same four direct heals as 900468 (Healing Wave, Lesser
//  Healing Wave, Chain Heal, Riptide's direct heal). Each unit the heal
//  lands on that carries THIS shaman's Earth Shield gets one more charge
//  on it, capped at the shield's own maximum (Aura::ModCharges - the
//  operator's default of 2026-10-31; the maximum includes Improved Earth
//  Shield). Healing Stream and the shield's own heal are not direct
//  heals and add nothing.
//  Only active when the shaman has passive 900470.
// ============================================================
class spell_custom_rst_shield_stacks : public SpellScript
{
    PrepareSpellScript(spell_custom_rst_shield_stacks);

    void HandleAfterHit()
    {
        Player* shaman = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!shaman || !target || !g_CustomSpellsEnabled
            || !shaman->HasAura(SPELL_RST_SHIELD_STACKS_PASSIVE))
            return;

        Aura* shield = target->GetAuraOfRankedSpell(SPELL_EARTH_SHIELD_R1,
            shaman->GetGUID());
        if (!shield || !shield->IsUsingCharges())
            return;

        shield->ModCharges(1);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Earth Shield on {}: {} charges",
            shaman->GetName(), target->GetName(), shield->GetCharges());
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_rst_shield_stacks::HandleAfterHit);
    }
};

void AddShamanSpellsScripts()
{
    // Shaman Elemental
    new custom_totem_follow_playerscript();
    RegisterSpellScript(spell_custom_ele_ragnaros);
    RegisterSpellScript(spell_custom_ele_overload_lvb);
    RegisterSpellScript(spell_custom_ele_lvb_spread_fs);
    RegisterSpellScript(spell_custom_ele_fs_reset_lvb);
    RegisterSpellScript(spell_custom_ele_lvb_charges);
    RegisterSpellScript(spell_custom_ele_cc_instant_lvb);
    RegisterSpellScript(spell_custom_ele_resonance);
    RegisterSpellScript(spell_custom_ele_ls_chain_lightning);

    // Shaman Enhancement (900435 Summons +50 % lives in the shared minion
    // auras, custom_spells_global.cpp)
    RegisterSpellScript(spell_custom_enh_maelstrom_aoe);
    RegisterSpellScript(spell_custom_enh_wolf_summon);
    RegisterSpellScript(spell_custom_enh_wolf_haste);
    new custom_enh_summon_melee_unitscript();
    RegisterSpellScript(spell_custom_enh_ghost_wolf);
    RegisterSpellScript(spell_custom_enh_ghost_charge);

    // Shaman Resto: 900467 Mana Regen is shared, custom_spells_global.cpp
    RegisterSpellScript(spell_custom_rst_heal_strikes);
    RegisterSpellScript(spell_custom_rst_earth_spikes);
    RegisterSpellScript(spell_custom_rst_shield_stacks);
}
