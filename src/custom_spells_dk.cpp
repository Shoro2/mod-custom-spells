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
//  SPELL 900300: 3 Rune Weapons (SpellScript)
//  Hooked on Dancing Rune Weapon (49028). After cast,
//  summons 2 additional rune weapon creatures.
//  Only active when player has passive 900300.
// ============================================================
class spell_custom_dkb_3_rune_weapons : public SpellScript
{
    PrepareSpellScript(spell_custom_dkb_3_rune_weapons);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_DKB_3_RUNE_WEAPONS_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Summon 2 extra rune weapons by re-casting the summon effect (triggered)
        for (int i = 0; i < 2; ++i)
            player->CastSpell(player, SPELL_DANCING_RUNE_WEAPON, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> 3 Rune Weapons summoned",
            player->GetName());
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_dkb_3_rune_weapons::HandleAfterCast);
    }
};

// ============================================================
//  SPELL 900301: Rune Weapon Double Casts (AuraScript)
//  Hooked on Dancing Rune Weapon aura (49028). Overrides
//  the proc to cast each spell twice instead of once.
//  Only active when player has passive 900301.
// ============================================================
class spell_custom_dkb_double_cast : public AuraScript
{
    PrepareAuraScript(spell_custom_dkb_double_cast);

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        // Don't prevent default — let the original DRW proc fire.
        // We just add ONE extra cast (so total = 2).

        Unit* player = eventInfo.GetActor();
        Unit* target = eventInfo.GetActionTarget();
        if (!player || !target || !target->IsAlive())
            return;

        Player* pl = player->ToPlayer();
        if (!pl)
            return;

        if (!pl->HasAura(SPELL_DKB_DOUBLE_CAST_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Find the rune weapon
        Unit* dancingRuneWeapon = nullptr;
        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            if ((*itr)->GetEntry() == NPC_DK_RUNE_WEAPON)
            {
                dancingRuneWeapon = *itr;
                break;
            }
        }

        if (!dancingRuneWeapon)
            return;

        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (procSpell)
        {
            // Cast the spell one extra time from the rune weapon
            dancingRuneWeapon->CastSpell(target, procSpell->Id, true, nullptr, aurEff, dancingRuneWeapon->GetGUID());
        }
        else if (eventInfo.GetDamageInfo())
        {
            // Extra melee hit at half damage (same as base DRW behavior)
            CalcDamageInfo damageInfo;
            player->CalculateMeleeDamage(target, &damageInfo, eventInfo.GetDamageInfo()->GetAttackType());
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                Unit::DealDamageMods(target, damageInfo.damages[i].damage, &damageInfo.damages[i].absorb);
                damageInfo.damages[i].damage /= 2.0f;
            }
            damageInfo.attacker = dancingRuneWeapon;
            dancingRuneWeapon->SendAttackStateUpdate(&damageInfo);
            dancingRuneWeapon->DealMeleeDamage(&damageInfo, true);
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_dkb_double_cast::HandleProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900303: Heart Strike +9 Targets (SpellScript)
//  Hooked on Heart Strike (55050). After hitting primary
//  target, deals same damage to up to 9 additional enemies.
//  Only active when player has passive 900303.
// ============================================================
class spell_custom_dkb_hs_aoe : public SpellScript
{
    PrepareSpellScript(spell_custom_dkb_hs_aoe);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* mainTarget = GetHitUnit();
        if (!caster || !mainTarget)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_DKB_HS_AOE_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        int32 damage = GetHitDamage();
        if (damage <= 0)
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(caster, targets, check);
        Cell::VisitObjects(caster, searcher, 8.0f);
        targets.remove(mainTarget);

        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= 9)
                break;
            if (!target->IsAlive())
                continue;

            Unit::DealDamage(caster, target, static_cast<uint32>(damage),
                nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL);
            ++count;
        }

        if (count > 0)
            LOG_INFO("module",
                "mod-custom-spells: Player {} -> HS AoE hit {} extra targets",
                player->GetName(), count);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_dkb_hs_aoe::HandleAfterHit);
    }
};

// ============================================================
//  SPELL 900304: Dealing Damage → Death Coil Proc (AuraScript)
//  Passive proc aura: when dealing melee/spell damage,
//  X% chance to auto-cast Death Coil on target.
//  Only active when player has passive 900304.
// ============================================================
class spell_custom_dkb_deathcoil_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_dkb_deathcoil_proc);

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

        // Cast Death Coil damage effect (triggered, no cost)
        player->CastSpell(target, SPELL_DK_DEATH_COIL_DMG, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> proc Death Coil on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_dkb_deathcoil_proc::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  SPELL 900333: Replace Ghoul with Frost Wyrm (SpellScript)
//  Hooked on Raise Dead (46584). After cast, despawns the
//  ghoul and summons a custom Frost Wyrm (NPC 900333) instead.
//  The Frost Wyrm has 2× Gargoyle HP and casts Frost Breath.
// ============================================================
class spell_custom_dkf_frost_wyrm : public SpellScript
{
    PrepareSpellScript(spell_custom_dkf_frost_wyrm);

    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!player->HasAura(SPELL_DKF_FROST_WYRM_PASSIVE))
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        // Find and despawn the ghoul
        for (auto itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
        {
            if ((*itr)->GetEntry() == NPC_DK_GHOUL && (*itr)->ToCreature())
            {
                (*itr)->ToCreature()->DespawnOrUnsummon();
                break;
            }
        }

        // Summon Frost Wyrm as guardian (follows + fights)
        if (Creature* wyrm = player->SummonCreature(NPC_DK_FROST_WYRM,
            player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),
            player->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000))
        {
            wyrm->SetOwnerGUID(player->GetGUID());
            wyrm->SetCreatorGUID(player->GetGUID());
            wyrm->SetFaction(player->GetFaction());
            wyrm->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

            if (Unit* victim = player->GetVictim())
            {
                wyrm->Attack(victim, true);
                wyrm->GetMotionMaster()->MoveChase(victim);
            }

            LOG_INFO("module",
                "mod-custom-spells: Player {} -> Frost Wyrm (NPC {}) summoned",
                player->GetName(), NPC_DK_FROST_WYRM);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_dkf_frost_wyrm::HandleAfterCast);
    }
};

// ============================================================
//  NPC 900333: Frost Wyrm CreatureScript
//  AI: Follows owner, auto-casts Frost Breath (900368) on
//  victim. Frost Breath is a 2s cast, cone 20yd, deals
//  5000 base damage + AP scaling, slows targets.
// ============================================================
struct npc_custom_frost_wyrm : public ScriptedAI
{
    npc_custom_frost_wyrm(Creature* creature) : ScriptedAI(creature)
    {
        _breathTimer = 0;
        _selectionTimer = 0;
    }

    void InitializeAI() override
    {
        ScriptedAI::InitializeAI();
        me->SetReactState(REACT_AGGRESSIVE);

        Unit* owner = me->GetOwner();
        if (!owner)
            return;

        if (Unit* victim = owner->GetVictim())
        {
            me->Attack(victim, true);
            me->GetMotionMaster()->MoveChase(victim);
        }
        else
        {
            me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, 0.0f);
        }
    }

    void IsSummonedBy(WorldObject* summoner) override
    {
        if (!summoner || !summoner->ToUnit())
            return;

        Unit* owner = summoner->ToUnit();
        if (Unit* victim = owner->GetVictim())
        {
            me->Attack(victim, true);
            me->GetMotionMaster()->MoveChase(victim);
        }
    }

    void SelectTarget()
    {
        Unit* owner = me->GetOwner();
        if (!owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player)
            return;

        // Follow player's target selection
        Unit* selection = player->GetSelectedUnit();
        if (selection && selection != me->GetVictim() && me->IsValidAttackTarget(selection))
        {
            me->GetMotionMaster()->Clear(false);
            AttackStart(selection);
        }
        else if (!me->GetVictim() || !me->GetVictim()->IsAlive())
        {
            me->CombatStop(true);
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, 0.0f);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _selectionTimer += diff;
        if (_selectionTimer >= 1000)
        {
            SelectTarget();
            _selectionTimer = 0;
        }

        if (!UpdateVictim())
            return;

        _breathTimer += diff;

        // Cast Frost Breath every ~4s (2s cast + 2s gap)
        if (_breathTimer >= 4000)
        {
            if (!me->HasUnitState(UNIT_STATE_CASTING))
            {
                // Scale damage with owner's AP
                Unit* owner = me->GetOwner();
                if (owner)
                {
                    float ap = owner->GetTotalAttackPowerValue(BASE_ATTACK);
                    int32 damage = 5000 + static_cast<int32>(ap * 0.5f);
                    me->CastCustomSpell(me->GetVictim(), SPELL_FROST_BREATH,
                        &damage, nullptr, nullptr, false);
                }
                else
                {
                    DoCastVictim(SPELL_FROST_BREATH);
                }
                _breathTimer = 0;
            }
        }

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*who*/) override
    {
        if (me->IsSummon())
            me->ToTempSummon()->UnSummon();
    }

private:
    uint32 _breathTimer;
    uint32 _selectionTimer;
};

// ============================================================
//  SPELL 900368: Frost Breath damage handler (SpellScript)
//  Overrides damage with 5000 base + 50% owner AP scaling.
// ============================================================
class spell_custom_frost_breath : public SpellScript
{
    PrepareSpellScript(spell_custom_frost_breath);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Unit* owner = caster->GetOwner();
        if (!owner)
            return;

        float ap = owner->GetTotalAttackPowerValue(BASE_ATTACK);
        int32 damage = 5000 + static_cast<int32>(ap * 0.5f);
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_custom_frost_breath::HandleDamage,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// ============================================================
//  SPELL 900366: DoTs → Shadow AoE Proc (AuraScript)
//  Passive proc aura: when periodic damage ticks,
//  X% chance to cast Shadow AoE (900367) on target.
// ============================================================
class spell_custom_dku_dot_aoe : public AuraScript
{
    PrepareAuraScript(spell_custom_dku_dot_aoe);

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

        // Cast Shadow AoE helper at target
        player->CastSpell(target, SPELL_DKU_SHADOW_AOE_HELPER, true);

        LOG_INFO("module",
            "mod-custom-spells: Player {} -> DoT proc Shadow AoE on {}",
            player->GetName(), target->GetName());
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_custom_dku_dot_aoe::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================

void AddDKSpellsScripts()
{
    // DK Blood
    RegisterSpellScript(spell_custom_dkb_3_rune_weapons);
    RegisterSpellScript(spell_custom_dkb_double_cast);
    RegisterSpellScript(spell_custom_dkb_hs_aoe);
    RegisterSpellScript(spell_custom_dkb_deathcoil_proc);

    // DK Frost
    RegisterSpellScript(spell_custom_dkf_frost_wyrm);
    RegisterCreatureAI(npc_custom_frost_wyrm);
    RegisterSpellScript(spell_custom_frost_breath);

    // DK Unholy
    RegisterSpellScript(spell_custom_dku_dot_aoe);
}
