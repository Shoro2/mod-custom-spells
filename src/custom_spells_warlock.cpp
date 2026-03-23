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
//  WARLOCK AFFLICTION: DoT ticks deal Shadow AoE (900800)
//  Proc aura: when a periodic damage tick lands (any Warlock
//  DoT), X% chance to cast Shadow AoE helper (900803)
//  centered on the DoT target. 2s ICD.
// ============================================================
class spell_custom_wlk_dot_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_dot_aoe);

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

        // Cast Shadow AoE centered on the DoT target
        player->CastSpell(target, SPELL_WLK_AFFL_DOT_AOE_HELPER, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_wlk_dot_aoe::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  WARLOCK AFFLICTION: DoT ticks spread to 2 additional
//  targets (900802). Proc aura: on periodic damage tick,
//  X% chance to copy the ticking DoT to up to 2 nearby
//  enemies that don't already have it.
// ============================================================
class spell_custom_wlk_dot_spread : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_dot_spread);

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

        // Get the spell that caused the periodic damage
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return;

        uint32 dotSpellId = procSpell->Id;

        // Only spread Warlock DoTs (SpellFamilyName = 5)
        if (procSpell->SpellFamilyName != SPELLFAMILY_WARLOCK_ID)
            return;

        // Find up to 2 nearby enemies that don't have this DoT
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, player, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(target, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        uint32 count = 0;
        for (Unit* spreadTarget : targets)
        {
            if (count >= 2)
                break;
            if (!spreadTarget->IsAlive() || !player->IsValidAttackTarget(spreadTarget))
                continue;

            // Skip if target already has this DoT from us
            if (spreadTarget->HasAura(dotSpellId, player->GetGUID()))
                continue;

            // Apply the DoT to the new target
            player->CastSpell(spreadTarget, dotSpellId, true);
            ++count;
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_wlk_dot_spread::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  End Warlock Affliction section
// ============================================================

// ============================================================
//  WARLOCK DEMONOLOGY
// ============================================================

// ============================================================
//  900833: Killing an enemy extends Metamorphosis duration
//  PlayerScript: On kill, if player has Metamorphosis (47241),
//  extend its duration by +5s (capped at 120s).
// ============================================================
class custom_wlk_meta_kill_extend_playerscript : public PlayerScript
{
public:
    custom_wlk_meta_kill_extend_playerscript()
        : PlayerScript("custom_wlk_meta_kill_extend_playerscript") {}

    void OnCreatureKill(Player* player, Creature* /*victim*/) override
    {
        if (!player || !player->IsAlive())
            return;

        if (!player->HasAura(SPELL_WLK_DEMO_META_KILL_EXT))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        Aura* meta = player->GetAura(SPELL_METAMORPHOSIS);
        if (!meta)
            return;

        int32 curDuration = meta->GetDuration();
        int32 maxCap = 120000; // 120s cap
        int32 extension = 5000; // +5s per kill

        if (curDuration < maxCap)
        {
            int32 newDuration = std::min(curDuration + extension, maxCap);
            meta->SetDuration(newDuration);
            if (newDuration > meta->GetMaxDuration())
                meta->SetMaxDuration(newDuration);
        }
    }
};

// ============================================================
//  900834: Demon Form periodic Shadow AoE + self heal
//  Proc aura: while player has Metamorphosis (47241), every
//  periodic tick casts Shadow AoE (900842) around caster and
//  heals self (900843). The DBC entry has a periodic dummy
//  tick (e.g. every 3s) so this fires while Meta is active.
//  CheckProc: only procs if player currently has Meta aura.
// ============================================================
class spell_custom_wlk_meta_aoe_heal : public AuraScript
{
    PrepareAuraScript(spell_custom_wlk_meta_aoe_heal);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetTarget();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Only active while in Metamorphosis
        if (!player->HasAura(SPELL_METAMORPHOSIS))
            return;

        // Shadow AoE around caster
        player->CastSpell(player, SPELL_WLK_DEMO_META_AOE_HELPER, true);
        // Self heal
        player->CastSpell(player, SPELL_WLK_DEMO_META_HEAL_HELPER, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_wlk_meta_aoe_heal::HandlePeriodic,
            EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================
//  900835: Demons have chance to spawn lesser versions
//  UnitScript: on pet melee auto-attack, X% chance to summon
//  a lesser demon (30s duration, reduced stats). Uses NPC
//  entries based on the pet type. 30s ICD via static map.
// ============================================================
class custom_wlk_lesser_demon_unitscript : public UnitScript
{
public:
    custom_wlk_lesser_demon_unitscript()
        : UnitScript("custom_wlk_lesser_demon_unitscript") {}

    void OnDamage(Unit* attacker, Unit* victim, uint32& /*damage*/) override
    {
        if (!attacker || !victim)
            return;

        // Must be a pet/minion with an owner
        Creature* creature = attacker->ToCreature();
        if (!creature || !creature->IsPet())
            return;

        Unit* ownerUnit = creature->GetOwner();
        if (!ownerUnit)
            return;

        Player* owner = ownerUnit->ToPlayer();
        if (!owner)
            return;

        if (!owner->HasAura(SPELL_WLK_DEMO_LESSER_SPAWN))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // 30s ICD per player
        uint32 now = static_cast<uint32>(GameTime::GetGameTime().count());
        static std::unordered_map<ObjectGuid, uint32> s_lastSpawn;
        ObjectGuid guid = owner->GetGUID();
        if (s_lastSpawn.count(guid) && (now - s_lastSpawn[guid]) < 30)
            return;

        // 10% chance
        if (urand(1, 100) > 10)
            return;

        s_lastSpawn[guid] = now;

        // Determine lesser demon NPC based on pet entry
        uint32 petEntry = creature->GetEntry();
        uint32 lesserNpc = 0;

        // Imp family
        if (petEntry == 416)
            lesserNpc = NPC_LESSER_IMP;
        // Felguard family
        else if (petEntry == 17252)
            lesserNpc = NPC_LESSER_FELGUARD;
        // Voidwalker family
        else if (petEntry == 1860)
            lesserNpc = NPC_LESSER_VOIDWALKER;
        else
            lesserNpc = NPC_LESSER_IMP; // fallback

        if (Creature* lesser = owner->SummonCreature(lesserNpc,
            creature->GetPositionX(), creature->GetPositionY(),
            creature->GetPositionZ(), creature->GetOrientation(),
            TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000))
        {
            lesser->SetOwnerGUID(owner->GetGUID());
            lesser->SetCreatorGUID(owner->GetGUID());
            lesser->SetFaction(owner->GetFaction());

            if (victim->IsAlive())
            {
                lesser->Attack(victim, true);
                lesser->GetMotionMaster()->MoveChase(victim);
            }
        }
    }
};

// ============================================================
//  900837: Imp Firebolt +9 targets
//  Hooked on Imp Firebolt (47964). After hitting main target,
//  deals same damage to up to 9 additional enemies within 10yd.
// ============================================================
class spell_custom_wlk_imp_fb_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_imp_fb_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        // Caster is the Imp — check owner
        Unit* ownerUnit = caster->GetOwner();
        if (!ownerUnit)
            return;

        Player* owner = ownerUnit->ToPlayer();
        if (!owner)
            return;

        if (!owner->HasAura(SPELL_WLK_DEMO_IMP_FB_AOE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        // Find up to 9 nearby enemies
        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, caster, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(target, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        uint32 count = 0;
        for (Unit* extra : targets)
        {
            if (count >= 9)
                break;
            if (!extra->IsAlive() || !caster->IsValidAttackTarget(extra))
                continue;

            caster->CastCustomSpell(SPELL_WLK_DEMO_IMP_FB_HELPER, SPELLVALUE_BASE_POINT0,
                damage, extra, true);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_wlk_imp_fb_aoe::HandleAfterHit);
    }
};

// ============================================================
//  900838: Felguard AoE unlimited targets
//  Hooked on Felguard Cleave (47994). Override target selection
//  to remove the target cap.
// ============================================================
class spell_custom_wlk_fg_unlim : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_fg_unlim);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // Check owner has the passive
        Unit* ownerUnit = caster->GetOwner();
        if (!ownerUnit)
            return;

        Player* owner = ownerUnit->ToPlayer();
        if (!owner || !owner->HasAura(SPELL_WLK_DEMO_FG_UNLIM_TARGETS))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // No filtering — allow all targets (unlimited)
        // The default hook would cap targets; we keep them all.
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(
            spell_custom_wlk_fg_unlim::FilterTargets,
            EFFECT_0, TARGET_UNIT_CONE_ENEMY_24);
    }
};

// ============================================================
//  900840: Sacrificing pet grants ALL pet bonuses
//  Hooked on Demonic Sacrifice (18788). After cast, apply all
//  sacrifice buffs regardless of which pet was sacrificed.
// ============================================================
class spell_custom_wlk_sacrifice_all : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_sacrifice_all);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_WLK_DEMO_SACRIFICE_ALL))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Apply all sacrifice buffs
        static const uint32 sacrificeBuffs[] = {
            SPELL_SACRIFICE_IMP,       // +15% Fire damage
            SPELL_SACRIFICE_VW,        // HP regen
            SPELL_SACRIFICE_SUCCUBUS,  // +15% Shadow damage
            SPELL_SACRIFICE_FELHUNTER  // Mana regen
        };

        for (uint32 buffId : sacrificeBuffs)
        {
            if (!player->HasAura(buffId))
                player->CastSpell(player, buffId, true);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_wlk_sacrifice_all::HandleAfterCast);
    }
};

// ============================================================
//  End Warlock Demonology section
// ============================================================

// ============================================================
//  WARLOCK DESTRUCTION
// ============================================================

// ============================================================
//  900866: Shadow Bolt +9 targets
//  Hooked on Shadow Bolt (all ranks via -47809). After hitting
//  main target, deals same damage to up to 9 additional enemies
//  within 10yd via helper spell 900871.
// ============================================================
class spell_custom_wlk_sb_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_sb_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_WLK_DEST_SB_AOE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, player, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(target, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        uint32 count = 0;
        for (Unit* extra : targets)
        {
            if (count >= 9)
                break;
            if (!extra->IsAlive() || !player->IsValidAttackTarget(extra))
                continue;

            player->CastCustomSpell(SPELL_WLK_DEST_SB_HELPER, SPELLVALUE_BASE_POINT0,
                damage, extra, true);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_wlk_sb_aoe::HandleAfterHit);
    }
};

// ============================================================
//  900870: Chaos Bolt +9 targets
//  Hooked on Chaos Bolt (all ranks via -59172). After hitting
//  main target, deals same damage to up to 9 additional enemies
//  within 10yd via helper spell 900872. Helper has
//  SPELL_ATTR0_NO_IMMUNITIES to mimic Chaos Bolt's resistance
//  ignoring behavior.
// ============================================================
class spell_custom_wlk_cb_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_wlk_cb_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_WLK_DEST_CB_AOE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(target, player, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(target, targets, check);
        Cell::VisitObjects(target, searcher, 10.0f);
        targets.remove(target);

        uint32 count = 0;
        for (Unit* extra : targets)
        {
            if (count >= 9)
                break;
            if (!extra->IsAlive() || !player->IsValidAttackTarget(extra))
                continue;

            player->CastCustomSpell(SPELL_WLK_DEST_CB_HELPER, SPELLVALUE_BASE_POINT0,
                damage, extra, true);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_wlk_cb_aoe::HandleAfterHit);
    }
};

// ============================================================
//  End Warlock Destruction section
// ============================================================

void AddWarlockSpellsScripts()
{
    // Warlock Affliction
    RegisterSpellScript(spell_custom_wlk_dot_aoe);
    RegisterSpellScript(spell_custom_wlk_dot_spread);

    // Warlock Demonology
    new custom_wlk_meta_kill_extend_playerscript();
    RegisterSpellScript(spell_custom_wlk_meta_aoe_heal);
    new custom_wlk_lesser_demon_unitscript();
    RegisterSpellScript(spell_custom_wlk_imp_fb_aoe);
    RegisterSpellScript(spell_custom_wlk_fg_unlim);
    RegisterSpellScript(spell_custom_wlk_sacrifice_all);

    // Warlock Destruction
    RegisterSpellScript(spell_custom_wlk_sb_aoe);
    RegisterSpellScript(spell_custom_wlk_cb_aoe);
}
