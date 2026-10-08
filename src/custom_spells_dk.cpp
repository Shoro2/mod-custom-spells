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
#include "AllCreatureScript.h"
#include "DynamicObject.h"

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

        if (!g_CustomSpellsEnabled)
            return;

        // Summon 2 extra rune weapons by re-casting the summon effect (triggered).
        // Re-entrancy guard: each triggered re-cast of Dancing Rune Weapon would
        // otherwise re-enter this AfterCast handler and recurse without bound.
        static thread_local bool reentry = false;
        if (reentry)
            return;
        reentry = true;
        for (int i = 0; i < 2; ++i)
            player->CastSpell(player, SPELL_DANCING_RUNE_WEAPON, true);
        reentry = false;

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
        // Don't prevent default â€” let the original DRW proc fire.
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

        if (!g_CustomSpellsEnabled)
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
//  SPELL 900304: Dealing Damage -> Death Coil Proc (AuraScript)
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

        if (!g_CustomSpellsEnabled)
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
//  The Frost Wyrm has 2Ã— Gargoyle HP and casts Frost Breath.
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

        if (!g_CustomSpellsEnabled)
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
//  SPELL 900366: DoTs -> Shadow AoE Proc (AuraScript)
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

        if (!g_CustomSpellsEnabled)
            return;

        Unit* target = eventInfo.GetActionTarget();
        if (!target || !target->IsAlive())
            return;

        // Cast Shadow AoE helper at target
        CastAnchoredBurst(player, target, SPELL_DKU_SHADOW_AOE_HELPER);

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
//  DK BLOOD: 900306 Bloodworm Burst
//  Concept: blood worms explode on death and deal 666 + 5 x Paragon
//  level damage to all enemies within 10 yd. A worm (28017, summon
//  50452, 20 s) of a marked DK gets the fuse 900308 when it enters
//  the world; the fuse lasts 19 s, so it ends before the summon
//  despawns. Its end - expiry or the worm's death - sets off the
//  burst 900307 (Shadow, 10 yd round the worm, the DK's spell).
//  Any other removal (unsummon, owner gone) stays quiet.
// ============================================================
class custom_dkb_bloodworm_creature : public AllCreatureScript
{
public:
    custom_dkb_bloodworm_creature()
        : AllCreatureScript("custom_dkb_bloodworm_creature") { }

    void OnCreatureAddWorld(Creature* creature) override
    {
        if (creature->GetEntry() != NPC_DK_BLOODWORM || !g_CustomSpellsEnabled)
            return;

        Player* owner = creature->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner || !owner->HasAura(SPELL_DKB_BLOODWORM_BURST_PASSIVE))
            return;

        creature->AddAura(SPELL_DKB_BLOODWORM_FUSE, creature);
    }
};

class spell_custom_dkb_bloodworm_fuse : public AuraScript
{
    PrepareAuraScript(spell_custom_dkb_bloodworm_fuse);

    void HandleRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode const mode = GetTargetApplication()->GetRemoveMode();
        if (mode != AURA_REMOVE_BY_EXPIRE && mode != AURA_REMOVE_BY_DEATH)
            return;

        Unit* worm = GetTarget();
        Player* owner = worm->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner || !owner->IsInMap(worm) || !g_CustomSpellsEnabled
            || !owner->HasAura(SPELL_DKB_BLOODWORM_BURST_PASSIVE))
            return;

        SpellInfo const* burst = sSpellMgr->GetSpellInfo(
            SPELL_DKB_BLOODWORM_BURST_DAMAGE);
        if (!burst)
            return;

        // the worm may already be dead: aim at its spot, not at the unit
        SpellCastTargets targets;
        targets.SetDst(*worm);
        CustomSpellValues values;
        values.AddSpellMod(SPELLVALUE_BASE_POINT0,
            int32(666 + 5 * GetParagonLevel(owner)));
        owner->CastSpell(targets, burst, &values, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_dkb_bloodworm_fuse::HandleRemove, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// ============================================================
//  DK FROST: 900337 Howling Blast applies Frost Fever
//  Hooked on Howling Blast (all ranks via -49184): every enemy the
//  blast damages gets the DK's Frost Fever (55095).
// ============================================================
class spell_custom_dkf_hb_frost_fever : public SpellScript
{
    PrepareSpellScript(spell_custom_dkf_hb_frost_fever);

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !target->IsAlive() || !g_CustomSpellsEnabled)
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->HasAura(SPELL_DKF_HB_FROST_FEVER_PASSIVE))
            return;

        player->CastSpell(target, SPELL_DK_FROST_FEVER, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_dkf_hb_frost_fever::HandleAfterHit);
    }
};

// ============================================================
//  DK FROST: 900338 Lichborne also leeches 30 % of damage done
//  Hooked on Lichborne (49039): while it lasts, the DK carries
//  900339, whose proc (damage done, spell_proc) heals the DK for
//  30 % of the damage.
// ============================================================
class spell_custom_dkf_lichborne_leech : public AuraScript
{
    PrepareAuraScript(spell_custom_dkf_lichborne_leech);

    void HandleApply(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_DKF_LICHBORNE_LEECH_PASSIVE))
            return;

        player->CastSpell(player, SPELL_DKF_LICHBORNE_LEECH_AURA, true);
        if (Aura* leech = player->GetAura(SPELL_DKF_LICHBORNE_LEECH_AURA))
        {
            leech->SetMaxDuration(GetAura()->GetMaxDuration());
            leech->SetDuration(GetAura()->GetDuration());
        }
    }

    void HandleRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_DKF_LICHBORNE_LEECH_AURA);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_custom_dkf_lichborne_leech::HandleApply, EFFECT_0,
            SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_dkf_lichborne_leech::HandleRemove, EFFECT_0,
            SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_custom_dkf_lichborne_leech_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_dkf_lichborne_leech_proc);

    void HandleProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        Unit* target = GetTarget();
        if (!damageInfo || !damageInfo->GetDamage() || !target->IsAlive())
            return;

        uint32 heal = CalculatePct(damageInfo->GetDamage(), 30);
        HealInfo healInfo(target, target, heal, GetSpellInfo(),
            GetSpellInfo()->GetSchoolMask());
        target->HealBySpell(healInfo);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_dkf_lichborne_leech_proc::HandleProc, EFFECT_0,
            SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  DK FROST: 900340 Improved Icy Talons gives Frozen Strikes
//  Improved Icy Talons (55610) is an area aura on the DK's party/raid
//  (the DK included). While a player carries it from a marked DK,
//  that player has 900341: melee hits (spell_proc: 10 %) strike the
//  target with 900342, Frost damage 666 + 5 x the DK's Paragon level
//  (taken when the aura reaches the player, kept in its amount).
// ============================================================
class spell_custom_dkf_frozen_strikes_grant : public AuraScript
{
    PrepareAuraScript(spell_custom_dkf_frozen_strikes_grant);

    void HandleApply(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        Player* dk = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!dk || !target->IsPlayer() || !g_CustomSpellsEnabled
            || !dk->HasAura(SPELL_DKF_FROZEN_STRIKES_PASSIVE))
            return;

        int32 amount = int32(666 + 5 * GetParagonLevel(dk));
        dk->CastCustomSpell(target, SPELL_DKF_FROZEN_STRIKES_AURA, &amount,
            nullptr, nullptr, true);
    }

    void HandleRemove(AuraEffect const* /*aurEff*/,
        AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_DKF_FROZEN_STRIKES_AURA,
            GetCasterGUID());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_custom_dkf_frozen_strikes_grant::HandleApply, EFFECT_0,
            SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_dkf_frozen_strikes_grant::HandleRemove, EFFECT_0,
            SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_custom_dkf_frozen_strikes_proc : public AuraScript
{
    PrepareAuraScript(spell_custom_dkf_frozen_strikes_proc);

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        Unit* victim = eventInfo.GetActionTarget();
        Unit* target = GetTarget();
        if (!victim || !victim->IsAlive() || victim == target)
            return;

        int32 amount = aurEff->GetAmount();
        if (amount <= 0)
            return;

        target->CastCustomSpell(victim, SPELL_DKF_FROZEN_STRIKE_DAMAGE,
            &amount, nullptr, nullptr, true, nullptr, aurEff);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(
            spell_custom_dkf_frozen_strikes_proc::HandleProc, EFFECT_0,
            SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  DK UNHOLY: 900369 Ghoul Cleave
//  A white melee hit of the DK's ghoul (26125) or an Army of the
//  Dead ghoul (24207) deals 50 % of its damage to every other enemy
//  within 5 yd of the victim (helper 900370). Only white hits count
//  (DealDamage with DIRECT_DAMAGE): the cleave itself is spell
//  damage and cannot set off another cleave.
// ============================================================
class custom_dku_ghoul_cleave_unitscript : public UnitScript
{
public:
    custom_dku_ghoul_cleave_unitscript()
        : UnitScript("custom_dku_ghoul_cleave_unitscript") { }

    uint32 DealDamage(Unit* attacker, Unit* victim, uint32 damage,
        DamageEffectType damagetype) override
    {
        if (damagetype != DIRECT_DAMAGE || !attacker || !victim
            || damage < 2 || !g_CustomSpellsEnabled)
            return damage;

        Creature* ghoul = attacker->ToCreature();
        if (!ghoul || (ghoul->GetEntry() != NPC_DK_GHOUL
            && ghoul->GetEntry() != NPC_DK_ARMY_GHOUL))
            return damage;

        Player* owner = ghoul->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!owner || !owner->HasAura(SPELL_DKU_GHOUL_CLEAVE_PASSIVE))
            return damage;

        CastAnchoredBurst(ghoul, victim, SPELL_DKU_GHOUL_CLEAVE_DAMAGE,
            int32(damage / 2));
        return damage;
    }
};

// ============================================================
//  DK UNHOLY: 900373 Death and Decay around you
//  Hooked on Death and Decay (all ranks via -43265), like the
//  paladin's mobile Consecration: the ground area is removed right
//  after the cast and the DK carries 900374 for its duration. Each
//  second the ticker strikes every enemy within 10 yd of the DK with
//  the core's Death and Decay tick (52212, its glyph and AoE rules).
// ============================================================
class spell_custom_dku_dnd_around : public SpellScript
{
    PrepareSpellScript(spell_custom_dku_dnd_around);

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_DKU_DND_AROUND_PASSIVE))
            return;

        uint32 const spellId = GetSpellInfo()->Id;
        int32 tick = GetSpellInfo()->Effects[EFFECT_0].CalcValue(player);
        int32 duration = GetSpellInfo()->GetMaxDuration();
        player->ApplySpellMod(spellId, SPELLMOD_DURATION, duration);
        if (DynamicObject* area = player->GetDynObject(spellId))
            duration = area->GetDuration();

        if (tick <= 0 || duration <= 0)
            return;

        player->RemoveDynObject(spellId);
        player->CastCustomSpell(player, SPELL_DKU_DND_MOBILE_AURA, &tick,
            nullptr, nullptr, true);
        if (Aura* ticker = player->GetAura(SPELL_DKU_DND_MOBILE_AURA))
        {
            ticker->SetMaxDuration(duration);
            ticker->SetDuration(duration);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_dku_dnd_around::HandleAfterCast);
    }
};

class spell_custom_dku_dnd_mobile : public AuraScript
{
    PrepareAuraScript(spell_custom_dku_dnd_mobile);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsAlive() || !g_CustomSpellsEnabled)
            return;

        int32 amount = aurEff->GetAmount();
        if (amount <= 0)
            return;

        // Ground visual: a short-lived area tagged with Death and Decay
        // makes the client draw it at the DK's current spot each tick
        DynamicObject* visual = new DynamicObject();
        if (!visual->CreateDynamicObject(
            player->GetMap()->GenerateLowGuid<HighGuid::DynamicObject>(),
            player, SPELL_DK_DND_VISUAL, *player, 10.0f,
            DYNAMIC_OBJECT_AREA_SPELL))
            delete visual;
        else
            visual->SetDuration(1100);

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(player, player, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(player, targets, check);
        Cell::VisitObjects(player, searcher, 10.0f);

        for (Unit* enemy : targets)
        {
            if (!enemy->IsAlive() || !player->IsValidAttackTarget(enemy))
                continue;

            player->CastCustomSpell(enemy, SPELL_DK_DND_DAMAGE, &amount,
                nullptr, nullptr, true, nullptr, aurEff);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(
            spell_custom_dku_dnd_mobile::HandlePeriodic, EFFECT_0,
            SPELL_AURA_PERIODIC_DUMMY);
    }
};

// ============================================================

void AddDKSpellsScripts()
{
    // DK Blood
    RegisterSpellScript(spell_custom_dkb_3_rune_weapons);
    RegisterSpellScript(spell_custom_dkb_double_cast);
    RegisterSpellScript(spell_custom_dkb_deathcoil_proc);
    new custom_dkb_bloodworm_creature();
    RegisterSpellScript(spell_custom_dkb_bloodworm_fuse);

    // DK Frost
    RegisterSpellScript(spell_custom_dkf_frost_wyrm);
    RegisterCreatureAI(npc_custom_frost_wyrm);
    RegisterSpellScript(spell_custom_frost_breath);
    RegisterSpellScript(spell_custom_dkf_hb_frost_fever);
    RegisterSpellScript(spell_custom_dkf_lichborne_leech);
    RegisterSpellScript(spell_custom_dkf_lichborne_leech_proc);
    RegisterSpellScript(spell_custom_dkf_frozen_strikes_grant);
    RegisterSpellScript(spell_custom_dkf_frozen_strikes_proc);

    // DK Unholy
    RegisterSpellScript(spell_custom_dku_dot_aoe);
    new custom_dku_ghoul_cleave_unitscript();
    RegisterSpellScript(spell_custom_dku_dnd_around);
    RegisterSpellScript(spell_custom_dku_dnd_mobile);
}
