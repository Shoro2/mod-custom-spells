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
#include "AllSpellScript.h"
#include "GlobalScript.h"

#include <algorithm>
#include <unordered_set>

// ============================================================
//  Rogue spells of the concept revision of 2026-10-08 (share-public
//  custom-spells/09-concept-revision-20261008.md). File-local: the
//  shared header keeps the ids of the earlier rounds.
// ============================================================
namespace
{
    enum RogueRevisionSpells
    {
        // ---- Rogue Assa (900605-900632) ----
        SPELL_ROG_ASSA_MUTI_AOE_PASSIVE     = 900605, // DBC only (jump targets 10 on the strikes)

        // ---- Rogue Combat (900639-900665) ----
        SPELL_ROG_COMBAT_AR_PASSIVE         = 900639,
        SPELL_ROG_COMBAT_AR_SPEED           = 900640,
        SPELL_ROG_COMBAT_AR_BURST           = 900641,
        SPELL_ROG_COMBAT_SPREE_KNIVES       = 900642,

        // ---- Rogue Sub (900670-900699) ----
        SPELL_ROG_SUB_AMBUSH_AOE_PASSIVE    = 900670, // DBC only (jump targets 10)
        SPELL_ROG_SUB_AMBUSH_DMG_PASSIVE    = 900671, // DBC only
        SPELL_ROG_SUB_DANCE_FLOW_PASSIVE    = 900672,
        SPELL_ROG_SUB_DANCE_FLOW_AURA       = 900673, // energy costs -100 %, GCD -0.5 s
        SPELL_ROG_SUB_FRONTAL_PASSIVE       = 900674,
    };

    constexpr uint32 SPELL_ROGUE_ADRENALINE_RUSH       = 13750;
    constexpr uint32 SPELL_ROGUE_FAN_OF_KNIVES         = 51723;
    constexpr uint32 SPELL_ROGUE_KILLING_SPREE_STRIKE  = 57841; // each Killing Spree tick

    constexpr float  SPREE_KNIVES_DAMAGE_MULTIPLIER = 2.0f;  // +100 %
    constexpr uint8  COMBO_FRENZY_POINTS            = 5;
    // Spell::TriggerGlobalCooldown applies GCD modifiers only to spells whose
    // own GCD lies in this range and clamps the result to it
    constexpr int32  CORE_GCD_MIN_MS                = 1000;
    constexpr int32  CORE_GCD_MAX_MS                = 1500;
    constexpr int32  DANCE_GCD_FLOOR_MS             = 100;

    // "damage scales with the Paragon level": 666 + 5 x the true level, as
    // the DK concept spells (Bloodworm Burst 900307, Frozen Strike 900342)
    int32 ParagonDamage(Player* player)
    {
        return 666 + 5 * int32(GetParagonLevel(player));
    }

    // ---- "behind the target" abilities from the front (shared) ----
    // The markers that lift the requirement for their owner: the rogue's
    // 900674 and the druid's 901053 (AddFrontalAttackMarker).
    std::vector<uint32>& FrontalAttackMarkers()
    {
        static std::vector<uint32> markers;
        return markers;
    }

    // Every spell whose spell_custom_attr row carried
    // SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET (filled once at startup)
    std::unordered_set<uint32>& BehindOnlySpells()
    {
        static std::unordered_set<uint32> spells;
        return spells;
    }

    // ---- 5-combo-point finisher bursts during a cooldown (shared) ----
    struct ComboFrenzyRule
    {
        uint32 buff;        // the cooldown's aura: Adrenaline Rush, Berserk
        uint32 passive;     // the marker that turns it on
        uint32 speedAura;   // +100 % movement speed while the buff lasts
        uint32 burstSpell;  // the area burst a 5-point finisher unleashes
    };

    std::vector<ComboFrenzyRule>& ComboFrenzyRules()
    {
        static std::vector<ComboFrenzyRule> rules;
        return rules;
    }

    ComboFrenzyRule const* FindComboFrenzyRule(uint32 buff)
    {
        for (ComboFrenzyRule const& rule : ComboFrenzyRules())
            if (rule.buff == buff)
                return &rule;
        return nullptr;
    }

    // The player whose Killing Spree strike is casting its Fan of Knives
    // right now (world-thread local, set only for that one triggered cast and
    // the off-hand Fan of Knives the core links to it)
    thread_local ObjectGuid s_SpreeKnivesCaster;
}

// Registration for the druid too (declared in custom_spells_common.h).
void AddFrontalAttackMarker(uint32 markerSpellId)
{
    FrontalAttackMarkers().push_back(markerSpellId);
}

void AddComboFrenzyRule(uint32 buffSpellId, uint32 passiveSpellId,
    uint32 speedAuraId, uint32 burstSpellId)
{
    ComboFrenzyRules().push_back({ buffSpellId, passiveSpellId, speedAuraId,
        burstSpellId });
}

// ============================================================
//  ROGUE ASSA: Poison Nova proc (900603)
//  Revision 2026-10-08: dealing damage with a poison has a chance of
//  5 % to trigger a Poison Nova on the target, damage scaling with the
//  Paragon level. spell_proc: Instant, Deadly and Wound Poison only
//  (rogue family flags0 0x10012000), their hits and Deadly Poison's
//  ticks, Nature, 5 %, no cooldown (none in the concept: a nova can
//  never set off another one - its own spell is not in the poison mask,
//  and CheckProc excludes it as well - so the count stays bounded by the
//  poison events, about one nova per 20 of them); weapon poisons are
//  triggered casts, so the row carries PROC_ATTR_TRIGGERED_CAN_PROC.
//  The nova (900604) deals 666 + 5 x the rogue's Paragon level Nature
//  to every other enemy within 10 yd of the target (was 800-1,000 on a
//  15 % / 3 s Nature proc).
// ============================================================
class spell_custom_rog_poison_nova : public AuraScript
{
    PrepareAuraScript(spell_custom_rog_poison_nova);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        return spellInfo && spellInfo->Id != SPELL_ROG_ASSA_POISON_NOVA_HELPER;
    }

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

        // Cast Poison Nova AoE centered on target
        CastAnchoredBurst(player, target, SPELL_ROG_ASSA_POISON_NOVA_HELPER,
            ParagonDamage(player));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_custom_rog_poison_nova::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_custom_rog_poison_nova::HandleProc,
            EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// ============================================================
//  ROGUE COMBAT: Blade Flurry +9 targets (900636)
//  Blade Flurry's extra hit is the core's proc 22482 (one other
//  enemy near the rogue gets the hit's damage). Hooked on 22482:
//  the same damage goes to up to 8 more enemies within 8 yd of the
//  rogue - the rogue's victim and 22482's own target excluded - so a
//  hit reaches up to 10 enemies. (The old jump-target modifier sat
//  on the Blade Flurry self-aura and changed nothing.)
// ============================================================
class spell_custom_rog_bf_targets : public SpellScript
{
    PrepareSpellScript(spell_custom_rog_bf_targets);

    void HandleAfterHit()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* bfTarget = GetHitUnit();
        if (!player || !bfTarget || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_ROG_COMBAT_BF_TARGETS_PASSIVE))
            return;

        int32 const damage = GetHitDamage();
        if (damage <= 0)
            return;

        std::list<Unit*> targets;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(player, player, 8.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck>
            searcher(player, targets, check);
        Cell::VisitObjects(player, searcher, 8.0f);
        targets.remove(bfTarget);
        targets.remove(player->GetVictim());

        SpellInfo const* spellInfo = GetSpellInfo();
        uint32 count = 0;
        for (Unit* target : targets)
        {
            if (count >= BLADE_FLURRY_EXTRA_TARGETS)
                break;
            if (!target->IsAlive() || !player->IsValidAttackTarget(target))
                continue;

            SpellNonMeleeDamage dmgInfo(player, target, spellInfo,
                spellInfo->GetSchoolMask());
            dmgInfo.damage = damage;
            player->DealSpellDamage(&dmgInfo, true);
            player->SendSpellNonMeleeDamageLog(&dmgInfo);
            ++count;
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_custom_rog_bf_targets::HandleAfterHit);
    }
};

// ============================================================
//  ROGUE COMBAT: 900642 Killing Spree: Fan of Knives
//  Concept: each tick of Killing Spree autocasts a free, off-cooldown
//  Fan of Knives with 100 % increased damage.
//  Every Killing Spree tick (the core's spell_rog_killing_spree_aura)
//  teleports the rogue to its next target and strikes with 57841; after
//  that strike the marked rogue casts Fan of Knives (51723), triggered:
//  no energy, no cooldown, round the rogue's new spot. Its main-hand and
//  off-hand parts (52874, linked by spell_linked_spell to 51723) deal
//  twice their weapon damage (Spell::SetScriptWeaponDamageMultiplier,
//  set before the launch, only for this cast and its linked off-hand).
// ============================================================
class spell_custom_rog_spree_knives : public SpellScript
{
    PrepareSpellScript(spell_custom_rog_spree_knives);

    void HandleAfterCast()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !player->IsAlive() || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_ROG_COMBAT_SPREE_KNIVES))
            return;

        s_SpreeKnivesCaster = player->GetGUID();
        player->CastSpell(player, SPELL_ROGUE_FAN_OF_KNIVES, true);
        s_SpreeKnivesCaster.Clear();
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_custom_rog_spree_knives::HandleAfterCast);
    }
};

class spell_custom_rog_spree_knives_damage : public SpellScript
{
    PrepareSpellScript(spell_custom_rog_spree_knives_damage);

    void HandleBeforeCast()
    {
        if (!s_SpreeKnivesCaster.IsEmpty() && GetCaster()
            && GetCaster()->GetGUID() == s_SpreeKnivesCaster)
            GetSpell()->SetScriptWeaponDamageMultiplier(
                SPREE_KNIVES_DAMAGE_MULTIPLIER);
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_custom_rog_spree_knives_damage::HandleBeforeCast);
    }
};

// ============================================================
//  ROGUE SUB: 900672 Shadow Dance: Flow
//  Concept: while in Shadow Dance your energy costs are reduced by
//  100 % and your global cooldown by 50 % (at least 0.1 sec).
//  - While Shadow Dance (51713) lasts, a marked rogue carries 900673:
//    aura 72 power costs -100 % (every school) and a flat
//    SPELLMOD_GLOBAL_COOLDOWN of -0.5 s on the rogue's abilities (their
//    global cooldown is 1 s, so -50 %; the client learns that modifier).
//    Flat, not -50 %: the core applies a percent global cooldown
//    modifier only to a cast whose cast time the same aura has already
//    changed (its Backdraft rule in Player::ApplySpellMod), so a percent
//    one would never apply.
//  - The core clamps every global cooldown of 1-1.5 s at 1 s
//    (Spell::TriggerGlobalCooldown, MIN_GCD). Right after it has set
//    the cooldown (OnSpellPrepare runs after it) this hook sets it
//    again for a rogue with 900673: the spell's own value with the
//    rogue's GCD modifiers, at least 0.1 s - so the 1 s of an ability
//    becomes 0.5 s. Triggered casts set no global cooldown and are left
//    alone. The client runs its own global cooldown; whether it also
//    stops at 1 s is not known here (bots have no client).
// ============================================================
class spell_custom_rog_shadow_dance_flow : public AuraScript
{
    PrepareAuraScript(spell_custom_rog_shadow_dance_flow);

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !g_CustomSpellsEnabled
            || !player->HasAura(SPELL_ROG_SUB_DANCE_FLOW_PASSIVE))
            return;

        player->CastSpell(player, SPELL_ROG_SUB_DANCE_FLOW_AURA, true);
        if (Aura* flow = player->GetAura(SPELL_ROG_SUB_DANCE_FLOW_AURA))
        {
            flow->SetMaxDuration(GetAura()->GetMaxDuration());
            flow->SetDuration(GetAura()->GetDuration());
        }
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->RemoveAurasDueToSpell(SPELL_ROG_SUB_DANCE_FLOW_AURA);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(
            spell_custom_rog_shadow_dance_flow::HandleApply, EFFECT_0,
            SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(
            spell_custom_rog_shadow_dance_flow::HandleRemove, EFFECT_0,
            SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class custom_rog_shadow_dance_gcd_allspell : public AllSpellScript
{
public:
    custom_rog_shadow_dance_gcd_allspell()
        : AllSpellScript("custom_rog_shadow_dance_gcd_allspell",
            { ALLSPELLHOOK_ON_PREPARE }) { }

    void OnSpellPrepare(Spell* spell, Unit* caster,
        SpellInfo const* spellInfo) override
    {
        if (!caster || !spell || !g_CustomSpellsEnabled
            || spellInfo->StartRecoveryTime < uint32(CORE_GCD_MIN_MS)
            || spellInfo->StartRecoveryTime > uint32(CORE_GCD_MAX_MS)
            || !spellInfo->StartRecoveryCategory
            || spell->HasTriggeredCastFlag(TRIGGERED_IGNORE_GCD))
            return;

        Player* player = caster->ToPlayer();
        if (!player || !player->HasAura(SPELL_ROG_SUB_DANCE_FLOW_AURA)
            || !player->GetGlobalCooldownMgr().HasGlobalCooldown(spellInfo))
            return;

        int32 gcd = int32(spellInfo->StartRecoveryTime);
        player->ApplySpellMod(spellInfo->Id, SPELLMOD_GLOBAL_COOLDOWN, gcd, spell);
        gcd = std::clamp(gcd, DANCE_GCD_FLOOR_MS, CORE_GCD_MAX_MS);
        player->GetGlobalCooldownMgr().AddGlobalCooldown(spellInfo, uint32(gcd));
    }
};

// ============================================================
//  "Behind the target" abilities from the front (shared)
//  Concept (Rogue Sub 900674, Druid Feral DPS 901053): you can now use
//  all spells requiring standing behind the target also from the front.
//  The core takes the requirement from the world table spell_custom_attr
//  (attribute 0x20000 SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET: Backstab,
//  Garrote, Ambush, Shred, Ravage and a few NPC spells) and tests it
//  inside Spell::CheckCast, where no script can lift it (a script's
//  CheckCast hook can only add a failure). So the module takes the
//  check over:
//  - at startup (GlobalScript OnLoadSpellCustomAttr, after the table has
//    been applied) every spell with the attribute is remembered and the
//    attribute is taken off its SpellInfo;
//  - at the start of every CheckCast of such a spell (AllSpellScript
//    OnSpellCheckCast) the same test the core made - a unit target that
//    is not the caster and has the caster in its front arc - fails the
//    cast with SPELL_FAILED_NOT_BEHIND, unless the caster is a player
//    with one of the markers and the module is enabled.
//  For everyone else nothing changes but the order of the errors: the
//  "must be behind" answer now comes before the cooldown and power
//  checks. The attribute's second use (Unit::MeleeSpellHitResult: an
//  attack from behind on a unit with an ignore-hit-direction aura) no
//  longer applies to these spells.
//  Client: Spell.dbc has no flag of its own for "behind" - these spells
//  share AttributesEx 0x200 / AttributesEx2 0x100000 with Mutilate and
//  Pounce, which need no position - so the 3.3.5a client has nothing to
//  refuse the cast with before it asks the server (not verified with a
//  real client).
// ============================================================
class custom_frontal_attack_loader : public GlobalScript
{
public:
    custom_frontal_attack_loader()
        : GlobalScript("custom_frontal_attack_loader",
            { GLOBALHOOK_ON_LOAD_SPELL_CUSTOM_ATTR }) { }

    void OnLoadSpellCustomAttr(SpellInfo* spellInfo) override
    {
        if (!spellInfo
            || !(spellInfo->AttributesCu & SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET))
            return;

        BehindOnlySpells().insert(spellInfo->Id);
        spellInfo->AttributesCu &= ~uint32(SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET);
    }
};

class custom_frontal_attack_allspell : public AllSpellScript
{
public:
    custom_frontal_attack_allspell()
        : AllSpellScript("custom_frontal_attack_allspell",
            { ALLSPELLHOOK_ON_SPELL_CHECK_CAST }) { }

    void OnSpellCheckCast(Spell* spell, bool /*strict*/, SpellCastResult& res) override
    {
        if (res != SPELL_CAST_OK || !spell
            || !BehindOnlySpells().count(spell->GetSpellInfo()->Id))
            return;

        Unit* caster = spell->GetCaster() ? spell->GetCaster()->ToUnit() : nullptr;
        Unit* target = spell->m_targets.GetUnitTarget();
        if (!caster || !target || target == caster
            || !target->HasInArc(float(M_PI), caster))
            return;

        if (g_CustomSpellsEnabled)
            if (Player* player = caster->ToPlayer())
                for (uint32 marker : FrontalAttackMarkers())
                    if (player->HasAura(marker))
                        return;

        res = SPELL_FAILED_NOT_BEHIND;
    }
};

// ============================================================
//  Cooldown frenzy (shared): Rogue Combat 900639 Adrenaline Rush,
//  Druid Feral DPS 901054 Berserk
//  Concept: <cooldown> increases your movement speed by 100 % and
//  whenever you spend 5 combo points you unleash a melee AoE around you,
//  damage increasing with the Paragon level.
//  - While the cooldown's aura lasts, a marked player carries its speed
//    aura (aura 31 +100 %, the cooldown's duration).
//  - A finisher (combo point spell) that lands on its target (miss,
//    dodge or parry keep the points: no burst) with 5 points, cast by
//    the player itself while the cooldown runs, unleashes the burst:
//    666 + 5 x the Paragon level Physical to every enemy within 8 yd of
//    the player. The points are read before the finisher's effects
//    (OnSpellBeforeEffects - a killing blow clears them before the hit
//    result), the burst goes off at its first landed hit
//    (OnSpellHitResult), once per cast. A finisher on the player itself
//    (Slice and Dice, Savage Roar) counts the same.
// ============================================================
class spell_custom_combo_frenzy : public AuraScript
{
    PrepareAuraScript(spell_custom_combo_frenzy);

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        ComboFrenzyRule const* rule = FindComboFrenzyRule(GetId());
        Player* player = GetTarget()->ToPlayer();
        if (!rule || !player || !g_CustomSpellsEnabled
            || !player->HasAura(rule->passive))
            return;

        player->CastSpell(player, rule->speedAura, true);
        if (Aura* speed = player->GetAura(rule->speedAura))
        {
            speed->SetMaxDuration(GetAura()->GetMaxDuration());
            speed->SetDuration(GetAura()->GetDuration());
        }
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (ComboFrenzyRule const* rule = FindComboFrenzyRule(GetId()))
            GetTarget()->RemoveAurasDueToSpell(rule->speedAura);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_custom_combo_frenzy::HandleApply,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_custom_combo_frenzy::HandleRemove,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class custom_combo_frenzy_allspell : public AllSpellScript
{
public:
    custom_combo_frenzy_allspell()
        : AllSpellScript("custom_combo_frenzy_allspell",
            { ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_HIT_RESULT }) { }

    // The points a finisher spends, taken before its effects: a finisher
    // that kills its target clears them (Unit::Kill) before the hit result
    void OnSpellBeforeEffects(Spell* spell, Unit* caster,
        SpellInfo const* spellInfo) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || !spell || !spellInfo->NeedsComboPoints() || spell->IsTriggered()
            || !g_CustomSpellsEnabled || !ActiveRule(player))
            return;

        spell->SetScriptValue(COMBO_FRENZY_SNAPSHOT_KEY,
            uint64(player->GetComboPoints()) + 1);
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 missInfo,
        uint32 /*damage*/, uint32 /*healing*/, bool /*critical*/) override
    {
        if (missInfo != SPELL_MISS_NONE || !spell || !target
            || spell->GetScriptValue(COMBO_FRENZY_SNAPSHOT_KEY) < COMBO_FRENZY_POINTS + 1u
            || spell->GetScriptValue(COMBO_FRENZY_DONE_KEY))
            return;

        Player* player = spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        ComboFrenzyRule const* rule = player ? ActiveRule(player) : nullptr;
        if (!rule)
            return;

        spell->SetScriptValue(COMBO_FRENZY_DONE_KEY, 1);
        int32 amount = ParagonDamage(player);
        player->CastCustomSpell(player, rule->burstSpell, &amount, nullptr,
            nullptr, true);
    }

private:
    // Spell::SetScriptValue keys are spell ids: two of this module's own
    static constexpr uint32 COMBO_FRENZY_SNAPSHOT_KEY = SPELL_ROG_COMBAT_AR_PASSIVE;
    static constexpr uint32 COMBO_FRENZY_DONE_KEY     = SPELL_ROG_COMBAT_AR_BURST;

    static ComboFrenzyRule const* ActiveRule(Player* player)
    {
        for (ComboFrenzyRule const& rule : ComboFrenzyRules())
            if (player->HasAura(rule.buff) && player->HasAura(rule.passive))
                return &rule;
        return nullptr;
    }
};

// ============================================================
//  ROGUE: DBC-only passives of the revision (spellmods in
//  mod_custom_spells_d_hunter_druid_rogue.sql):
//  900605 Mutilate +9 targets (jump targets 10 on the two hand strikes
//  48665 / 48664, flags1 0x6 - not on Mutilate 48666 itself: it only
//  triggers the strikes, and a strike triggered on a far chain target
//  fails its melee range check), 900670 / 900671 Ambush +9 targets
//  (jump targets 10) / +50 %.
// ============================================================

// ============================================================
//  End Rogue section
// ============================================================

void AddRogueSpellsScripts()
{
    // Rogue Assa
    RegisterSpellScript(spell_custom_rog_poison_nova);

    // Rogue Combat
    RegisterSpellScript(spell_custom_rog_bf_targets);
    RegisterSpellScript(spell_custom_rog_spree_knives);
    RegisterSpellScript(spell_custom_rog_spree_knives_damage);
    AddComboFrenzyRule(SPELL_ROGUE_ADRENALINE_RUSH, SPELL_ROG_COMBAT_AR_PASSIVE,
        SPELL_ROG_COMBAT_AR_SPEED, SPELL_ROG_COMBAT_AR_BURST);

    // Rogue Sub
    RegisterSpellScript(spell_custom_rog_shadow_dance_flow);
    new custom_rog_shadow_dance_gcd_allspell();
    AddFrontalAttackMarker(SPELL_ROG_SUB_FRONTAL_PASSIVE);

    // Shared with the druid (Feral DPS registers its marker and Berserk)
    new custom_frontal_attack_loader();
    new custom_frontal_attack_allspell();
    RegisterSpellScript(spell_custom_combo_frenzy);
    new custom_combo_frenzy_allspell();
}
