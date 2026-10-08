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
#include "CharmInfo.h"
#include "Chat.h"
#include "CommandScript.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "PetAI.h"
#include "SpellMgr.h"
#include "StringConvert.h"
#include "TemporarySummon.h"
#include "Util.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <cmath>
#include <numbers>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Acore::ChatCommands;

// ============================================================
//  SECOND PET: Hunter BM 900525 "you can now have 2 pets active",
//  Warlock Demonology 900858 "you can now have 2 demons active"
//  (concept revision 2026-10-08, the operator's choice "B").
//  The 3.3.5 client knows one pet - one pet bar, pet frame and pet
//  talent tab - so the second one gets its own AIO control bar
//  (lua/CustomSpells_PetBar_Client.lua and _Server.lua): attack,
//  follow, stay, the react modes, its spells with autocast, its
//  name, health and power. Its talents are the first pet's.
//
//  - Hunter: while the hunter's pet is out, a copy of it fights at
//    its side: creature 900525 with the pet's model, size, name,
//    level, passive spells (family passives, the scaling auras that
//    give it a share of the hunter's stats, pet talents), the four
//    spells of the pet's action bar with their autocast states and
//    the hunter's pet auras (talents such as Unleashed Fury). It
//    goes with the pet - dismissed, dead, unsummoned for a mount, a
//    taxi or a teleport - and comes back with it (Call Pet, Revive
//    Pet, login). Another pet, a level, a learned spell or talent, a
//    renamed pet or a rearranged pet bar makes a new copy. The pet's
//    happiness reaches the copy (its melee damage, as the pet's), and
//    the copy is drawn at the pet's size: the client sizes a pet by its
//    family, the copy by its model (ClientModelScale).
//  - Warlock: when a demon is summoned while another one is out, the
//    one that was out stays as the second demon (a copy of it under
//    its own entry), two demons at most. Summoning the type that is
//    out again changes nothing (the core keeps the same pet); a third
//    type replaces the second with the demon that was out. Dismissing
//    or losing the demon (its death, the warlock's death, a
//    sacrifice) ends the second; a mount, a taxi or a teleport only
//    hides it. A logout ends it too: the previous demon is kept in
//    memory only.
//  - A second pet that dies comes back 10 sec after its owner left
//    combat, or at once with the next pet.
//
//  It stays out of the native pet's way: it is a plain Guardian
//  (SummonProperties 61: ally, guardian, no slot), never the player's
//  pet (Player::GetPet, UNIT_FIELD_SUMMON) and never a controllable
//  guardian, so the core sends no SMSG_PET_SPELLS for it and never
//  hands it the pet bar when the real pet goes. A CharmInfo of its
//  own and PetAI make it follow, stay, attack and autocast like a pet;
//  the bar's commands reach it through WorldSession::HandlePetActionHelper,
//  for this unit only. The native bar does not reach it:
//  HandlePetAction - and the set-action and autocast opcodes - act on
//  the first controlled unit and on the controlled units OF ITS ENTRY,
//  and the second pet never shares the pet's entry (the hunter's copy
//  is creature 900525; the second demon is another demon type, and
//  Reconcile drops it if both ever match).
//  It is in the player's controlled set, so the minion manager
//  (custom_spells_global.cpp) gives it the auras of 900502 / 900503
//  and, as an Imp or a Felguard, of 900836 / 900839. Other scripts
//  recognise it by the marker aura 900526.
//
//  Bridge to the bar: server Lua cannot reach the pet's charm data, so
//  the bar's requests arrive as the player's own chat command .cspet
//  (player:RunCommand in CustomSpells_PetBar_Server.lua), checked again
//  here; the state goes the other way as addon whispers with the
//  prefix CSPB (see PushBar).
// ============================================================

namespace
{
    constexpr uint32 SPELL_HUNT_BM_SECOND_PET       = 900525; // passive
    constexpr uint32 SPELL_CUSTOM_SECOND_PET_MARKER = 900526; // on every second pet
    constexpr uint32 SPELL_WLK_DEMO_SECOND_DEMON    = 900858; // passive
    constexpr uint32 NPC_HUNTER_SECOND_PET          = 900525;

    // SummonProperties.dbc 61: category ally, type guardian, no slot
    constexpr uint32 SUMMON_PROPERTIES_ALLY_GUARDIAN = 61;

    // The real pet follows at pi/2 (PET_FOLLOW_ANGLE), the second pet on
    // the other side
    constexpr float SECOND_PET_FOLLOW_ANGLE  = std::numbers::pi_v<float> * 1.5f;
    constexpr float SECOND_PET_CATCH_UP_DIST = 80.0f;
    constexpr float HUNTER_PET_FOCUS         = 100.0f;

    constexpr uint32 RECONCILE_INTERVAL_MS = 500;
    constexpr uint32 BAR_INTERVAL_MS       = 250;
    constexpr uint32 FOCUS_REGEN_MS        = 4000; // PET_FOCUS_REGEN_INTERVAL
    constexpr uint32 RESPAWN_OOC_MS        = 10000;
    constexpr uint32 CORPSE_MS             = 3000;
    constexpr uint32 DEMON_SWAP_WAIT_MS    = 15000;

    char const* const STATE_KEY  = "mod_custom_spells_second_pet";
    char const* const BAR_PREFIX = "CSPB";

    // Set while a second pet is summoned: OnPlayerBeforeGuardianInitStatsForLevel
    // gives it a hunter pet's or a demon's stats instead of a plain guardian's
    thread_local uint32 t_creatingEntry = 0;
    thread_local PetType t_creatingType = MAX_PET_TYPE;

    enum SecondPetKind : uint8
    {
        SECOND_PET_NONE,
        SECOND_PET_HUNTER,
        SECOND_PET_WARLOCK
    };

    struct SecondPetSpell
    {
        uint32 Id = 0;
        bool Autocast = false;
    };

    // What a second pet is made from: the hunter's pet now, or the
    // warlock's demon from before the last summon
    struct SecondPetSource
    {
        uint32 Key = 0;             // pet number of the copied pet
        uint32 Entry = 0;           // creature entry of the second pet
        uint32 DisplayId = 0;
        float Scale = 1.0f;         // the pet's OBJECT_FIELD_SCALE_X without scale auras
        uint32 Family = 0;          // the pet's creature family (its client size)
        bool Numbered = false;      // the pet has a pet number (ditto)
        uint8 Level = 0;
        PetType Type = MAX_PET_TYPE;
        ReactStates React = REACT_DEFENSIVE;
        std::string Name;
        uint32 NameTimestamp = 0;
        uint32 Signature = 0;       // a change means a new copy
        std::vector<uint32> Passives;
        std::vector<SecondPetSpell> Spells; // the pet bar's, MAX_SPELL_CHARM at most
    };

    class SecondPetState : public DataMap::Base
    {
    public:
        ObjectGuid Guid;            // the second pet in the world
        uint32 Key = 0;             // its source's key and signature
        uint32 Signature = 0;
        float Scale = 1.0f;         // its source's size (SecondPetSource)
        uint32 Family = 0;
        bool Numbered = false;

        SecondPetSource Demon;      // warlock: the demon before the last summon
        bool DemonValid = false;
        uint32 DemonSwapMs = 0;     // a summon is replacing the demon

        // kept while copies are made from the same source again
        uint32 SettingsKey = 0;
        ReactStates React = REACT_DEFENSIVE;
        std::unordered_map<uint32, bool> Autocast;
        float HealthPct = 0.0f;

        // the owner's pet auras (spell_pet_auras: talents, 900507 Pet Health)
        // put on the current second pet - Unit::AddPetAura / RemovePetAura
        // reach Player::GetPet() only
        std::set<uint32> PetAuras;

        bool Dead = false;          // the second pet died, RespawnMs to go
        uint32 DeadKey = 0;
        uint32 RespawnMs = 0;

        uint32 ReconcileMs = 0;
        uint32 BarMs = 0;
        uint32 FocusMs = 0;

        // what the bar was told last
        bool BarShown = false;
        std::string LastFull;
        std::string LastStatus;
        std::unordered_map<uint32, uint32> LastCooldown;
    };

    SecondPetState* GetState(Player* player)
    {
        return player->CustomData.GetDefault<SecondPetState>(STATE_KEY);
    }

    bool IsWarlockDemon(uint32 entry)
    {
        switch (entry)
        {
            case NPC_IMP:
            case NPC_VOIDWALKER:
            case NPC_SUCCUBUS:
            case NPC_FELHUNTER:
            case NPC_FELGUARD:
                return true;
            default:
                return false;
        }
    }

    std::string_view TrimView(std::string_view text)
    {
        while (!text.empty() && (text.front() == ' ' || text.front() == '\t'))
            text.remove_prefix(1);
        while (!text.empty() && (text.back() == ' ' || text.back() == '\t'))
            text.remove_suffix(1);
        return text;
    }

    // The bar's separators must not appear inside a field
    std::string BarSafe(std::string text)
    {
        for (char& c : text)
            if (c == ';' || c == ',' || c == ':' || c == '|' || c == '\t' || c == '\n')
                c = ' ';
        return text;
    }

    char const* ReactName(ReactStates react)
    {
        switch (react)
        {
            case REACT_PASSIVE:
                return "passive";
            case REACT_DEFENSIVE:
                return "defensive";
            default:
                return "aggressive";
        }
    }

    char const* PowerName(Powers power)
    {
        switch (power)
        {
            case POWER_MANA:
                return "mana";
            case POWER_RAGE:
                return "rage";
            case POWER_FOCUS:
                return "focus";
            case POWER_ENERGY:
                return "energy";
            default:
                return "power";
        }
    }

    // The live second pet, or nullptr once it is gone (despawned, on
    // another map after a teleport); forgets a stale guid
    Creature* FindSecondPet(Player* player, SecondPetState& state)
    {
        if (state.Guid.IsEmpty())
            return nullptr;

        Creature* second = ObjectAccessor::GetCreature(*player, state.Guid);
        if (!second || !second->IsInWorld() || second->GetOwnerGUID() != player->GetGUID()
            || !second->IsSummon()
            || second->ToTempSummon()->GetSummonType() == TEMPSUMMON_DESPAWNED)
        {
            state.Guid.Clear();
            return nullptr;
        }
        return second;
    }

    // The size the 3.3.5a client draws a creature model at before it applies
    // OBJECT_FIELD_SCALE_X (build 12340: 0x0071C110, reached from the unit's
    // model setup through 0x00722AE0): the model's own size (CreatureDisplayInfo
    // x CreatureModelData), raised to its creature family's size at the unit's
    // level - and set to the family's size outright for a unit with a pet
    // number. So a hunter pet is drawn at its family's size whatever creature
    // it was tamed from, the copy (creature 900525, no family) at its model's:
    // the Diseased Young Wolf's 0.4 against the wolf family's 1.0 at level 60+.
    // A humanoid look (ExtendedDisplayInfoID) also takes its race model's size
    // in, by the look's sex, which the server's DBC stores do not load: 0 =
    // unknown (beasts and demons have none).
    float ClientModelScale(uint32 displayId, uint32 familyId, uint8 level, bool petNumber)
    {
        CreatureDisplayInfoEntry const* display = sCreatureDisplayInfoStore.LookupEntry(displayId);
        CreatureModelDataEntry const* model = display
            ? sCreatureModelDataStore.LookupEntry(display->ModelId) : nullptr;
        if (!model)
            return 1.0f;
        if (display->ExtendedDisplayInfoID)
            return 0.0f;

        float scale = display->scale * model->Scale;
        if (scale <= 0.0f)
            scale = 1.0f;

        CreatureFamilyEntry const* family = sCreatureFamilyStore.LookupEntry(familyId);
        if (!family)
            return scale;

        int32 const range = int32(family->maxScaleLevel) - int32(family->minScaleLevel);
        int32 step = int32(level) >= int32(family->minScaleLevel)
            ? int32(level) - int32(family->minScaleLevel) : 0;
        if (step > range)
            step = range;
        float const part = range ? float(step) / float(range) : 0.0f;
        float const familyScale = (family->maxScale - family->minScale) * part + family->minScale;
        return familyScale > scale || petNumber ? familyScale : scale;
    }

    // How much larger the client draws the pet than the second pet in its
    // current look at the same OBJECT_FIELD_SCALE_X (1 for a second demon:
    // the demon's own entry)
    float SecondPetSizeRatio(Creature* second, SecondPetState const& state)
    {
        uint32 const displayId = second->GetDisplayId();
        uint8 const level = second->GetLevel();
        float const petSize = ClientModelScale(displayId, state.Family, level, state.Numbered);
        float const ownSize = ClientModelScale(displayId, second->GetCreatureTemplate()->family, level,
            second->GetUInt32Value(UNIT_FIELD_PETNUMBER) != 0);
        return petSize > 0.0f && ownSize > 0.0f ? petSize / ownSize : 1.0f;
    }

    // The second pet drawn at the pet's size: the pet's scale (plus scale
    // auras, as Unit::RecalculateObjectScale adds them) times the size ratio,
    // with a pet's reach for that scale (Creature::SetObjectScale: 1 and
    // DEFAULT_COMBAT_REACH times the scale for a pet, the model's for a
    // guardian). Checked every half second: a scale aura coming or going, or a
    // look restored, sets the guardian's scale from its own template again.
    void ApplySecondPetSize(Creature* second, SecondPetState const& state, bool force)
    {
        int32 const auras = second->GetTotalAuraModifier(SPELL_AURA_MOD_SCALE)
            + second->GetTotalAuraModifier(SPELL_AURA_MOD_SCALE_2);
        float const petScale = std::max(state.Scale + CalculatePct(1.0f, auras), 0.01f);
        float const scale = petScale * SecondPetSizeRatio(second, state);
        if (!force && std::fabs(second->GetObjectScale() - scale) < 0.0001f)
            return;

        second->SetObjectScale(scale);
        second->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, petScale);
        second->SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_COMBAT_REACH * petScale);
    }

    // The hunter pet whose copy the second pet is, or nullptr (a second demon)
    Pet* CopiedHunterPet(Player* player, Creature* second)
    {
        Pet* pet = player->GetPet();
        return pet && pet->IsHunterPet() && second->GetEntry() == NPC_HUNTER_SECOND_PET ? pet : nullptr;
    }

    // What a hunter pet's happiness adds to its melee weapon damage
    // (Guardian::UpdateDamagePhysical: 125 % happy, 75 % unhappy), in percent
    int32 HappinessDamagePct(Pet* pet)
    {
        switch (pet->GetHappinessState())
        {
            case HAPPY:
                return 25;
            case UNHAPPY:
                return -25;
            default:
                return 0;
        }
    }

    // The hunter pet's happiness on its copy. The core gives it to a hunter
    // pet only, never to a guardian: the marker's second effect, a physical
    // damage-done aura, does the same for the copy - physical percent auras
    // reach the weapon damage only (Unit::UpdateDamagePctDoneMods;
    // MeleeDamageBonusDone skips them). 0 on a second demon.
    void MirrorHappiness(Player* player, Creature* second)
    {
        AuraEffect* effect = second->GetAuraEffect(SPELL_CUSTOM_SECOND_PET_MARKER, EFFECT_1);
        if (!effect)
            return;

        Pet* pet = CopiedHunterPet(player, second);
        int32 const amount = pet ? HappinessDamagePct(pet) : 0;
        if (effect->GetAmount() != amount)
            effect->ChangeAmount(amount);
    }

    // full = false fills only what Reconcile compares (key, entry,
    // signature); a copy needs the full snapshot
    void SnapshotPet(Pet* pet, uint32 entry, SecondPetSource& out, bool full)
    {
        CharmInfo* charmInfo = pet->GetCharmInfo();
        out.Key = charmInfo->GetPetNumber();
        out.Entry = entry;
        out.Type = pet->getPetType();
        out.Level = pet->GetLevel();
        out.DisplayId = pet->GetNativeDisplayId();
        out.NameTimestamp = pet->GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP);

        // another look, level or name, a spell or talent learned or lost,
        // another pet bar: the copy is made again
        uint32 signature = (uint32(out.Level) * 2654435761u) ^ out.DisplayId ^ (out.NameTimestamp << 1);
        for (auto const& [spellId, petSpell] : pet->m_spells)
            if (petSpell.state != PETSPELL_REMOVED)
                signature += spellId * 2654435761u;

        if (full)
        {
            out.Spells.clear();
            out.Passives.clear();
        }

        uint8 count = 0;
        for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX && count < MAX_SPELL_CHARM; ++i)
        {
            UnitActionBarEntry const* button = charmInfo->GetActionBarEntry(i);
            uint32 const spellId = button->GetAction();
            if (!spellId || !button->IsActionBarForSpell())
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo || spellInfo->IsPassive())
                continue;

            ++count;
            signature = signature * 31 + spellId;
            if (full)
                out.Spells.push_back({ spellId, button->GetType() == ACT_ENABLED });
        }
        out.Signature = signature;

        if (!full)
            return;

        int32 const scaleAuras = pet->GetTotalAuraModifier(SPELL_AURA_MOD_SCALE)
            + pet->GetTotalAuraModifier(SPELL_AURA_MOD_SCALE_2);
        out.Scale = pet->GetObjectScale() - CalculatePct(1.0f, scaleAuras);
        if (out.Scale <= 0.0f)
            out.Scale = pet->GetNativeObjectScale();
        out.Family = pet->GetCreatureTemplate()->family;
        out.Numbered = pet->GetUInt32Value(UNIT_FIELD_PETNUMBER) != 0;
        out.React = pet->GetReactState();
        out.Name = pet->GetName();
        for (auto const& [spellId, petSpell] : pet->m_spells)
            if (petSpell.state != PETSPELL_REMOVED)
                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
                    if (spellInfo->IsPassive())
                        out.Passives.push_back(spellId);
    }

    // Which second pet the player should have now
    SecondPetKind WantedSource(Player* player, SecondPetState& state, SecondPetSource& out)
    {
        Pet* pet = player->GetPet();
        bool const petOut = pet && pet->IsInWorld() && pet->IsAlive()
            && !pet->isBeingLoaded() && pet->GetCharmInfo();
        bool const warlock = g_CustomSpellsEnabled
            && player->HasAura(SPELL_WLK_DEMO_SECOND_DEMON);

        // The previous demon is kept while a summon replaces the demon and
        // while the demon is only unsummoned for a while (mount, taxi,
        // teleport); dismissed or lost, the second demon ends with it. A pet
        // out ends the swap (the summon's effect removes the old demon in the
        // same cast, before any check here), so a dismiss right after a swap
        // ends the second demon at once.
        if (petOut)
            state.DemonSwapMs = 0;
        if (!warlock)
            state.DemonValid = false;
        else if (state.DemonValid && !petOut && !state.DemonSwapMs
            && !player->GetTemporaryUnsummonedPetNumber())
            state.DemonValid = false;

        if (!g_CustomSpellsEnabled || !player->IsAlive() || !petOut)
            return SECOND_PET_NONE;

        if (pet->getPetType() == HUNTER_PET && player->HasAura(SPELL_HUNT_BM_SECOND_PET))
        {
            SnapshotPet(pet, NPC_HUNTER_SECOND_PET, out, false);
            return SECOND_PET_HUNTER;
        }

        if (warlock && state.DemonValid && pet->getPetType() == SUMMON_PET
            && IsWarlockDemon(pet->GetEntry()) && pet->GetEntry() != state.Demon.Entry)
        {
            out = state.Demon;
            return SECOND_PET_WARLOCK;
        }

        return SECOND_PET_NONE;
    }

    void SetAutocast(Creature* second, SpellInfo const* spellInfo, bool on)
    {
        CharmInfo* charmInfo = second->GetCharmInfo();
        if (!charmInfo || spellInfo->IsPassive() || !spellInfo->IsAutocastable())
            return;

        charmInfo->ToggleCreatureAutocast(spellInfo, on);
        charmInfo->SetSpellAutocast(spellInfo, on);
    }

    bool IsAutocastOn(Creature* second, uint32 spellId)
    {
        CharmInfo* charmInfo = second->GetCharmInfo();
        for (uint8 i = 0; i < MAX_SPELL_CHARM; ++i)
            if (charmInfo->GetCharmSpell(i)->GetAction() == spellId)
                return charmInfo->GetCharmSpell(i)->GetType() == ACT_ENABLED;
        return false;
    }

    // The owner's pet auras on the second pet: each one cast once, and taken
    // away again when the owner loses it (a passive or talent unlearned)
    void SyncPetAuras(Player* player, SecondPetState& state, Creature* second)
    {
        std::set<uint32> wanted;
        for (PetAura const* petAura : player->m_petAuras)
            if (uint32 const auraId = petAura->GetAura(second->GetEntry()))
            {
                wanted.insert(auraId);
                if (state.PetAuras.insert(auraId).second)
                    second->CastPetAura(petAura);
            }

        for (auto itr = state.PetAuras.begin(); itr != state.PetAuras.end();)
        {
            if (wanted.count(*itr))
            {
                ++itr;
                continue;
            }
            second->RemoveAurasDueToSpell(*itr);
            itr = state.PetAuras.erase(itr);
        }
    }

    void DespawnSecondPet(Player* player, SecondPetState& state)
    {
        if (Creature* second = FindSecondPet(player, state))
        {
            if (second->IsAlive())
                state.HealthPct = second->GetHealthPct();
            if (TempSummon* summon = second->ToTempSummon())
                summon->UnSummon();
        }

        state.Guid.Clear();
        state.Key = 0;
        state.Signature = 0;
    }

    Creature* CreateSecondPet(Player* player, SecondPetState& state, SecondPetSource const& source)
    {
        SummonPropertiesEntry const* properties =
            sSummonPropertiesStore.LookupEntry(SUMMON_PROPERTIES_ALLY_GUARDIAN);
        if (!properties || !sObjectMgr->GetCreatureTemplate(source.Entry))
            return nullptr;

        float x, y, z;
        player->GetClosePoint(x, y, z, player->GetObjectSize(), PET_FOLLOW_DIST,
            SECOND_PET_FOLLOW_ANGLE);
        Position const pos(x, y, z, player->GetOrientation());

        t_creatingEntry = source.Entry;
        t_creatingType = source.Type;
        TempSummon* summon = player->SummonCreature(source.Entry, pos,
            TEMPSUMMON_MANUAL_DESPAWN, 0, 0, properties);
        Guardian* guardian = summon && !summon->IsPet()
            && summon->HasUnitTypeMask(UNIT_MASK_GUARDIAN) ? static_cast<Guardian*>(summon) : nullptr;
        if (guardian && source.Level && guardian->GetLevel() != source.Level)
            guardian->InitStatsForLevel(source.Level);
        t_creatingEntry = 0;
        t_creatingType = MAX_PET_TYPE;

        if (!guardian)
        {
            if (summon)
                summon->UnSummon();
            LOG_ERROR("module", "mod-custom-spells: second pet {} of {} could not be summoned",
                source.Entry, player->GetGUID().ToString());
            return nullptr;
        }

        // the copied pet's look and name; the pet number lets the client
        // show the pet's name over the copy as it does over the pet
        if (source.DisplayId)
        {
            guardian->SetDisplayId(source.DisplayId);
            guardian->SetNativeDisplayId(source.DisplayId);
        }
        if (!source.Name.empty())
            guardian->SetName(source.Name);

        CharmInfo* charmInfo = guardian->InitCharmInfo();
        charmInfo->SetPetNumber(source.Key, true);
        guardian->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, source.NameTimestamp);

        // the pet's size, once the copy has its look and pet number
        state.Scale = source.Scale;
        state.Family = source.Family;
        state.Numbered = source.Numbered;
        ApplySecondPetSize(guardian, state, true);

        // the pet bar's spells become the second pet's charm spells
        for (uint32& spellId : guardian->m_spells)
            spellId = 0;
        std::size_t const spellCount = std::min<std::size_t>(source.Spells.size(), MAX_SPELL_CHARM);
        for (std::size_t i = 0; i < spellCount; ++i)
            guardian->m_spells[i] = source.Spells[i].Id;
        charmInfo->InitCharmCreateSpells();

        bool const sameSource = state.SettingsKey == source.Key;
        for (SecondPetSpell const& spell : source.Spells)
        {
            bool autocast = spell.Autocast;
            if (sameSource)
            {
                auto const itr = state.Autocast.find(spell.Id);
                if (itr != state.Autocast.end())
                    autocast = itr->second;
            }
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell.Id))
                SetAutocast(guardian, spellInfo, autocast);
        }

        // the pet's passives (family, scaling, pet talents), the owner's pet
        // auras (talents) and the marker
        for (uint32 spellId : source.Passives)
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
                if (!spellInfo->CasterAuraState
                    || guardian->HasAuraState(AuraStateType(spellInfo->CasterAuraState)))
                    guardian->CastSpell(guardian, spellId, true);
        state.PetAuras.clear();
        SyncPetAuras(player, state, guardian);
        guardian->AddAura(SPELL_CUSTOM_SECOND_PET_MARKER, guardian);
        MirrorHappiness(player, guardian);

        // a hunter pet's focus: Unit::GetCreatePowers gives it to real
        // hunter pets only, Pet::Update regenerates it (here: MaintainSecondPet)
        if (source.Type == HUNTER_PET)
        {
            guardian->setPowerType(POWER_FOCUS);
            guardian->SetStatFlatModifier(UNIT_MOD_FOCUS, BASE_VALUE, HUNTER_PET_FOCUS);
            guardian->UpdateMaxPower(POWER_FOCUS);
        }

        // the auras above raised the maximum health and mana, not the current
        // values: full, or what a copy of the same pet had when it went
        Powers const power = guardian->getPowerType();
        guardian->SetPower(power, guardian->GetMaxPower(power));
        if (sameSource && state.HealthPct > 0.0f)
            guardian->SetHealth(std::max<uint32>(1,
                uint32(guardian->GetMaxHealth() * state.HealthPct / 100.0f)));
        else
            guardian->SetFullHealth();

        // the pet's AI with the pet's orders; a stock demon entry would
        // otherwise run the AI of a hostile guardian
        guardian->AIM_Initialize(new PetAI(guardian));
        guardian->AttackStop();
        guardian->CombatStop(true);
        guardian->SetFollowAngle(SECOND_PET_FOLLOW_ANGLE);

        ReactStates const react = sameSource ? state.React : source.React;
        WorldSession* session = player->GetSession();
        session->HandlePetActionHelper(guardian, guardian->GetGUID(), react,
            ACT_REACTION, ObjectGuid::Empty);
        session->HandlePetActionHelper(guardian, guardian->GetGUID(), COMMAND_FOLLOW,
            ACT_COMMAND, ObjectGuid::Empty);

        if (!sameSource)
        {
            state.SettingsKey = source.Key;
            state.React = source.React;
            state.Autocast.clear();
            for (SecondPetSpell const& spell : source.Spells)
                state.Autocast[spell.Id] = spell.Autocast;
        }
        state.HealthPct = 0.0f;
        state.Guid = guardian->GetGUID();
        state.Key = source.Key;
        state.Signature = source.Signature;
        state.FocusMs = 0;

        LOG_DEBUG("module", "mod-custom-spells: {} has second pet {} (entry {}, copy of pet number {})",
            player->GetGUID().ToString(), guardian->GetGUID().ToString(), source.Entry, source.Key);
        return guardian;
    }

    void RegenerateFocus(Creature* second)
    {
        if (second->GetPower(POWER_FOCUS) >= second->GetMaxPower(POWER_FOCUS))
            return;

        // as Creature::Regenerate does for a hunter pet every 4 sec
        float gain = 24.0f * sWorld->getRate(RATE_POWER_FOCUS);
        gain *= second->GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_FOCUS);
        gain += second->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_FOCUS)
            * float(FOCUS_REGEN_MS) / (5 * IN_MILLISECONDS);
        second->ModifyPower(POWER_FOCUS, int32(gain));
    }

    void MaintainSecondPet(Player* player, SecondPetState& state, Creature* second, uint32 elapsed)
    {
        // charmed by somebody else: the end of the charm restores the
        // creature's own AI - start over with a fresh copy
        if (second->IsCharmed() || !second->GetCharmInfo())
        {
            DespawnSecondPet(player, state);
            return;
        }

        state.HealthPct = second->GetHealthPct();
        SyncPetAuras(player, state, second);
        ApplySecondPetSize(second, state, false);
        MirrorHappiness(player, second);

        // left behind by its owner's near teleport: bring it along
        if (!second->IsWithinDistInMap(player, SECOND_PET_CATCH_UP_DIST))
        {
            float x, y, z;
            player->GetClosePoint(x, y, z, second->GetObjectSize(), PET_FOLLOW_DIST,
                SECOND_PET_FOLLOW_ANGLE);
            second->CombatStop(true);
            second->NearTeleportTo(x, y, z, player->GetOrientation());
            player->GetSession()->HandlePetActionHelper(second, second->GetGUID(),
                COMMAND_FOLLOW, ACT_COMMAND, ObjectGuid::Empty);
        }

        if (second->getPowerType() == POWER_FOCUS)
        {
            state.FocusMs += elapsed;
            if (state.FocusMs >= FOCUS_REGEN_MS)
            {
                state.FocusMs = 0;
                RegenerateFocus(second);
            }
        }
    }

    void Reconcile(Player* player, SecondPetState& state, uint32 elapsed)
    {
        // a teleport or a login in progress: decide once the player is placed
        if (!player->IsInWorld() || player->IsBeingTeleported())
            return;

        if (state.DemonSwapMs)
            state.DemonSwapMs = elapsed < state.DemonSwapMs ? state.DemonSwapMs - elapsed : 0;

        Creature* second = FindSecondPet(player, state);

        SecondPetSource wanted;
        SecondPetKind const kind = WantedSource(player, state, wanted);
        if (kind == SECOND_PET_NONE)
        {
            if (second)
                DespawnSecondPet(player, state);
            // a second pet that died comes back with the next pet at once
            state.Dead = false;
            state.RespawnMs = 0;
            return;
        }

        if (second && (state.Key != wanted.Key || state.Signature != wanted.Signature
            || second->GetEntry() != wanted.Entry))
        {
            DespawnSecondPet(player, state);
            second = nullptr;
        }

        if (second)
        {
            if (second->IsAlive())
            {
                MaintainSecondPet(player, state, second, elapsed);
                return;
            }

            if (!state.Dead)
            {
                state.Dead = true;
                state.DeadKey = state.Key;
                state.RespawnMs = RESPAWN_OOC_MS;
                state.HealthPct = 0.0f;
                if (TempSummon* summon = second->ToTempSummon())
                    summon->UnSummon(Milliseconds(CORPSE_MS));
            }
            return;
        }

        // killed: back 10 sec after the owner left combat (another pet: at once)
        if (state.Dead && state.DeadKey == wanted.Key)
        {
            if (!player->IsInCombat())
                state.RespawnMs = elapsed < state.RespawnMs ? state.RespawnMs - elapsed : 0;
            if (state.RespawnMs)
                return;
        }
        state.Dead = false;

        if (kind == SECOND_PET_HUNTER)
            if (Pet* pet = player->GetPet())
                SnapshotPet(pet, NPC_HUNTER_SECOND_PET, wanted, true);

        // a summon that fails is tried again after the same wait, not
        // every half second
        if (!CreateSecondPet(player, state, wanted))
        {
            state.Dead = true;
            state.DeadKey = wanted.Key;
            state.RespawnMs = RESPAWN_OOC_MS;
        }
    }

    // ------------------------------------------------------------ the bar
    // The state reaches CustomSpells_PetBar_Client.lua as addon whispers
    // from the player to himself (as AIO's own messages), prefix CSPB:
    //   X                                       no second pet: hide the bar
    //   F;name;level;power type;react;command;spells
    //      spells = id:autocastable:autocast,...  (react 0 passive, 1 defensive,
    //      2 aggressive; command 0 stay, 1 follow)
    //   S;alive;health;max health;power;max power;attacking
    //   C;spell id;cooldown left in ms           when a cooldown starts or ends early
    void SendBar(Player* player, std::string const& text)
    {
        WorldSession* session = player->GetSession();
        if (!session)
            return;

        WorldPacket data;
        ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, LANG_ADDON, player->GetGUID(),
            player->GetGUID(), std::string(BAR_PREFIX) + '\t' + text, 0);
        session->SendPacket(&data);
    }

    std::string BuildFullState(Creature* second)
    {
        CharmInfo* charmInfo = second->GetCharmInfo();
        std::string spells;
        for (uint8 i = 0; i < MAX_SPELL_CHARM; ++i)
        {
            CharmSpellInfo* charmSpell = charmInfo->GetCharmSpell(i);
            uint32 const spellId = charmSpell->GetAction();
            SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
            if (!spellInfo || spellInfo->IsPassive())
                continue;

            if (!spells.empty())
                spells += ',';
            spells += Acore::StringFormat("{}:{}:{}", spellId, spellInfo->IsAutocastable() ? 1 : 0,
                charmSpell->GetType() == ACT_ENABLED ? 1 : 0);
        }

        return Acore::StringFormat("{};{};{};{};{};{}", BarSafe(second->GetName()),
            uint32(second->GetLevel()), uint32(second->getPowerType()),
            uint32(second->GetReactState()), charmInfo->HasCommandState(COMMAND_STAY) ? 0 : 1, spells);
    }

    std::string BuildStatus(Creature* second)
    {
        Powers const power = second->getPowerType();
        return Acore::StringFormat("{};{};{};{};{};{}", second->IsAlive() ? 1 : 0, second->GetHealth(),
            second->GetMaxHealth(), second->GetPower(power), second->GetMaxPower(power),
            second->GetVictim() ? 1 : 0);
    }

    // Sends what changed since the last push (everything with force)
    void PushBar(Player* player, SecondPetState& state, bool force)
    {
        Creature* second = FindSecondPet(player, state);
        CharmInfo* charmInfo = second ? second->GetCharmInfo() : nullptr;
        if (!charmInfo)
        {
            if (state.BarShown || force)
                SendBar(player, "X");
            state.BarShown = false;
            state.LastFull.clear();
            state.LastStatus.clear();
            state.LastCooldown.clear();
            return;
        }

        std::string const full = BuildFullState(second);
        if (force || full != state.LastFull)
        {
            SendBar(player, "F;" + full);
            state.LastFull = full;
            state.LastCooldown.clear();
            force = true; // other spells: their cooldowns again
        }

        std::string const status = BuildStatus(second);
        if (force || status != state.LastStatus)
        {
            SendBar(player, "S;" + status);
            state.LastStatus = status;
        }

        // a running cooldown counts down on the client by itself
        for (uint8 i = 0; i < MAX_SPELL_CHARM; ++i)
        {
            uint32 const spellId = charmInfo->GetCharmSpell(i)->GetAction();
            if (!spellId)
                continue;

            uint32 const left = second->GetSpellCooldown(spellId);
            uint32& last = state.LastCooldown[spellId];
            if ((force && left) || left > last + BAR_INTERVAL_MS || (!left && last > 2 * BAR_INTERVAL_MS))
                SendBar(player, Acore::StringFormat("C;{};{}", spellId, left));
            last = left;
        }

        state.BarShown = true;
    }

    // ------------------------------------------------------------ commands
    // One of the second pet's charm spells, by id or by name
    SpellInfo const* FindSecondPetSpell(Creature* second, std::string_view text, LocaleConstant locale)
    {
        text = TrimView(text);
        if (text.empty())
            return nullptr;

        Optional<uint32> const wantedId = Acore::StringTo<uint32>(text);
        CharmInfo* charmInfo = second->GetCharmInfo();
        for (uint8 i = 0; i < MAX_SPELL_CHARM; ++i)
        {
            uint32 const spellId = charmInfo->GetCharmSpell(i)->GetAction();
            SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
            if (!spellInfo || spellInfo->IsPassive())
                continue;

            if (wantedId)
            {
                if (*wantedId == spellId)
                    return spellInfo;
                continue;
            }

            for (char const* name : { spellInfo->SpellName[locale], spellInfo->SpellName[LOCALE_enUS] })
                if (name && StringEqualI(text, name))
                    return spellInfo;
        }
        return nullptr;
    }

    // The native pet bar's cast (WorldSession::HandlePetCastSpellOpcode and
    // HandlePetActionHelper) for the second pet: at the player's target when
    // the spell wants one, turning to it, moving into range and casting
    // there when it is too far (Unit::PetSpellFail's forced spell)
    void CastSecondPetSpell(Player* player, Creature* second, SpellInfo const* spellInfo)
    {
        Unit* target = player->GetSelectedUnit();
        SpellCastTargets targets;
        if (spellInfo->NeedsExplicitUnitTarget())
        {
            if (target)
                targets.SetUnitTarget(target);
        }
        else if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        {
            if (target)
                targets.SetDst(*target);
            else
                targets.SetDst(*second);
        }

        bool const following = second->HasUnitState(UNIT_STATE_FOLLOW);
        second->ClearUnitState(UNIT_STATE_FOLLOW);

        Spell* spell = new Spell(second, spellInfo, TRIGGERED_NONE);
        spell->m_targets = targets;
        spell->LoadScripts();
        SpellCastResult result = spell->CheckPetCast(targets.GetUnitTarget());

        if (result == SPELL_FAILED_UNIT_NOT_INFRONT && spell->m_targets.GetUnitTarget())
        {
            second->SetFacingToObject(spell->m_targets.GetUnitTarget());
            result = SPELL_CAST_OK;
        }

        if (result == SPELL_CAST_OK)
        {
            second->AddSpellCooldown(spellInfo->Id, 0, 0);

            Unit* unitTarget = spell->m_targets.GetUnitTarget();
            if (unitTarget && !player->IsFriendlyTo(unitTarget) && second->GetVictim() != unitTarget
                && second->IsAIEnabled)
                second->AI()->AttackStart(unitTarget);

            spell->prepare(&spell->m_targets);

            if (CharmInfo* charmInfo = second->GetCharmInfo())
            {
                charmInfo->SetForcedSpell(0);
                charmInfo->SetForcedTargetGUID();
            }
        }
        else
        {
            spell->SendPetCastResult(result);
            second->PetSpellFail(spellInfo, spell->m_targets.GetUnitTarget(), result);
            spell->finish(false);
            delete spell;
        }

        if (following && !second->IsInCombat())
            second->AddUnitState(UNIT_STATE_FOLLOW);
    }
}

// ============================================================
//  The second pet's life: made, kept and removed once every 0.5 sec
//  from what the player has (the passive, the pet); the bar is told
//  four times a second what changed.
// ============================================================
class custom_second_pet_playerscript : public PlayerScript
{
public:
    custom_second_pet_playerscript() : PlayerScript("custom_second_pet_playerscript",
        { PLAYERHOOK_ON_UPDATE, PLAYERHOOK_ON_SPELL_CAST,
          PLAYERHOOK_ON_BEFORE_GUARDIAN_INIT_STATS_FOR_LEVEL }) { }

    void OnPlayerUpdate(Player* player, uint32 diff) override
    {
        SecondPetState* state = player->CustomData.Get<SecondPetState>(STATE_KEY);
        if (!state)
        {
            // nothing to do for a player who never had either passive
            if (!g_CustomSpellsEnabled || (!player->HasAura(SPELL_HUNT_BM_SECOND_PET)
                && !player->HasAura(SPELL_WLK_DEMO_SECOND_DEMON)))
                return;
            state = GetState(player);
        }

        state->ReconcileMs += diff;
        if (state->ReconcileMs >= RECONCILE_INTERVAL_MS)
        {
            uint32 const elapsed = state->ReconcileMs;
            state->ReconcileMs = 0;
            Reconcile(player, *state, elapsed);
        }

        state->BarMs += diff;
        if (state->BarMs >= BAR_INTERVAL_MS)
        {
            state->BarMs = 0;
            if (player->IsInWorld())
                PushBar(player, *state, false);
        }
    }

    // 900858: a demon summoned while another one is out - remember the one
    // that was out, before the summon's effect removes it
    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!g_CustomSpellsEnabled || !player->HasAura(SPELL_WLK_DEMO_SECOND_DEMON))
            return;

        uint32 summoned = 0;
        for (SpellEffectInfo const& effect : spell->GetSpellInfo()->GetEffects())
            if (effect.Effect == SPELL_EFFECT_SUMMON_PET && effect.MiscValue > 0)
                summoned = uint32(effect.MiscValue);
        if (!summoned)
            return;

        Pet* pet = player->GetPet();
        if (!pet || !pet->IsInWorld() || !pet->IsAlive() || !pet->GetCharmInfo()
            || pet->getPetType() != SUMMON_PET || !IsWarlockDemon(pet->GetEntry())
            || pet->GetEntry() == summoned)
            return;

        SecondPetState* state = GetState(player);
        // a second demon of the summoned type goes at once: the native pet bar
        // acts on every controlled unit of the pet's entry
        if (Creature* second = FindSecondPet(player, *state))
            if (second->GetEntry() == summoned)
                DespawnSecondPet(player, *state);

        SnapshotPet(pet, pet->GetEntry(), state->Demon, true);
        state->DemonValid = true;
        state->DemonSwapMs = DEMON_SWAP_WAIT_MS;
    }

    // A hunter pet's or a demon's stats for the second pet (pet_levelstats,
    // weapon damage, run speed) instead of a plain guardian's
    void OnPlayerBeforeGuardianInitStatsForLevel(Player* /*player*/, Guardian* guardian,
        CreatureTemplate const* /*cinfo*/, PetType& petType) override
    {
        if (t_creatingEntry && guardian->GetEntry() == t_creatingEntry && !guardian->IsPet())
            petType = t_creatingType;
    }
};

// ============================================================
//  .cspet - the bar's bridge (and the test bots' handle): every
//  subcommand acts on the invoking player's own second pet only and
//  checks what it is asked (a live second pet, a valid attack target,
//  one of the pet's spells). Usable by players; the server Lua runs it
//  with player:RunCommand.
//    .cspet attack | follow | stay
//    .cspet aggressive | defensive | passive
//    .cspet cast <spell id or name>
//    .cspet autocast <on|off|toggle> <spell id or name>
//    .cspet status        the second pet's state as text
//    .cspet sync          the bar's full state again
// ============================================================
class custom_second_pet_commandscript : public CommandScript
{
public:
    custom_second_pet_commandscript() : CommandScript("custom_second_pet_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable secondPetTable =
        {
            { "attack",     HandleAttack,     SEC_PLAYER, Console::No },
            { "follow",     HandleFollow,     SEC_PLAYER, Console::No },
            { "stay",       HandleStay,       SEC_PLAYER, Console::No },
            { "aggressive", HandleAggressive, SEC_PLAYER, Console::No },
            { "defensive",  HandleDefensive,  SEC_PLAYER, Console::No },
            { "passive",    HandlePassive,    SEC_PLAYER, Console::No },
            { "cast",       HandleCast,       SEC_PLAYER, Console::No },
            { "autocast",   HandleAutocast,   SEC_PLAYER, Console::No },
            { "status",     HandleStatus,     SEC_PLAYER, Console::No },
            { "sync",       HandleSync,       SEC_PLAYER, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "cspet", secondPetTable },
        };

        return commandTable;
    }

    // The invoking player's live second pet; answers when there is none
    static Creature* CommandPet(ChatHandler* handler, Player*& player, SecondPetState*& state)
    {
        player = handler->GetPlayer();
        state = player ? player->CustomData.Get<SecondPetState>(STATE_KEY) : nullptr;
        Creature* second = state ? FindSecondPet(player, *state) : nullptr;
        if (!second || !second->IsAlive() || !second->GetCharmInfo())
        {
            handler->SendSysMessage("You have no second pet.");
            return nullptr;
        }
        return second;
    }

    static bool HandleAttack(ChatHandler* handler)
    {
        Player* player = nullptr;
        SecondPetState* state = nullptr;
        Creature* second = CommandPet(handler, player, state);
        if (!second)
            return true;

        Unit* target = player->GetSelectedUnit();
        if (!target || target == second || !player->IsValidAttackTarget(target)
            || !second->CanCreatureAttack(target))
        {
            handler->SendSysMessage("Your second pet cannot attack that target.");
            return true;
        }

        player->GetSession()->HandlePetActionHelper(second, second->GetGUID(), COMMAND_ATTACK,
            ACT_COMMAND, target->GetGUID());
        return true;
    }

    static bool HandleOrder(ChatHandler* handler, CommandStates order)
    {
        Player* player = nullptr;
        SecondPetState* state = nullptr;
        if (Creature* second = CommandPet(handler, player, state))
            player->GetSession()->HandlePetActionHelper(second, second->GetGUID(), order,
                ACT_COMMAND, ObjectGuid::Empty);
        return true;
    }

    static bool HandleFollow(ChatHandler* handler)
    {
        return HandleOrder(handler, COMMAND_FOLLOW);
    }

    static bool HandleStay(ChatHandler* handler)
    {
        return HandleOrder(handler, COMMAND_STAY);
    }

    static bool HandleReact(ChatHandler* handler, ReactStates react)
    {
        Player* player = nullptr;
        SecondPetState* state = nullptr;
        Creature* second = CommandPet(handler, player, state);
        if (!second)
            return true;

        player->GetSession()->HandlePetActionHelper(second, second->GetGUID(), react,
            ACT_REACTION, ObjectGuid::Empty);
        state->React = react;
        return true;
    }

    static bool HandleAggressive(ChatHandler* handler)
    {
        return HandleReact(handler, REACT_AGGRESSIVE);
    }

    static bool HandleDefensive(ChatHandler* handler)
    {
        return HandleReact(handler, REACT_DEFENSIVE);
    }

    static bool HandlePassive(ChatHandler* handler)
    {
        return HandleReact(handler, REACT_PASSIVE);
    }

    static bool HandleCast(ChatHandler* handler, Tail args)
    {
        Player* player = nullptr;
        SecondPetState* state = nullptr;
        Creature* second = CommandPet(handler, player, state);
        if (!second)
            return true;

        SpellInfo const* spellInfo = FindSecondPetSpell(second, args, handler->GetSessionDbcLocale());
        if (!spellInfo)
        {
            handler->SendSysMessage("Your second pet does not have that spell.");
            return true;
        }

        CastSecondPetSpell(player, second, spellInfo);
        return true;
    }

    static bool HandleAutocast(ChatHandler* handler, Tail args)
    {
        Player* player = nullptr;
        SecondPetState* state = nullptr;
        Creature* second = CommandPet(handler, player, state);
        if (!second)
            return true;

        std::string_view const text = TrimView(args);
        std::size_t const space = text.find(' ');
        std::string_view const mode = text.substr(0, space);
        std::string_view const rest = space == std::string_view::npos ? std::string_view() : text.substr(space + 1);

        SpellInfo const* spellInfo = FindSecondPetSpell(second, rest, handler->GetSessionDbcLocale());
        if (!spellInfo || !spellInfo->IsAutocastable())
        {
            handler->SendSysMessage("Your second pet has no such spell to autocast.");
            return true;
        }

        bool on = false;
        if (StringEqualI(mode, "on"))
            on = true;
        else if (StringEqualI(mode, "off"))
            on = false;
        else if (StringEqualI(mode, "toggle"))
            on = !IsAutocastOn(second, spellInfo->Id);
        else
        {
            handler->SendSysMessage("Usage: .cspet autocast on|off|toggle <spell id or name>");
            return true;
        }

        SetAutocast(second, spellInfo, on);
        state->Autocast[spellInfo->Id] = on;
        return true;
    }

    static bool HandleStatus(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        SecondPetState* state = player ? player->CustomData.Get<SecondPetState>(STATE_KEY) : nullptr;
        Creature* second = state ? FindSecondPet(player, *state) : nullptr;
        CharmInfo* charmInfo = second ? second->GetCharmInfo() : nullptr;
        if (!charmInfo)
        {
            handler->SendSysMessage("You have no second pet.");
            return true;
        }

        Powers const power = second->getPowerType();
        Unit* victim = second->GetVictim();
        handler->PSendSysMessage("Second pet {}: entry {}, {}, health {}/{}, {} {}/{}, react {}, command {}, victim {}",
            second->GetName(), second->GetEntry(), second->IsAlive() ? "alive" : "dead",
            second->GetHealth(), second->GetMaxHealth(), PowerName(power), second->GetPower(power),
            second->GetMaxPower(power), ReactName(second->GetReactState()),
            charmInfo->HasCommandState(COMMAND_STAY) ? "stay" : "follow",
            victim ? victim->GetName() : std::string("none"));

        // its size against the pet's, and the pet's happiness on it: the
        // copy's weapon damage factor against the hunter pet's own, its
        // happiness included - 1 while the copy mirrors it (the pet's own
        // buffs, such as Bestial Wrath, are the pet's alone)
        AuraEffect const* happiness = second->GetAuraEffect(SPELL_CUSTOM_SECOND_PET_MARKER, EFFECT_1);
        std::string damage = "n/a";
        if (Pet* pet = CopiedHunterPet(player, second))
        {
            float const petFactor = pet->GetPctModifierValue(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT)
                * (100 + HappinessDamagePct(pet)) / 100.0f;
            if (petFactor > 0.0f)
                damage = Acore::StringFormat("{:.3f}",
                    second->GetPctModifierValue(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT) / petFactor);
        }
        handler->PSendSysMessage("Second pet look: scale {:.3f}, pet scale {:.3f}, size ratio {:.3f}; happiness {}%, weapon damage {} x the pet's",
            second->GetObjectScale(), state->Scale, SecondPetSizeRatio(second, *state),
            100 + (happiness ? happiness->GetAmount() : 0), damage);

        LocaleConstant const locale = handler->GetSessionDbcLocale();
        for (uint8 i = 0; i < MAX_SPELL_CHARM; ++i)
        {
            CharmSpellInfo* charmSpell = charmInfo->GetCharmSpell(i);
            uint32 const spellId = charmSpell->GetAction();
            SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
            if (!spellInfo || spellInfo->IsPassive())
                continue;

            char const* name = spellInfo->SpellName[locale];
            handler->PSendSysMessage("Spell {} {}: autocast {}, cooldown {} ms", spellId,
                name ? name : "", !spellInfo->IsAutocastable() ? "-"
                : charmSpell->GetType() == ACT_ENABLED ? "on" : "off",
                second->GetSpellCooldown(spellId));
        }
        return true;
    }

    static bool HandleSync(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        if (!player)
            return true;

        if (SecondPetState* state = player->CustomData.Get<SecondPetState>(STATE_KEY))
            PushBar(player, *state, true);
        else
            SendBar(player, "X");
        return true;
    }
};

void AddSecondPetSpellsScripts()
{
    new custom_second_pet_playerscript();
    new custom_second_pet_commandscript();
}
