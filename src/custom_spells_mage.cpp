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
//  MAGE ARCANE: Arcane Charges stack to 8 (900705)
//  Hooked on Arcane Blast (all ranks via -42897).
//  Overrides the DBC stack cap of 4 on the Arcane Blast
//  debuff (36032), allowing up to 8 stacks.
// ============================================================
class spell_custom_mage_ab_charges : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_ab_charges);

    uint8 _precastStacks = 0;

    void HandleBeforeCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        Aura* debuff = player->GetAura(SPELL_ARCANE_BLAST_DEBUFF);
        _precastStacks = debuff ? debuff->GetStackAmount() : 0;
    }

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_MAGE_ARC_CHARGES_8_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Aura* debuff = player->GetAura(SPELL_ARCANE_BLAST_DEBUFF);
        if (!debuff)
            return;

        // The game already applied debuff and capped at 4.
        // Calculate what the real stack count should be (up to 8).
        uint8 realStacks = std::min<uint8>(_precastStacks + 1, 8);

        if (realStacks > debuff->GetStackAmount())
            debuff->SetStackAmount(realStacks);
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_custom_mage_ab_charges::HandleBeforeCast);
        AfterCast += SpellCastFn(spell_custom_mage_ab_charges::HandleAfterCast);
    }
};

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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Record current stacks before applying
        Aura* debuff = player->GetAura(SPELL_ARCANE_BLAST_DEBUFF);
        uint8 currentStacks = debuff ? debuff->GetStackAmount() : 0;

        // Apply Arcane Blast debuff (adds 1 stack, capped at 4 by DBC)
        player->CastSpell(player, SPELL_ARCANE_BLAST_DEBUFF, true);

        // If player has charges-to-8 passive and we need stacks > 4
        if (player->HasAura(SPELL_MAGE_ARC_CHARGES_8_PASSIVE) && currentStacks >= 4)
        {
            debuff = player->GetAura(SPELL_ARCANE_BLAST_DEBUFF);
            if (debuff)
            {
                uint8 newStacks = std::min<uint8>(currentStacks + 1, 8);
                if (newStacks > debuff->GetStackAmount())
                    debuff->SetStackAmount(newStacks);
            }
        }
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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
//  Proc aura: when taking damage and health drops below 30%,
//  auto-cast Mana Shield (highest rank) and restore all mana.
//  60s internal cooldown.
// ============================================================
class spell_custom_mage_emergency_shield : public AuraScript
{
    PrepareAuraScript(spell_custom_mage_emergency_shield);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Only trigger when below 30% health
        if (player->GetHealthPct() > 30.0f)
            return;

        // Activate Mana Shield (highest rank)
        player->CastSpell(player, SPELL_MANA_SHIELD_R9, true);

        // Restore all mana
        player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
    }

    void Register() override
    {
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
class spell_custom_mage_targeted_blink : public SpellScript
{
    PrepareSpellScript(spell_custom_mage_targeted_blink);

    void HandleAfterCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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
//  MAGE ARCANE: Mana regen +2% per missing mana % (900700)
//  PlayerScript: every 5s, regenerate mana based on how much
//  mana is missing. At 50% missing -> +1% max mana/5s.
//  At 90% missing -> +1.8% max mana/5s.
// ============================================================
class custom_mage_mana_regen_playerscript : public PlayerScript
{
public:
    custom_mage_mana_regen_playerscript() : PlayerScript("custom_mage_mana_regen_playerscript") {}

    void OnPlayerLogout(Player* player) override
    {
        if (player)
            _lastRegen.erase(player->GetGUID());
    }

    void OnPlayerUpdate(Player* player, uint32 /*p_time*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_MAGE_ARC_MANA_REGEN))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Throttle: only every 5 seconds
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        ObjectGuid guid = player->GetGUID();
        if (_lastRegen.count(guid) && (now - _lastRegen[guid]) < 5)
            return;
        _lastRegen[guid] = now;

        uint32 maxMana = player->GetMaxPower(POWER_MANA);
        uint32 curMana = player->GetPower(POWER_MANA);
        if (maxMana == 0)
            return;

        // Calculate missing mana percentage (0-100)
        float missingPct = 100.0f * (1.0f - static_cast<float>(curMana) / maxMana);
        // Regen bonus = missingPct * 2% of max mana, applied per 5s tick
        int32 bonus = static_cast<int32>(maxMana * (missingPct * 0.02f) / 100.0f);
        if (bonus > 0)
            player->EnergizeBySpell(player, SPELL_MAGE_ARC_MANA_REGEN,
                bonus, POWER_MANA);
    }

private:
    std::unordered_map<ObjectGuid, uint32> _lastRegen;
};

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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
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

void AddMageSpellsScripts()
{
    // Mage Arcane
    RegisterSpellScript(spell_custom_mage_ab_charges);
    RegisterSpellScript(spell_custom_mage_ae_charges);
    RegisterSpellScript(spell_custom_mage_evocation_power);
    RegisterSpellScript(spell_custom_mage_emergency_shield);
    RegisterSpellScript(spell_custom_mage_targeted_blink);
    new custom_mage_mana_regen_playerscript();

    // Mage Fire
    RegisterSpellScript(spell_custom_mage_pyro_hotstreak);

    // Mage Frost
    RegisterSpellScript(spell_custom_mage_permanent_water_ele);
    RegisterSpellScript(spell_custom_mage_comet_shower);
}
