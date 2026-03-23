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
    SPELL_BLOODY_WHIRLWIND_BUFF         = 900139,
    SPELL_BLOODY_WHIRLWIND_PASSIVE      = 900140,
    SPELL_CUSTOM_SPEEDY_BLOODTHIRST     = 900141,
    SPELL_WW_OVERPOWER_PASSIVE          = 900142,
    SPELL_WW_SLAM_PASSIVE               = 900143,
    SPELL_WW_OVERPOWER_BOOSTED          = 900144,
    SPELL_WW_SLAM_BOOSTED               = 900145,

    // ---- Warrior Prot (900168-900199) ----
    SPELL_PROT_REVENGE_DMG_PASSIVE      = 900168,
    SPELL_PROT_REVENGE_AOE_PASSIVE      = 900169,
    SPELL_PROT_TC_REND_SUNDER_PASSIVE   = 900170,
    SPELL_PROT_TC_DMG_PASSIVE           = 900171,
    SPELL_PROT_BLOCK_AOE_PASSIVE        = 900172,
    SPELL_PROT_BLOCK_TC_PASSIVE         = 900173,
    SPELL_PROT_BLOCK_AOE_DAMAGE         = 900174,
    SPELL_PROT_ENHANCED_THUNDERCLAP     = 900175,

    // ---- Paladin Holy (900200-900233) ----
    SPELL_HOLY_HS_AOE_DMG_PASSIVE       = 900200,
    SPELL_HOLY_HS_AOE_HEAL_PASSIVE      = 900201,
    SPELL_HOLY_HS_BOTH_PASSIVE          = 900202,
    SPELL_HOLY_HS_DMG_PASSIVE           = 900203,
    SPELL_HOLY_CONSEC_HEAL_PASSIVE      = 900204,
    SPELL_HOLY_CONSEC_AROUND_PASSIVE    = 900205,
    SPELL_HOLY_CONSEC_DMG_PASSIVE       = 900206,
    SPELL_HOLY_CONSEC_DUR_PASSIVE       = 900207,
    SPELL_HOLY_HS_AOE_DMG_HELPER        = 900208,
    SPELL_HOLY_HS_AOE_HEAL_HELPER       = 900209,
    SPELL_HOLY_CONSEC_HEAL_HELPER       = 900210,

    // ---- Paladin Prot (900234-900267) ----
    SPELL_PPROT_CONSEC_AROUND_PASSIVE   = 900234,
    SPELL_PPROT_AS_TARGETS_PASSIVE      = 900235,
    SPELL_PPROT_AS_DMG_PASSIVE          = 900236,
    SPELL_PPROT_HS_CHARGES_PASSIVE      = 900237,
    SPELL_PPROT_HS_DMG_PASSIVE          = 900238,
    SPELL_PPROT_AS_CONSEC_PASSIVE       = 900239,
    SPELL_PPROT_JUDGE_AS_PASSIVE        = 900240,
    SPELL_PPROT_JUDGE_CD_PASSIVE        = 900241,

    // ---- Paladin Ret (900268-900299) ----
    SPELL_RET_CONSEC_AROUND_PASSIVE     = 900268,
    SPELL_RET_JUDGE_CD_PASSIVE          = 900269,
    SPELL_RET_DS_TARGETS_PASSIVE        = 900270,
    SPELL_RET_DS_DMG_PASSIVE            = 900271,
    SPELL_RET_CS_DMG_PASSIVE            = 900272,
    SPELL_RET_CS_AOE_PASSIVE            = 900273,
    SPELL_RET_EXORCISM_PROC_PASSIVE     = 900274,
    SPELL_RET_EXORCISM_BUFF             = 900275,

    // ---- DK Blood (900300-900332) ----
    SPELL_DKB_3_RUNE_WEAPONS_PASSIVE    = 900300,
    SPELL_DKB_DOUBLE_CAST_PASSIVE       = 900301,
    SPELL_DKB_HS_DMG_PASSIVE            = 900302,
    SPELL_DKB_HS_AOE_PASSIVE            = 900303,
    SPELL_DKB_DEATHCOIL_PROC_PASSIVE    = 900304,

    // ---- DK Frost (900333-900365) ----
    SPELL_DKF_FROST_WYRM_PASSIVE        = 900333,

    // ---- DK Unholy (900366-900399) ----
    SPELL_DKU_DOT_AOE_PASSIVE           = 900366,
    SPELL_DKU_SHADOW_AOE_HELPER         = 900367,

    // ---- Shaman Ele (900400-900432) ----
    SPELL_ELE_CL_AOE_PASSIVE            = 900400,
    SPELL_ELE_TOTEM_FOLLOW_PASSIVE      = 900401,
    SPELL_ELE_RAGNAROS_PASSIVE          = 900402,
    SPELL_ELE_OVERLOAD_LVB_PASSIVE      = 900403,
    SPELL_ELE_LVB_SPREAD_FS_PASSIVE     = 900404,
    SPELL_ELE_FS_RESET_LVB_PASSIVE      = 900405,
    SPELL_ELE_LVB_TWO_CHARGES_PASSIVE   = 900406,
    SPELL_ELE_CC_INSTANT_LVB_PASSIVE    = 900407,
    SPELL_ELE_CL_AOE_HELPER             = 900408,

    // ---- Shaman Enhance (900433-900465) ----
    SPELL_ENH_TOTEM_FOLLOW_PASSIVE      = 900433,
    SPELL_ENH_MAELSTROM_AOE_PASSIVE     = 900434,
    SPELL_ENH_MAELSTROM_AOE_BUFF        = 900439,
    SPELL_ENH_SUMMON_DMG_PASSIVE        = 900435,
    SPELL_ENH_WOLF_SUMMON_PASSIVE       = 900436,
    SPELL_ENH_WOLF_HASTE_PASSIVE        = 900437,
    SPELL_ENH_WOLF_CL_PASSIVE           = 900438,
    SPELL_ENH_WOLF_AOE_HELPER           = 900440,

    // ---- Shaman Resto (900466-900499) ----
    SPELL_RST_TOTEM_FOLLOW_PASSIVE      = 900466,
    SPELL_RST_MANA_REGEN_PASSIVE        = 900467,

    // ---- Hunter Shared (900500-900501) ----
    SPELL_HUNT_ARROWS_PASSIVE           = 900500,
    SPELL_HUNT_MULTISHOT_AOE_PASSIVE    = 900501,

    // ---- Hunter BM (900502-900532) ----
    SPELL_HUNT_BM_PET_DMG_PASSIVE       = 900502,
    SPELL_HUNT_BM_PET_SPEED_PASSIVE     = 900503,
    SPELL_HUNT_BM_PET_AOE_PASSIVE       = 900504,
    SPELL_HUNT_BM_PET_AOE_HELPER        = 900505,

    // ---- Hunter MM (900533-900565) ----
    SPELL_HUNT_MM_AUTOSHOT_BOUNCE       = 900533,
    SPELL_HUNT_MM_BARRAGE_PASSIVE       = 900534,
    SPELL_HUNT_MM_BOUNCE_HELPER         = 900535,
    SPELL_HUNT_MM_BARRAGE_SLOW          = 900536,

    // ---- Hunter Surv (900566-900599) ----
    SPELL_HUNT_SURV_TRAP_PROC_PASSIVE   = 900566,
    SPELL_HUNT_SURV_TRAP_HELPER         = 900567,

    // ---- Rogue Assa (900600-900632) ----
    SPELL_ROG_ASSA_ENERGY_REGEN         = 900600,
    SPELL_ROG_ASSA_MUTI_DMG_PASSIVE     = 900601,
    SPELL_ROG_ASSA_POISON_DMG_PASSIVE   = 900602,
    SPELL_ROG_ASSA_POISON_NOVA_PASSIVE  = 900603,
    SPELL_ROG_ASSA_POISON_NOVA_HELPER   = 900604,

    // ---- Rogue Combat (900633-900665) ----
    SPELL_ROG_COMBAT_SS_DMG_PASSIVE     = 900633,
    SPELL_ROG_COMBAT_SS_AOE_PASSIVE     = 900634,
    SPELL_ROG_COMBAT_BF_DUR_PASSIVE     = 900635,
    SPELL_ROG_COMBAT_BF_TARGETS_PASSIVE = 900636,
    SPELL_ROG_COMBAT_ENERGY_REGEN       = 900637,
    SPELL_ROG_COMBAT_SS_AOE_HELPER      = 900638,

    // ---- Rogue Sub (900666-900699) ----
    SPELL_ROG_SUB_ENERGY_REGEN          = 900666,
    SPELL_ROG_SUB_HEMO_DMG_PASSIVE      = 900667,
    SPELL_ROG_SUB_HEMO_AOE_PASSIVE      = 900668,
    SPELL_ROG_SUB_HEMO_AOE_HELPER       = 900669,

    // ---- Mage Arcane (900700-900732) ----
    SPELL_MAGE_ARC_MANA_REGEN          = 900700,
    SPELL_MAGE_ARC_ABARR_DMG_PASSIVE   = 900701,
    SPELL_MAGE_ARC_ABARR_AOE_PASSIVE   = 900702,
    SPELL_MAGE_ARC_AB_CAST_PASSIVE     = 900703,
    SPELL_MAGE_ARC_AB_AOE_PASSIVE      = 900704,
    SPELL_MAGE_ARC_CHARGES_8_PASSIVE   = 900705,
    SPELL_MAGE_ARC_AE_CHARGES_PASSIVE  = 900706,
    SPELL_MAGE_ARC_EVOC_POWER_PASSIVE  = 900707,
    SPELL_MAGE_ARC_EMERGENCY_PASSIVE   = 900708,
    SPELL_MAGE_ARC_BLINK_PASSIVE       = 900709,
    SPELL_MAGE_ARC_ABARR_AOE_HELPER    = 900710,
    SPELL_MAGE_ARC_AB_AOE_HELPER       = 900711,
    SPELL_MAGE_ARC_EVOC_BUFF           = 900712,
    SPELL_MAGE_ARC_TARGETED_BLINK      = 900713,

    // ---- Mage Fire (900733-900765) ----
    SPELL_MAGE_FIRE_FB_DMG_PASSIVE     = 900733,
    SPELL_MAGE_FIRE_FB_AOE_PASSIVE     = 900734,
    SPELL_MAGE_FIRE_PYRO_AOE_PASSIVE   = 900735,
    SPELL_MAGE_FIRE_PYRO_DMG_PASSIVE   = 900736,
    SPELL_MAGE_FIRE_FBLAST_GCD_PASSIVE = 900737,
    SPELL_MAGE_FIRE_PYRO_HS_PASSIVE    = 900738,
    SPELL_MAGE_FIRE_FB_AOE_HELPER      = 900739,
    SPELL_MAGE_FIRE_PYRO_AOE_HELPER    = 900740,

    // ---- Mage Frost (900766-900799) ----
    SPELL_MAGE_FROST_FB_DMG_PASSIVE    = 900766,
    SPELL_MAGE_FROST_FB_AOE_PASSIVE    = 900767,
    SPELL_MAGE_FROST_IL_DMG_PASSIVE    = 900768,
    SPELL_MAGE_FROST_IL_AOE_PASSIVE    = 900769,
    SPELL_MAGE_FROST_PERM_ELE_PASSIVE  = 900770,
    SPELL_MAGE_FROST_COMET_SHOWER      = 900771,
    SPELL_MAGE_FROST_FB_AOE_HELPER     = 900772,
    SPELL_MAGE_FROST_IL_AOE_HELPER     = 900773,
    SPELL_MAGE_FROST_COMET_HELPER      = 900774,

    // ---- Warlock Affliction (900800-900832) ----
    SPELL_WLK_AFFL_DOT_AOE_PASSIVE     = 900800,
    SPELL_WLK_AFFL_CORR_DMG_PASSIVE    = 900801,
    SPELL_WLK_AFFL_DOT_SPREAD_PASSIVE  = 900802,
    SPELL_WLK_AFFL_DOT_AOE_HELPER      = 900803,

    // ---- Warlock Demonology (900833-900865) ----
    SPELL_WLK_DEMO_META_KILL_EXT       = 900833,
    SPELL_WLK_DEMO_META_AOE_HEAL       = 900834,
    SPELL_WLK_DEMO_LESSER_SPAWN        = 900835,
    SPELL_WLK_DEMO_IMP_FB_DMG          = 900836,
    SPELL_WLK_DEMO_IMP_FB_AOE          = 900837,
    SPELL_WLK_DEMO_FG_UNLIM_TARGETS    = 900838,
    SPELL_WLK_DEMO_FG_DMG              = 900839,
    SPELL_WLK_DEMO_SACRIFICE_ALL       = 900840,
    SPELL_WLK_DEMO_IMP_FB_HELPER       = 900841,
    SPELL_WLK_DEMO_META_AOE_HELPER     = 900842,
    SPELL_WLK_DEMO_META_HEAL_HELPER    = 900843,

    // ---- Warlock Destruction (900866-900899) ----
    SPELL_WLK_DEST_SB_AOE              = 900866,
    SPELL_WLK_DEST_SB_DMG              = 900867,
    SPELL_WLK_DEST_CB_DMG              = 900868,
    SPELL_WLK_DEST_CB_CD               = 900869,
    SPELL_WLK_DEST_CB_AOE              = 900870,
    SPELL_WLK_DEST_SB_HELPER           = 900871,
    SPELL_WLK_DEST_CB_HELPER           = 900872,

    // ---- Priest Discipline (900900-900932) ----
    SPELL_PRI_DISC_SHIELD_EXPLODE      = 900900,
    SPELL_PRI_DISC_SHIELD_DMG          = 900901,
    SPELL_PRI_DISC_WEAKENED_SOUL_CD    = 900902,
    SPELL_PRI_DISC_EXPLODE_HELPER      = 900903,

    // ---- Priest Holy (900933-900965) ----
    SPELL_PRI_HOLY_HEAL_FIRE           = 900933,

    // ---- Priest Shadow (900966-900999) ----
    SPELL_PRI_SHADOW_DOT_AOE           = 900966,
    SPELL_PRI_SHADOW_DOT_SPREAD        = 900967,
    SPELL_PRI_SHADOW_AOE_HELPER        = 900968,

    // ---- Druid Balance (901000-901032) ----
    SPELL_BAL_MF_AOE_PASSIVE            = 901000,
    SPELL_BAL_MF_DMG_PASSIVE            = 901001,
    SPELL_BAL_SF_TARGETS_PASSIVE        = 901002,
    SPELL_BAL_SF_DMG_PASSIVE            = 901003,
    SPELL_BAL_SF_CD_REDUCE_PASSIVE      = 901004,
    SPELL_BAL_SF_STACKS_PASSIVE         = 901005,

    // ---- Druid Feral Tank (901033-901048) ----
    SPELL_FERAL_BEAR_SWIPE_BLEED        = 901033,
    SPELL_FERAL_BEAR_SWIPE_BLEED_DOT    = 901034,

    // ---- Druid Feral DPS (901049-901065) ----
    SPELL_FERAL_CAT_SWIPE_BLEED         = 901049,
    SPELL_FERAL_CAT_SWIPE_BLEED_DOT     = 901050,
    SPELL_FERAL_CAT_ENERGY_REGEN        = 901051,

    // ---- Druid Resto (901066-901099) ----
    SPELL_DRST_HOT_TREANT_PASSIVE       = 901066,
    SPELL_DRST_SUMMON_SCALE_PASSIVE     = 901067,
    SPELL_DRST_SUMMON_HEAL_PASSIVE      = 901068,
    SPELL_DRST_THORNS_REJUV_PASSIVE     = 901069,
    SPELL_DRST_HOT_HEAL_PASSIVE         = 901070,
    SPELL_DRST_HOT_HASTE_PASSIVE        = 901071,
    SPELL_DRST_MANA_REGEN_PASSIVE       = 901072,
    SPELL_DRST_TREANT_HEAL_HELPER       = 901073,
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

// ---- Mage constants ----
constexpr uint32 SPELL_ARCANE_BLAST_DEBUFF     = 36032;  // Arcane Blast stacking debuff
constexpr uint32 SPELL_ARCANE_BARRAGE_R3       = 44781;  // Arcane Barrage highest rank
constexpr uint32 SPELL_ARCANE_BLAST_R4         = 42897;  // Arcane Blast highest rank
constexpr uint32 SPELL_ARCANE_EXPLOSION_R9     = 42921;  // Arcane Explosion highest rank
constexpr uint32 SPELL_EVOCATION               = 12051;  // Evocation
constexpr uint32 SPELL_MANA_SHIELD_R9          = 43020;  // Mana Shield highest rank
constexpr uint32 SPELL_BLINK                   = 1953;   // Blink
constexpr uint32 SPELLFAMILY_MAGE_ID           = 3;
// Arcane Barrage SpellFamilyFlags[1] = 0x1000000 (verify!)
// Arcane Blast SpellFamilyFlags[0] = 0x20000000 (verify!)
constexpr uint32 SPELL_FIREBALL_R16        = 42833;  // Fireball highest rank
constexpr uint32 SPELL_PYROBLAST_R12       = 42891;  // Pyroblast highest rank
constexpr uint32 SPELL_FIRE_BLAST_R11      = 42873;  // Fire Blast highest rank
constexpr uint32 SPELL_HOT_STREAK          = 48108;  // Hot Streak buff
// Fireball SpellFamilyFlags[0] = 0x1 (verify!)
// Pyroblast SpellFamilyFlags[0] = 0x400000 (verify!)
constexpr uint32 SPELL_FROSTBOLT_R16       = 42842;  // Frostbolt highest rank
constexpr uint32 SPELL_ICE_LANCE_R3        = 42914;  // Ice Lance highest rank
constexpr uint32 SPELL_SUMMON_WATER_ELE    = 31687;  // Summon Water Elemental
constexpr uint32 NPC_WATER_ELEMENTAL       = 510;    // Water Elemental NPC
// Frostbolt SpellFamilyFlags[0] = 0x20 (verify!)
// Ice Lance SpellFamilyFlags[0] = 0x20000 (verify!)

// ---- Warlock constants ----
constexpr uint32 SPELL_CORRUPTION_R10      = 47813;  // Corruption highest rank
constexpr uint32 SPELLFAMILY_WARLOCK_ID    = 5;
constexpr uint32 SPELL_METAMORPHOSIS       = 47241;  // Metamorphosis demon form
constexpr uint32 SPELL_IMP_FIREBOLT_R9     = 47964;  // Imp Firebolt highest rank
constexpr uint32 SPELL_FELGUARD_CLEAVE     = 47994;  // Felguard Cleave
constexpr uint32 SPELL_DEMONIC_SACRIFICE   = 18788;  // Demonic Sacrifice
// Demonic Sacrifice buffs per pet type
constexpr uint32 SPELL_SACRIFICE_IMP       = 18789;  // Imp: +15% Fire damage
constexpr uint32 SPELL_SACRIFICE_VW        = 18790;  // Voidwalker: HP regen
constexpr uint32 SPELL_SACRIFICE_SUCCUBUS  = 18791;  // Succubus: +15% Shadow damage
constexpr uint32 SPELL_SACRIFICE_FELHUNTER = 18792;  // Felhunter: Mana regen
// NPC entries for Lesser Demons
constexpr uint32 NPC_LESSER_IMP            = 900835; // Lesser Imp
constexpr uint32 NPC_LESSER_FELGUARD       = 900836; // Lesser Felguard
constexpr uint32 NPC_LESSER_VOIDWALKER     = 900837; // Lesser Voidwalker
// Corruption SpellFamilyFlags[0] = 0x2 (verify!)
constexpr uint32 SPELL_SHADOW_BOLT_R13     = 47809;  // Shadow Bolt highest rank
constexpr uint32 SPELL_CHAOS_BOLT_R4       = 59172;  // Chaos Bolt highest rank
// Shadow Bolt SpellFamilyFlags[0] = 0x1 (verify!)
// Chaos Bolt SpellFamilyFlags[1] = 0x1000000 (verify!)
// Warlock DoTs: Corruption, Curse of Agony, Immolate, Unstable Affliction, Seed of Corruption

// ---- Priest constants ----
constexpr uint32 SPELL_PW_SHIELD_R14       = 48066;  // Power Word: Shield highest rank
constexpr uint32 SPELL_WEAKENED_SOUL       = 6788;   // Weakened Soul debuff
constexpr uint32 SPELL_HOLY_FIRE_R11       = 48135;  // Holy Fire highest rank
constexpr uint32 SPELLFAMILY_PRIEST_ID     = 6;
// PW:Shield SpellFamilyFlags[0] = 0x1 (verify!)
// Holy Fire SpellFamilyFlags[0] = 0x8000 (verify!)

// Per-class registration functions
void AddWarriorSpellsScripts();
void AddPaladinSpellsScripts();
void AddDKSpellsScripts();
void AddShamanSpellsScripts();
void AddHunterSpellsScripts();
void AddRogueSpellsScripts();
void AddDruidSpellsScripts();
void AddMageSpellsScripts();
void AddWarlockSpellsScripts();
void AddPriestSpellsScripts();
