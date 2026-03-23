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

#pragma once

#include "ScriptMgr.h"
#include "Player.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "Config.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "GridNotifiers.h"
#include "CellImpl.h"
#include "Group.h"
#include "GroupReference.h"
#include "ScriptedCreature.h"
#include "CreatureScript.h"
#include "PetDefines.h"
#include "PlayerScript.h"
#include "GameTime.h"
#include "UnitScript.h"

enum CustomSpellIds
{
    // Custom damage spell: Base 666 + 66% AP + 1% per Paragon level
    SPELL_CUSTOM_PARAGON_STRIKE         = 900106,
    // Each cast reduces Bladestorm (46927) cooldown by 0.5s
    SPELL_CUSTOM_BLADESTORM_CD_REDUCE   = 900107,
    // ---- Warrior Fury (900134-900167) ----
    // Bloody Whirlwind buff: +50% Whirlwind damage per stack, consumed on WW cast
    SPELL_BLOODY_WHIRLWIND_BUFF         = 900139,
    // Bloody Whirlwind passive: procs 900139 on Bloodthirst hit
    SPELL_BLOODY_WHIRLWIND_PASSIVE      = 900140,
    // Speedy Bloodthirst: Whirlwind resets Bloodthirst cooldown
    SPELL_CUSTOM_SPEEDY_BLOODTHIRST     = 900141,
    // WW single-target: passive auras (markers checked via HasAura)
    SPELL_WW_OVERPOWER_PASSIVE          = 900142,
    SPELL_WW_SLAM_PASSIVE               = 900143,
    // WW single-target: boosted damage spells (autocast)
    SPELL_WW_OVERPOWER_BOOSTED          = 900144,
    SPELL_WW_SLAM_BOOSTED               = 900145,

    // ---- Warrior Prot (900168-900199) ----
    SPELL_PROT_REVENGE_DMG_PASSIVE      = 900168, // Revenge +50% damage (DBC-only)
    SPELL_PROT_REVENGE_AOE_PASSIVE      = 900169, // Revenge unlimited targets (C++)
    SPELL_PROT_TC_REND_SUNDER_PASSIVE   = 900170, // TC → Rend + 5× Sunder (C++)
    SPELL_PROT_TC_DMG_PASSIVE           = 900171, // TC +50% damage (DBC-only)
    SPELL_PROT_BLOCK_AOE_PASSIVE        = 900172, // AoE damage on Block (C++)
    SPELL_PROT_BLOCK_TC_PASSIVE         = 900173, // 10% Block → Enhanced TC (C++)
    SPELL_PROT_BLOCK_AOE_DAMAGE         = 900174, // Helper: AoE damage (triggered)
    SPELL_PROT_ENHANCED_THUNDERCLAP     = 900175, // Helper: Enhanced TC (triggered)

    // ---- Paladin Holy (900200-900233) ----
    SPELL_HOLY_HS_AOE_DMG_PASSIVE       = 900200, // Holy Shock AoE damage (C++)
    SPELL_HOLY_HS_AOE_HEAL_PASSIVE      = 900201, // Holy Shock AoE heal (C++)
    SPELL_HOLY_HS_BOTH_PASSIVE          = 900202, // Holy Shock always both (C++)
    SPELL_HOLY_HS_DMG_PASSIVE           = 900203, // Holy Shock +50% (DBC-only)
    SPELL_HOLY_CONSEC_HEAL_PASSIVE      = 900204, // Consecration also heals (C++)
    SPELL_HOLY_CONSEC_AROUND_PASSIVE    = 900205, // Consecration around you (DBC)
    SPELL_HOLY_CONSEC_DMG_PASSIVE       = 900206, // Consecration +50% (DBC-only)
    SPELL_HOLY_CONSEC_DUR_PASSIVE       = 900207, // Consecration +5sec (DBC-only)
    // Helper spells
    SPELL_HOLY_HS_AOE_DMG_HELPER        = 900208, // AoE Holy damage around target
    SPELL_HOLY_HS_AOE_HEAL_HELPER       = 900209, // AoE Holy heal around target
    SPELL_HOLY_CONSEC_HEAL_HELPER       = 900210, // Consecration heal tick helper

    // ---- Paladin Prot (900234-900267) ----
    SPELL_PPROT_CONSEC_AROUND_PASSIVE   = 900234, // Consecration around you (DBC marker)
    SPELL_PPROT_AS_TARGETS_PASSIVE      = 900235, // Avenger's Shield +9 targets (DBC)
    SPELL_PPROT_AS_DMG_PASSIVE          = 900236, // Avenger's Shield +50% (DBC)
    SPELL_PPROT_HS_CHARGES_PASSIVE      = 900237, // Holy Shield charges +99 (DBC)
    SPELL_PPROT_HS_DMG_PASSIVE          = 900238, // Holy Shield +50% (DBC)
    SPELL_PPROT_AS_CONSEC_PASSIVE       = 900239, // AS leaves Consecration (C++)
    SPELL_PPROT_JUDGE_AS_PASSIVE        = 900240, // Judgement → free AS (C++)
    SPELL_PPROT_JUDGE_CD_PASSIVE        = 900241, // Judgement cd -2sec (DBC)

    // ---- Paladin Ret (900268-900299) ----
    SPELL_RET_CONSEC_AROUND_PASSIVE     = 900268, // Consecration around you (DBC marker)
    SPELL_RET_JUDGE_CD_PASSIVE          = 900269, // Judgement cd -2sec (DBC)
    SPELL_RET_DS_TARGETS_PASSIVE        = 900270, // Divine Storm +6 targets (DBC marker)
    SPELL_RET_DS_DMG_PASSIVE            = 900271, // Divine Storm +50% (DBC)
    SPELL_RET_CS_DMG_PASSIVE            = 900272, // Crusader Strike +50% (DBC)
    SPELL_RET_CS_AOE_PASSIVE            = 900273, // Crusader Strike +9 targets (C++)
    SPELL_RET_EXORCISM_PROC_PASSIVE     = 900274, // CS/Judge/DS → Exorcism buff (C++)
    SPELL_RET_EXORCISM_BUFF             = 900275, // Exorcism +50% per stack (DBC buff)

    // ---- DK Blood (900300-900332) ----
    SPELL_DKB_3_RUNE_WEAPONS_PASSIVE    = 900300, // 3 Rune Weapons (C++)
    SPELL_DKB_DOUBLE_CAST_PASSIVE       = 900301, // Rune Weapon double casts (C++)
    SPELL_DKB_HS_DMG_PASSIVE            = 900302, // Heart Strike +50% (DBC)
    SPELL_DKB_HS_AOE_PASSIVE            = 900303, // Heart Strike +9 targets (C++)
    SPELL_DKB_DEATHCOIL_PROC_PASSIVE    = 900304, // Dealing dmg → Death Coil (C++)

    // ---- DK Frost (900333-900365) ----
    SPELL_DKF_FROST_WYRM_PASSIVE        = 900333, // Replace Ghoul with Frost Wyrm (C++)

    // ---- DK Unholy (900366-900399) ----
    SPELL_DKU_DOT_AOE_PASSIVE           = 900366, // DoTs → Shadow AoE proc (C++)
    SPELL_DKU_SHADOW_AOE_HELPER         = 900367, // Shadow AoE damage helper

    // ---- Shaman Ele (900400-900432) ----
    SPELL_ELE_CL_AOE_PASSIVE            = 900400, // Chain Lightning +6 targets + no reduction (C++)
    SPELL_ELE_TOTEM_FOLLOW_PASSIVE      = 900401, // Totems → following creatures (C++)
    SPELL_ELE_RAGNAROS_PASSIVE          = 900402, // Fire Ele → Ragnaros (C++)
    SPELL_ELE_OVERLOAD_LVB_PASSIVE      = 900403, // Lightning Overload + Lava Burst (C++)
    SPELL_ELE_LVB_SPREAD_FS_PASSIVE     = 900404, // Lava Burst spreads Flame Shock (C++)
    SPELL_ELE_FS_RESET_LVB_PASSIVE      = 900405, // Flame Shock ticks → reset LvB CD (C++)
    SPELL_ELE_LVB_TWO_CHARGES_PASSIVE   = 900406, // Lava Burst two charges (C++)
    SPELL_ELE_CC_INSTANT_LVB_PASSIVE    = 900407, // Clearcasting → instant Lava Burst (DBC)
    SPELL_ELE_CL_AOE_HELPER             = 900408, // Chain Lightning AoE damage helper

    // ---- Shaman Enhance (900433-900465) ----
    SPELL_ENH_TOTEM_FOLLOW_PASSIVE      = 900433, // Totems follow (reuse PlayerScript)
    SPELL_ENH_MAELSTROM_AOE_PASSIVE     = 900434, // 5 Maelstrom → summons AoE 5s (C++)
    SPELL_ENH_MAELSTROM_AOE_BUFF        = 900439, // Buff: summons deal AoE on attack 5s
    SPELL_ENH_SUMMON_DMG_PASSIVE        = 900435, // Summons +50% damage (DBC)
    SPELL_ENH_WOLF_SUMMON_PASSIVE       = 900436, // Auto attack → summon wolf (C++)
    SPELL_ENH_WOLF_HASTE_PASSIVE        = 900437, // Spirit Wolves inherit haste (C++)
    SPELL_ENH_WOLF_CL_PASSIVE           = 900438, // Spirit Wolves 5% CL on hit (C++)
    SPELL_ENH_WOLF_AOE_HELPER           = 900440, // Wolf AoE damage helper

    // ---- Shaman Resto (900466-900499) ----
    SPELL_RST_TOTEM_FOLLOW_PASSIVE      = 900466, // Totems follow (reuse PlayerScript)
    SPELL_RST_MANA_REGEN_PASSIVE        = 900467, // Mana regen +2% per missing mana% (C++)

    // ---- Hunter Shared (900500-900501) ----
    SPELL_HUNT_ARROWS_PASSIVE           = 900500, // Get back arrows (no ammo consumption)
    SPELL_HUNT_MULTISHOT_AOE_PASSIVE    = 900501, // Multi-Shot unlimited targets (C++)

    // ---- Hunter BM (900502-900532) ----
    SPELL_HUNT_BM_PET_DMG_PASSIVE       = 900502, // Pet damage +50% (C++)
    SPELL_HUNT_BM_PET_SPEED_PASSIVE     = 900503, // Pet attack speed +50% (C++)
    SPELL_HUNT_BM_PET_AOE_PASSIVE       = 900504, // Pet AoE on hit (C++)
    SPELL_HUNT_BM_PET_AOE_HELPER        = 900505, // Helper: Pet AoE damage

    // ---- Hunter MM (900533-900565) ----
    SPELL_HUNT_MM_AUTOSHOT_BOUNCE       = 900533, // Auto Shot bounces +9 targets (C++)
    SPELL_HUNT_MM_BARRAGE_PASSIVE       = 900534, // Multi-Shot barrage channel (C++)
    SPELL_HUNT_MM_BOUNCE_HELPER         = 900535, // Helper: Auto Shot bounce damage
    SPELL_HUNT_MM_BARRAGE_SLOW          = 900536, // Helper: Barrage slow debuff

    // ---- Hunter Surv (900566-900599) ----
    SPELL_HUNT_SURV_TRAP_PROC_PASSIVE   = 900566, // Chance to drop explosion on dmg (C++)
    SPELL_HUNT_SURV_TRAP_HELPER         = 900567, // Helper: Explosive burst damage

    // ---- Rogue Assa (900600-900632) ----
    SPELL_ROG_ASSA_ENERGY_REGEN         = 900600, // Energy regen +50% (DBC)
    SPELL_ROG_ASSA_MUTI_DMG_PASSIVE     = 900601, // Mutilate +50% damage (DBC)
    SPELL_ROG_ASSA_POISON_DMG_PASSIVE   = 900602, // Poison damage +50% (DBC)
    SPELL_ROG_ASSA_POISON_NOVA_PASSIVE  = 900603, // Poison nova proc (C++)
    SPELL_ROG_ASSA_POISON_NOVA_HELPER   = 900604, // Helper: Poison Nova AoE

    // ---- Rogue Combat (900633-900665) ----
    SPELL_ROG_COMBAT_SS_DMG_PASSIVE     = 900633, // SS +50% damage (DBC)
    SPELL_ROG_COMBAT_SS_AOE_PASSIVE     = 900634, // SS +9 targets (C++)
    SPELL_ROG_COMBAT_BF_DUR_PASSIVE     = 900635, // Blade Flurry 2min (DBC)
    SPELL_ROG_COMBAT_BF_TARGETS_PASSIVE = 900636, // Blade Flurry +9 targets (DBC)
    SPELL_ROG_COMBAT_ENERGY_REGEN       = 900637, // Energy regen +50% (DBC)
    SPELL_ROG_COMBAT_SS_AOE_HELPER      = 900638, // Helper: SS bounce damage

    // ---- Rogue Sub (900666-900699) ----
    SPELL_ROG_SUB_ENERGY_REGEN          = 900666, // Energy regen +50% (DBC)
    SPELL_ROG_SUB_HEMO_DMG_PASSIVE      = 900667, // Hemorrhage +50% damage (DBC)
    SPELL_ROG_SUB_HEMO_AOE_PASSIVE      = 900668, // Hemorrhage +9 targets (C++)
    SPELL_ROG_SUB_HEMO_AOE_HELPER       = 900669, // Helper: Hemorrhage bounce damage

    // ---- Druid Balance (901000-901032) ----
    SPELL_BAL_MF_AOE_PASSIVE            = 901000, // Moonfire +9 targets (C++)
    SPELL_BAL_MF_DMG_PASSIVE            = 901001, // Moonfire +50% damage (DBC)
    SPELL_BAL_SF_TARGETS_PASSIVE        = 901002, // Starfall +9 targets (DBC)
    SPELL_BAL_SF_DMG_PASSIVE            = 901003, // Starfall +50% damage (DBC)
    SPELL_BAL_SF_CD_REDUCE_PASSIVE      = 901004, // Spell dmg reduces Starfall CD (C++)
    SPELL_BAL_SF_STACKS_PASSIVE         = 901005, // Starfall stacks up to 10 (DBC)

    // ---- Druid Feral Tank (901033-901048) ----
    SPELL_FERAL_BEAR_SWIPE_BLEED        = 901033, // Swipe Bear applies bleed (C++)
    SPELL_FERAL_BEAR_SWIPE_BLEED_DOT    = 901034, // Helper: Bear Swipe bleed DoT

    // ---- Druid Feral DPS (901049-901065) ----
    SPELL_FERAL_CAT_SWIPE_BLEED         = 901049, // Swipe Cat applies bleed (C++)
    SPELL_FERAL_CAT_SWIPE_BLEED_DOT     = 901050, // Helper: Cat Swipe bleed DoT
    SPELL_FERAL_CAT_ENERGY_REGEN        = 901051, // Energy regen +50% (DBC)

    // ---- Druid Resto (901066-901099) ----
    SPELL_DRST_HOT_TREANT_PASSIVE       = 901066, // HoTs chance to summon FoN (C++)
    SPELL_DRST_SUMMON_SCALE_PASSIVE     = 901067, // Summons scale with healing power (C++)
    SPELL_DRST_SUMMON_HEAL_PASSIVE      = 901068, // Summons heal on death/despawn (C++)
    SPELL_DRST_THORNS_REJUV_PASSIVE     = 901069, // Thorns chance to cast Rejuv (C++)
    SPELL_DRST_HOT_HEAL_PASSIVE         = 901070, // HoTs +50% healing (DBC)
    SPELL_DRST_HOT_HASTE_PASSIVE        = 901071, // HoTs tick 2x fast, 2x duration (C++)
    SPELL_DRST_MANA_REGEN_PASSIVE       = 901072, // Mana regen per missing mana% (C++)
    SPELL_DRST_TREANT_HEAL_HELPER       = 901073, // Helper: Treant heal on despawn
};

// ---- DK constants ----
constexpr uint32 SPELL_DANCING_RUNE_WEAPON  = 49028;
constexpr uint32 SPELL_HEART_STRIKE_R4      = 55050; // Highest rank
constexpr uint32 SPELL_DK_DEATH_COIL_DMG    = 47632; // Death Coil damage effect
constexpr uint32 SPELL_DK_RAISE_DEAD        = 46584;
constexpr uint32 NPC_DK_RUNE_WEAPON         = 27893;
constexpr uint32 NPC_DK_GHOUL               = 26125;
constexpr uint32 NPC_DK_FROST_WYRM          = 900333; // Custom Frost Wyrm creature
constexpr uint32 SPELL_FROST_BREATH         = 900368; // Frost Wyrm cone breath

// ---- Shaman constants ----
constexpr uint32 SPELL_CHAIN_LIGHTNING_R4   = 49271;  // Highest rank CL
constexpr uint32 SPELL_LAVA_BURST_R2        = 51505;  // Highest rank LvB
constexpr uint32 SPELL_FLAME_SHOCK_R9       = 49233;  // Highest rank Flame Shock
constexpr uint32 SPELL_ELEMENTAL_FOCUS      = 16164;  // Clearcasting buff
constexpr uint32 SPELL_OVERLOAD_LB          = 45284;  // Lightning Overload (LB)
constexpr uint32 SPELL_OVERLOAD_CL          = 45297;  // Lightning Overload (CL)
constexpr uint32 SPELL_FIRE_ELEMENTAL_TOTEM = 2894;
// NPC_FIRE_ELEMENTAL = 15438 already defined in PetDefines.h
constexpr uint32 NPC_RAGNAROS_CUSTOM        = 900402; // Custom Ragnaros NPC
constexpr uint32 SPELL_MAELSTROM_WEAPON     = 53817;  // Maelstrom Weapon buff
constexpr uint32 SPELL_FERAL_SPIRIT         = 51533;  // Summon Spirit Wolves
constexpr uint32 NPC_SPIRIT_WOLF            = 29264;  // Spirit Wolf NPC
constexpr uint32 NPC_CUSTOM_WOLF            = 900436; // Custom summoned wolf
constexpr uint32 SPELL_CL_R6               = 49271;  // Chain Lightning rank 6
// Flame Shock SpellFamilyFlags[0] = 0x10000000, SpellFamilyName = 11

// ---- Rogue constants ----
constexpr uint32 SPELL_MUTILATE_R5         = 48666;  // Mutilate highest rank
constexpr uint32 SPELL_SINISTER_STRIKE_R12 = 48638;  // Sinister Strike highest rank
constexpr uint32 SPELL_HEMORRHAGE_R4       = 48660;  // Hemorrhage highest rank
constexpr uint32 SPELL_BLADE_FLURRY        = 13877;  // Blade Flurry
constexpr uint32 SPELLFAMILY_ROGUE_ID      = 8;
// Mutilate SpellFamilyFlags[1] = 0x200000 (verify!)
// Sinister Strike SpellFamilyFlags[0] = 0x2 (verify!)
// Hemorrhage SpellFamilyFlags[0] = 0x2000000 (verify!)
// Blade Flurry SpellFamilyFlags[1] = 0x800 (verify!)

// ---- Druid constants ----
constexpr uint32 SPELL_MOONFIRE_R14        = 48463;  // Moonfire highest rank
constexpr uint32 SPELL_STARFALL_R2         = 53201;  // Starfall highest rank
constexpr uint32 SPELL_SWIPE_BEAR_R6       = 48562;  // Swipe (Bear) highest rank
constexpr uint32 SPELL_SWIPE_CAT           = 62078;  // Swipe (Cat)
constexpr uint32 SPELL_THORNS_R8           = 53307;  // Thorns highest rank
constexpr uint32 SPELL_REJUV_R15           = 48441;  // Rejuvenation highest rank
constexpr uint32 SPELL_FORCE_OF_NATURE     = 33831;  // Force of Nature (summon treants)
constexpr uint32 NPC_TREANT                = 1964;   // Treant NPC entry
constexpr uint32 NPC_CUSTOM_TREANT         = 901066; // Custom Treant NPC entry
constexpr uint32 SPELLFAMILY_DRUID_ID      = 7;
// Moonfire SpellFamilyFlags[0] = 0x2 (verify!)
// Starfall SpellFamilyFlags[0] = 0x100 (verify!)
// Rejuvenation SpellFamilyFlags[0] = 0x10 (verify!)

// ---- Hunter constants ----
constexpr uint32 SPELL_AUTO_SHOT            = 75;     // Auto Shot
constexpr uint32 SPELL_MULTISHOT_R6         = 49048;  // Multi-Shot highest rank
constexpr uint32 SPELL_EXPLOSIVE_TRAP_R4    = 49067;  // Explosive Trap highest rank
constexpr uint32 SPELL_EXPLOSIVE_TRAP_DMG   = 49064;  // Explosive Trap damage effect
constexpr uint32 SPELLFAMILY_HUNTER_ID      = 9;
// Multi-Shot SpellFamilyFlags[0] = 0x200 (verify!)

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

// ---- Warrior Prot constants ----
constexpr uint32 SPELL_REND_R10       = 47465;
constexpr uint32 SPELL_SUNDER_ARMOR   = 58567;

// ---- Paladin Holy constants ----
// Holy Shock base spell (dummy → routes to dmg/heal)
constexpr uint32 SPELL_HOLY_SHOCK           = 20473;
// Holy Shock damage spell (highest rank: 48824, R1: 25912)
constexpr uint32 SPELL_HOLY_SHOCK_DMG_R7    = 48824;
// Holy Shock heal spell (highest rank: 48825, R1: 25914)
constexpr uint32 SPELL_HOLY_SHOCK_HEAL_R7   = 48825;
// Consecration highest rank
constexpr uint32 SPELL_CONSECRATION_R8      = 48819;
// Avenger's Shield highest rank
constexpr uint32 SPELL_AVENGERS_SHIELD_R3   = 48827;
// Judgement damage spell (always triggered by all Judgement types)
constexpr uint32 SPELL_JUDGEMENT_DAMAGE     = 54158;
// Divine Storm
constexpr uint32 SPELL_DIVINE_STORM        = 53385;
// Crusader Strike (highest rank)
constexpr uint32 SPELL_CRUSADER_STRIKE_R6   = 35395;
// Exorcism (highest rank)
constexpr uint32 SPELL_EXORCISM_R9          = 48801;

// Per-class registration functions
void AddWarriorSpellsScripts();
void AddPaladinSpellsScripts();
void AddDKSpellsScripts();
void AddShamanSpellsScripts();
void AddHunterSpellsScripts();
void AddRogueSpellsScripts();
void AddDruidSpellsScripts();
