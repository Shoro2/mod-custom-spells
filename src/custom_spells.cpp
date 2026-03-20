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

#include "ScriptMgr.h"
#include "Player.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "Config.h"
#include "Log.h"
#include "ObjectAccessor.h"

/*
 * ==========================================================================
 *  mod-custom-spells - Custom Spell Effects Module
 * ==========================================================================
 *
 *  This module uses SpellScripts to override spell effect behavior.
 *  Each custom spell gets its own SpellScript class that hooks into
 *  the spell's DBC effects (e.g. School Damage) and replaces the
 *  damage calculation with custom logic.
 *
 *  HOW TO ADD A NEW CUSTOM SPELL:
 *  1. Define the spell ID as a constant in the enum below.
 *  2. Create a new SpellScript class (see spell_custom_paragon_strike).
 *  3. Register it in AddCustomSpellsScripts() at the bottom.
 */

enum CustomSpellIds
{
    // Custom damage spell: Base 666 + 66% AP + 1% per Paragon level
    SPELL_CUSTOM_PARAGON_STRIKE         = 900106,
    // Each cast reduces Bladestorm (46927) cooldown by 0.5s
    SPELL_CUSTOM_BLADESTORM_CD_REDUCE   = 900107,
    // Bloody Whirlwind buff: +50% Whirlwind damage per stack, consumed on WW cast
    SPELL_BLOODY_WHIRLWIND_BUFF         = 900115,
    // Bloody Whirlwind passive: procs 900115 on Bloodthirst hit
    SPELL_BLOODY_WHIRLWIND_PASSIVE      = 900116,
    // Speedy Bloodthirst: Whirlwind resets Bloodthirst cooldown
    SPELL_CUSTOM_SPEEDY_BLOODTHIRST     = 900117,
    // WW single-target: passive auras (markers checked via HasAura)
    SPELL_WW_OVERPOWER_PASSIVE          = 900118,
    SPELL_WW_SLAM_PASSIVE               = 900119,
    // WW single-target: boosted damage spells (autocast)
    SPELL_WW_OVERPOWER_BOOSTED          = 900120,
    SPELL_WW_SLAM_BOOSTED               = 900121,
};

// ---- Bloodthirst SpellFamilyFlags ----
// Bloodthirst (23881): SpellFamilyName=4, SpellFamilyFlags[1]=0x00000400 (bit 42)
constexpr uint32 SPELLFAMILY_WARRIOR_ID        = 4;
constexpr uint32 BLOODTHIRST_FAMILY_FLAG1      = 0x00000400;

// ---- Paragon Strike constants ----
constexpr uint32 AURA_PARAGON_LEVEL   = 100000;
constexpr int32  CUSTOM_BASE_DAMAGE   = 666;
constexpr float  CUSTOM_AP_COEFF      = 0.66f;   // 66% of Attack Power
constexpr float  CUSTOM_PARAGON_BONUS = 0.01f;    // +1% damage per Paragon level

// ---- Bladestorm CD Reduction constants ----
constexpr uint32 SPELL_BLADESTORM     = 46924;
constexpr int32  BLADESTORM_CD_REDUCE_MS = -500;  // -0.5 seconds (in ms)

// ---- Speedy Bloodthirst constants ----
constexpr uint32 SPELL_BLOODTHIRST    = 23881;

// ============================================================
//  SPELL 900106: Paragon Strike (SpellScript)
//  Overrides the School Damage effect with custom calculation:
//  - Base damage: 666
//  - Bonus: 66% Attack Power
//  - +1% total damage per Paragon level (aura 100000 stacks)
// ============================================================
class spell_custom_paragon_strike : public SpellScript
{
    PrepareSpellScript(spell_custom_paragon_strike);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // 66% of melee attack power
        float ap = player->GetTotalAttackPowerValue(BASE_ATTACK);
        float bonusDmg = ap * CUSTOM_AP_COEFF;

        // Paragon level from aura stacks on ID 100000
        uint32 paragonLevel = player->GetAuraCount(AURA_PARAGON_LEVEL);
        float paragonMult = 1.0f + (paragonLevel * CUSTOM_PARAGON_BONUS);

        // Final damage = (base + AP bonus) * paragon multiplier
        int32 totalDmg = static_cast<int32>(
            (CUSTOM_BASE_DAMAGE + bonusDmg) * paragonMult);

        SetHitDamage(totalDmg);

        LOG_INFO("module", "mod-custom-spells: Player {} -> "
            "Paragon Strike {} dmg on {} "
            "(AP: {:.0f}, Paragon Lv: {}, Mult: {:.2f})",
            player->GetName(), totalDmg, target->GetName(),
            ap, paragonLevel, paragonMult);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(
            spell_custom_paragon_strike::HandleDamage,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// ============================================================
//  SPELL 900107: Bladestorm CD Reduction (AuraScript)
//  Passive proc aura: on melee damage dealt, reduces the
//  cooldown of Bladestorm (46927) by 0.5s.
//  DBC: EFFECT_0 = SPELL_EFFECT_APPLY_AURA / SPELL_AURA_DUMMY
//  Proc filtering via spell_proc table (ProcFlags=0x14).
// ============================================================
class spell_custom_bladestorm_cd_reduce : public AuraScript
{
    PrepareAuraScript(spell_custom_bladestorm_cd_reduce);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
        {
            LOG_INFO("module", "mod-custom-spells: 900107 CheckProc -> "
                "NO player, returning false");
            return false;
        }

        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        LOG_INFO("module", "mod-custom-spells: 900107 CheckProc -> "
            "procSpell={}, flags=0x{:X}, hasBladestormCD={}",
            spellInfo ? spellInfo->Id : 0,
            eventInfo.GetTypeMask(),
            player->HasSpellCooldown(SPELL_BLADESTORM) ? "yes" : "no");

        // Only proc when Bladestorm is actually on cooldown
        return player->HasSpellCooldown(SPELL_BLADESTORM);
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

        LOG_INFO("module", "mod-custom-spells: 900107 HandleProc -> "
            "Player {} reducing Bladestorm CD by {}ms",
            player->GetName(), -BLADESTORM_CD_REDUCE_MS);

        player->ModifySpellCooldown(SPELL_BLADESTORM, BLADESTORM_CD_REDUCE_MS);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(
            spell_custom_bladestorm_cd_reduce::CheckProc);
        OnProc += AuraProcFn(
            spell_custom_bladestorm_cd_reduce::HandleProc);
    }
};

// ============================================================
//  SPELL 900116: Bloody Whirlwind Passive (AuraScript)
//  DBC has the proc aura on EFFECT_1 (not EFFECT_0).
//  Bloodthirst (23881) SpellFamilyFlags[1]=0x00000400 (bit 42).
//  This script explicitly filters procs to only fire on Bloodthirst.
// ============================================================
class spell_custom_bloody_whirlwind_passive : public AuraScript
{
    PrepareAuraScript(spell_custom_bloody_whirlwind_passive);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        // Only proc on Bloodthirst: Warrior family, flags[1] bit 10
        if (spellInfo->SpellFamilyName != SPELLFAMILY_WARRIOR_ID)
            return false;

        if (!(spellInfo->SpellFamilyFlags[1] & BLOODTHIRST_FAMILY_FLAG1))
            return false;

        return true;
    }

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Unit* caster = GetTarget();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_BLOODY_WHIRLWIND_BUFF, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_bloody_whirlwind_passive::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_bloody_whirlwind_passive::HandleProc,
            EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// ============================================================
//  SPELL 1680: Whirlwind - Consume Bloody Whirlwind stacks
//  + Single-target autocast (Overpower / Slam)
//
//  After Whirlwind finishes casting:
//  1. Remove all stacks of the Bloody Whirlwind buff (900115)
//  2. If WW hit exactly 1 target and player has passive 900118/900119,
//     autocast boosted Overpower (900120) / Slam (900121) on the target
// ============================================================
class spell_custom_bloody_whirlwind_consume : public SpellScript
{
    PrepareSpellScript(spell_custom_bloody_whirlwind_consume);

    uint32 _targetCount = 0;
    ObjectGuid _singleTargetGuid;

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        _targetCount = targets.size();
        if (_targetCount == 1 && !targets.empty())
            _singleTargetGuid = targets.front()->GetGUID();
    }

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // --- Bloody Whirlwind: consume stacks ---
        if (caster->HasAura(SPELL_BLOODY_WHIRLWIND_BUFF))
        {
            uint32 stacks = caster->GetAuraCount(SPELL_BLOODY_WHIRLWIND_BUFF);
            caster->RemoveAurasDueToSpell(SPELL_BLOODY_WHIRLWIND_BUFF);

            if (Player* player = caster->ToPlayer())
                LOG_INFO("module", "mod-custom-spells: Player {} -> "
                    "Bloody Whirlwind consumed {} stacks on Whirlwind cast",
                    player->GetName(), stacks);
        }

        // --- Single-target autocast ---
        if (_targetCount != 1)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Unit* target = ObjectAccessor::GetUnit(*player, _singleTargetGuid);
        if (!target || !target->IsAlive())
            return;

        if (player->HasAura(SPELL_WW_OVERPOWER_PASSIVE))
        {
            player->CastSpell(target, SPELL_WW_OVERPOWER_BOOSTED, true);
            LOG_INFO("module", "mod-custom-spells: Player {} -> "
                "WW single-target: autocast Overpower on {}",
                player->GetName(), target->GetName());
        }

        if (player->HasAura(SPELL_WW_SLAM_PASSIVE))
        {
            player->CastSpell(target, SPELL_WW_SLAM_BOOSTED, true);
            LOG_INFO("module", "mod-custom-spells: Player {} -> "
                "WW single-target: autocast Slam on {}",
                player->GetName(), target->GetName());
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(
            spell_custom_bloody_whirlwind_consume::FilterTargets,
            EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        AfterCast += SpellCastFn(spell_custom_bloody_whirlwind_consume::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900117: Speedy Bloodthirst (AuraScript)
//  Passive proc aura: when Whirlwind (1680) is cast,
//  resets the cooldown of Bloodthirst (23881).
//  DBC: EFFECT_0 = SPELL_EFFECT_APPLY_AURA / SPELL_AURA_PROC_TRIGGER_SPELL
//  Proc filtering via spell_proc (ProcFlags=0x10, SpellPhaseMask=FINISH).
// ============================================================
class spell_custom_speedy_bloodthirst : public AuraScript
{
    PrepareAuraScript(spell_custom_speedy_bloodthirst);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
        {
            LOG_INFO("module", "mod-custom-spells: 900117 CheckProc -> "
                "NO spell info, returning false");
            return false;
        }

        LOG_INFO("module", "mod-custom-spells: 900117 CheckProc -> "
            "spell={}, typeMask=0x{:X}, spellFamily={}, "
            "flags[0]=0x{:08X} flags[1]=0x{:08X}",
            spellInfo->Id, eventInfo.GetTypeMask(),
            spellInfo->SpellFamilyName,
            spellInfo->SpellFamilyFlags[0], spellInfo->SpellFamilyFlags[1]);

        // Only proc on Whirlwind (1680)
        return spellInfo->Id == 1680;
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player)
        {
            LOG_INFO("module", "mod-custom-spells: 900117 HandleProc -> "
                "NO player, returning");
            return;
        }

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        LOG_INFO("module", "mod-custom-spells: 900117 HandleProc -> "
            "Player {} resetting Bloodthirst CD, hasBTCooldown={}",
            player->GetName(),
            player->HasSpellCooldown(SPELL_BLOODTHIRST) ? "yes" : "no");

        player->RemoveSpellCooldown(SPELL_BLOODTHIRST, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(
            spell_custom_speedy_bloodthirst::CheckProc);
        OnProc += AuraProcFn(
            spell_custom_speedy_bloodthirst::HandleProc);
    }
};

// ============================================================
//  SPELL 900120: WW Boosted Overpower (SpellScript)
//  Autocast by Whirlwind when only 1 target is hit.
//  Multiplies DBC base damage by Paragon bonus: +1% per level.
//  Uses OnHit instead of OnEffectHitTarget to avoid DBC effect
//  type validation (the DBC effect may not be SCHOOL_DAMAGE).
// ============================================================
class spell_custom_ww_overpower : public SpellScript
{
    PrepareSpellScript(spell_custom_ww_overpower);

    void HandleHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        uint32 paragonLevel = player->GetAuraCount(AURA_PARAGON_LEVEL);
        float paragonMult = 1.0f + (paragonLevel * CUSTOM_PARAGON_BONUS);
        int32 damage = static_cast<int32>(GetHitDamage() * paragonMult);
        SetHitDamage(damage);

        LOG_INFO("module", "mod-custom-spells: Player {} -> "
            "WW Overpower {} dmg on {} (Paragon Lv: {}, Mult: {:.2f})",
            player->GetName(), damage, target->GetName(),
            paragonLevel, paragonMult);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_custom_ww_overpower::HandleHit);
    }
};

// ============================================================
//  SPELL 900121: WW Boosted Slam (SpellScript)
//  Autocast by Whirlwind when only 1 target is hit.
//  Multiplies DBC base damage by Paragon bonus: +1% per level.
//  Uses OnHit instead of OnEffectHitTarget to avoid DBC effect
//  type validation (the DBC effect may not be SCHOOL_DAMAGE).
// ============================================================
class spell_custom_ww_slam : public SpellScript
{
    PrepareSpellScript(spell_custom_ww_slam);

    void HandleHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        uint32 paragonLevel = player->GetAuraCount(AURA_PARAGON_LEVEL);
        float paragonMult = 1.0f + (paragonLevel * CUSTOM_PARAGON_BONUS);
        int32 damage = static_cast<int32>(GetHitDamage() * paragonMult);
        SetHitDamage(damage);

        LOG_INFO("module", "mod-custom-spells: Player {} -> "
            "WW Slam {} dmg on {} (Paragon Lv: {}, Mult: {:.2f})",
            player->GetName(), damage, target->GetName(),
            paragonLevel, paragonMult);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_custom_ww_slam::HandleHit);
    }
};

void AddCustomSpellsScripts()
{
    RegisterSpellScript(spell_custom_paragon_strike);
    RegisterSpellScript(spell_custom_bladestorm_cd_reduce);
    RegisterSpellScript(spell_custom_bloody_whirlwind_passive);
    RegisterSpellScript(spell_custom_bloody_whirlwind_consume);
    RegisterSpellScript(spell_custom_speedy_bloodthirst);
    RegisterSpellScript(spell_custom_ww_overpower);
    RegisterSpellScript(spell_custom_ww_slam);
}
