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
//  SPELL 900400: Chain Lightning +6 targets, no damage reduction
//  Hooked on Chain Lightning (all ranks via -49271).
//  After hitting primary target, casts AoE helper (900408) on
//  up to 6 additional enemies at full damage.
// ============================================================
class spell_custom_ele_cl_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_cl_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_CL_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        // Find 6 additional enemies and deal full CL damage
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 12.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(mainTarget, searcher, 12.0f);
        targets.remove(mainTarget);

        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= 6)
                break;
            if (!target->IsAlive() || !caster->IsValidAttackTarget(target))
                continue;

            caster->CastCustomSpell(target, SPELL_ELE_CL_AOE_HELPER,
                &damage, nullptr, nullptr, true);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_ele_cl_aoe::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900401: Totems → Following Creatures (PlayerScript)
//  When player has 900401, totems follow the player instead
//  of being static. Checked every 2 seconds via OnUpdate.
// ============================================================
class custom_totem_follow_playerscript : public PlayerScript
{
public:
    custom_totem_follow_playerscript() : PlayerScript("custom_totem_follow_playerscript") {}

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        // Check for any of the three totem follow passives (Ele/Enh/Resto)
        if (!player->HasAura(SPELL_ELE_TOTEM_FOLLOW_PASSIVE) &&
            !player->HasAura(SPELL_ENH_TOTEM_FOLLOW_PASSIVE) &&
            !player->HasAura(SPELL_RST_TOTEM_FOLLOW_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Throttle: only check every ~2 seconds using game time
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        static std::unordered_map<ObjectGuid, uint32> s_lastCheck;
        ObjectGuid guid = player->GetGUID();
        if (s_lastCheck.count(guid) && (now - s_lastCheck[guid]) < 2)
            return;
        s_lastCheck[guid] = now;

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
};

// ============================================================
//  SPELL 900402: Fire Elemental → Ragnaros
//  Hooked on Fire Elemental Totem (2894). After cast,
//  replaces the Fire Elemental with a Ragnaros model.
// ============================================================
class spell_custom_ele_ragnaros : public SpellScript
{
    PrepareSpellScript(spell_custom_ele_ragnaros);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ELE_RAGNAROS_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Find the fire elemental and change its display to Ragnaros
        constexpr uint32 RAGNAROS_DISPLAY_ID = 11121;
        constexpr float  RAGNAROS_SCALE      = 0.35f;

        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            if ((*itr)->GetEntry() == NPC_FIRE_ELEMENTAL && (*itr)->ToCreature())
            {
                (*itr)->SetDisplayId(RAGNAROS_DISPLAY_ID);
                (*itr)->SetObjectScale(RAGNAROS_SCALE);
                // Double the health
                (*itr)->SetMaxHealth((*itr)->GetMaxHealth() * 2);
                (*itr)->SetHealth((*itr)->GetMaxHealth());
                LOG_INFO("module",
                    "mod-custom-spells: Player {} -> Fire Elemental → Ragnaros",
                    player->GetName());
                break;
            }
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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
            caster->CastCustomSpell(target, GetSpellInfo()->Id, &damage,
                nullptr, nullptr, true);
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Check if target has Flame Shock from this caster
        AuraEffect* fs = target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE,
            SPELLFAMILY_SHAMAN, 0x10000000, 0, 0, caster->GetGUID());
        if (!fs)
            return;

        uint32 fsSpellId = fs->GetBase()->GetId();

        // Spread to up to 5 nearby enemies
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 10.0f);
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
//  SPELL 900405: Flame Shock ticks → reset Lava Burst CD
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Only proc on Flame Shock ticks (SpellFamilyFlags[0] = 0x10000000)
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell || procSpell->SpellFamilyName != SPELLFAMILY_SHAMAN)
            return;
        if (!(procSpell->SpellFamilyFlags[0] & 0x10000000))
            return;

        // Reset Lava Burst cooldown (all ranks)
        player->RemoveSpellCooldown(SPELL_LAVA_BURST_R2, true);

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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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
            // Don't reset CD — it's now on real cooldown
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
//  SPELL 900407: Clearcasting → Lava Burst instant
//  Implemented purely via DBC: ADD_PCT_MODIFIER with
//  SPELLMOD_CASTING_TIME = -100% on Lava Burst.
//  This makes LvB always instant when the passive is active.
//  (No separate C++ class needed — DBC handles it.)
// ============================================================

// ============================================================
//  SPELL 900434: 5 Maelstrom stacks → summons empowered AoE 5s
//  Hooked on Maelstrom Weapon (53817). When stacks reach 5,
//  apply a 5s buff (900439) that makes summons deal AoE.
// ============================================================
class spell_custom_enh_maelstrom_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_enh_maelstrom_aoe);

    void HandleStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetStackAmount() < 5)
            return;

        Unit* owner = GetUnitOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ENH_MAELSTROM_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Apply empowerment buff on player (5s duration)
        player->CastSpell(player, SPELL_ENH_MAELSTROM_AOE_BUFF, true);

        // Make all controlled units deal AoE around their targets
        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            Unit* pet = *itr;
            if (pet && pet->IsAlive() && pet->ToCreature())
                pet->CastSpell(pet, SPELL_ENH_WOLF_AOE_HELPER, true);
        }

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> Maelstrom 5 stacks → summons empowered",
            player->GetName());
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_custom_enh_maelstrom_aoe::HandleStackChange,
            EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_CHANGE_AMOUNT);
    }
};

// ============================================================
//  SPELL 900436: Auto attacks → chance to summon wolf
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Summon a temporary wolf (15s)
        if (Creature* wolf = player->SummonCreature(NPC_CUSTOM_WOLF,
            target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
            player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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
                    "mod-custom-spells: Player {} -> Spirit Wolf haste applied ({}ms → {}ms)",
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
//  SPELL 900438: Spirit Wolves 5% Chain Lightning on melee hit
//  UnitScript: On any damage dealt, if attacker is a Spirit
//  Wolf and owner has passive, 5% CL proc.
// ============================================================
class custom_wolf_cl_unitscript : public UnitScript
{
public:
    custom_wolf_cl_unitscript() : UnitScript("custom_wolf_cl_unitscript") {}

    void OnDamage(Unit* attacker, Unit* victim, uint32& /*damage*/) override
    {
        if (!attacker || !victim || !victim->IsAlive())
            return;

        Creature* wolf = attacker->ToCreature();
        if (!wolf || wolf->GetEntry() != NPC_SPIRIT_WOLF)
            return;

        Unit* owner = wolf->GetOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_ENH_WOLF_CL_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // 5% chance
        if (!roll_chance_i(5))
            return;

        // Cast Chain Lightning from the wolf
        wolf->CastSpell(victim, SPELL_CL_R6, true);
    }
};

// ============================================================
//  SPELL 900467: Mana regen +2% per missing mana %
//  PlayerScript: periodically adjust mana regen based on
//  how much mana is missing.
// ============================================================
class custom_mana_regen_playerscript : public PlayerScript
{
public:
    custom_mana_regen_playerscript() : PlayerScript("custom_mana_regen_playerscript") {}

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_RST_MANA_REGEN_PASSIVE))
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

        // Calculate missing mana percentage (0-100)
        float missingPct = 100.0f * (1.0f - static_cast<float>(curMana) / maxMana);
        // Regen bonus = missingPct * 2% of max mana, applied per 5s tick
        int32 bonus = static_cast<int32>(maxMana * (missingPct * 0.02f) / 100.0f);
        // Scale to per-5s (this runs every 5s)
        if (bonus > 0)
            player->EnergizeBySpell(player, SPELL_RST_MANA_REGEN_PASSIVE,
                bonus, POWER_MANA);
    }
};

void AddShamanSpellsScripts()
{
    // Shaman Elemental
    RegisterSpellScript(spell_custom_ele_cl_aoe);
    new custom_totem_follow_playerscript();
    RegisterSpellScript(spell_custom_ele_ragnaros);
    RegisterSpellScript(spell_custom_ele_overload_lvb);
    RegisterSpellScript(spell_custom_ele_lvb_spread_fs);
    RegisterSpellScript(spell_custom_ele_fs_reset_lvb);
    RegisterSpellScript(spell_custom_ele_lvb_charges);

    // Shaman Enhancement
    RegisterSpellScript(spell_custom_enh_maelstrom_aoe);
    RegisterSpellScript(spell_custom_enh_wolf_summon);
    RegisterSpellScript(spell_custom_enh_wolf_haste);
    new custom_wolf_cl_unitscript();

    // Shaman Resto
    new custom_mana_regen_playerscript();
}
