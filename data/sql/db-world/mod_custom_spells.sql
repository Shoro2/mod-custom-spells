-- Link custom spell IDs to their SpellScript names
DELETE FROM `spell_script_names` WHERE `spell_id` IN (900106, 900107, 900140, 900141, 900144, 900145, 1680, 57823, 47502, 900172, 900173, -25912, -25914, 48819, -48827, 54158, -35395, 900274, 48801);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(900106, 'spell_custom_paragon_strike'),
(900107, 'spell_custom_bladestorm_cd_reduce'),
(900140, 'spell_custom_bloody_whirlwind_passive'),
(900141, 'spell_custom_speedy_bloodthirst'),
(900144, 'spell_custom_ww_overpower'),
(900145, 'spell_custom_ww_slam'),
(1680, 'spell_custom_bloody_whirlwind_consume'),
-- Warrior Prot
(57823, 'spell_custom_prot_revenge_aoe'),
(47502, 'spell_custom_prot_tc_rend_sunder'),
(900172, 'spell_custom_prot_block_aoe'),
(900173, 'spell_custom_prot_block_tc'),
-- Paladin Holy: Holy Shock damage (all ranks via negative ID)
(-25912, 'spell_custom_holy_hs_aoe_dmg'),
(-25912, 'spell_custom_holy_hs_both_dmg'),
-- Paladin Holy: Holy Shock heal (all ranks via negative ID)
(-25914, 'spell_custom_holy_hs_aoe_heal'),
(-25914, 'spell_custom_holy_hs_both_heal'),
-- Paladin Holy: Consecration heal hook (max rank)
(48819, 'spell_custom_holy_consec_heal'),
-- Paladin Prot: AS leaves Consecration (all ranks via negative ID)
(-48827, 'spell_custom_pprot_as_consec'),
-- Paladin Prot: Judgement → free AS (on Judgement damage spell)
(54158, 'spell_custom_pprot_judge_as'),
-- Paladin Ret: CS +9 targets (all ranks via negative ID)
(-35395, 'spell_custom_ret_cs_aoe'),
-- Paladin Ret: Exorcism buff proc passive
(900274, 'spell_custom_ret_exorcism_proc'),
-- Paladin Ret: Exorcism consume stacks (on Exorcism max rank)
(48801, 'spell_custom_ret_exorcism_consume');

-- ICD for Whirlwind proc aura (900138): 500ms cooldown to prevent
-- recursive loop (Whirlwind hits count as auto-attacks → re-proc → crash)
DELETE FROM `spell_proc` WHERE `SpellId` = 900138;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900138, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 500, 0);

-- Bladestorm CD Reduce passive (900107): procs on melee damage dealt.
-- ProcFlags=0x14 (DONE_MELEE_AUTO_ATTACK | DONE_SPELL_MELEE_DMG_CLASS).
-- C++ CheckProc filters: only when Bladestorm is on cooldown.
DELETE FROM `spell_proc` WHERE `SpellId` = 900107;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900107, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Bloody Whirlwind passive (900140): C++ AuraScript handles Bloodthirst filtering.
-- ProcFlags=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS) matches Bloodthirst's melee dmg class.
DELETE FROM `spell_proc` WHERE `SpellId` = 900140;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900140, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Speedy Bloodthirst (900141): Whirlwind resets Bloodthirst CD.
-- ProcFlags=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS), SpellPhaseMask=4 (FINISH).
DELETE FROM `spell_proc` WHERE `SpellId` = 900141;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900141, 0, 0, 0, 0, 0, 0x10, 1, 4, 0, 0, 0, 0, 100, 0, 0);

-- ============================================================
-- Warrior Prot: spell_proc entries
-- ============================================================

-- Block → AoE damage (900172): procs on TAKEN_MELEE_AUTO_ATTACK (0x2).
-- C++ HandleProc checks HitMask for BLOCK. 100% chance, 1s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900172;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900172, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 100, 1000, 0);

-- Block → Enhanced TC (900173): procs on TAKEN_MELEE_AUTO_ATTACK (0x2).
-- C++ HandleProc checks HitMask for BLOCK. 10% chance, 3s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900173;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900173, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 10, 3000, 0);

-- ============================================================
-- Warrior Prot: spell_dbc entries (900168-900175)
-- ============================================================

DELETE FROM `spell_dbc` WHERE `ID` IN (900168, 900169, 900170, 900171, 900172, 900173, 900174, 900175);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellFamilyName`, `SpellIconID`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900168: Revenge +50% damage passive (SPELL_AURA_ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA_1=0x400 targets Revenge SpellFamilyFlags[0]
(900168, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x400, 4, 132, 'Prot: Revenge Damage', 0x003F3F),
-- 900169: Revenge AoE passive (marker aura, logic in C++)
(900169, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Revenge AoE', 0x003F3F),
-- 900170: TC → Rend + Sunder passive (marker aura, logic in C++)
(900170, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: TC Rend Sunder', 0x003F3F),
-- 900171: Thunderclap +50% damage passive (SPELL_AURA_ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA_1=0x80 targets Thunderclap SpellFamilyFlags[0]
(900171, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x80, 4, 132, 'Prot: TC Damage', 0x003F3F),
-- 900172: Block → AoE damage proc passive (SPELL_AURA_DUMMY)
(900172, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block AoE', 0x003F3F),
-- 900173: Block → Enhanced TC proc passive (SPELL_AURA_DUMMY)
(900173, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block TC', 0x003F3F),
-- 900174: Helper - Block AoE damage (instant AoE physical, 8yd radius)
(900174, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 100, 500, 22, 0, 0, 0, 0, 4, 132, 'Block Shield Burst', 0x003F3F),
-- 900175: Helper - Enhanced Thunderclap (instant AoE physical, 10yd radius)
(900175, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 1000, 22, 0, 0, 0, 0, 4, 132, 'Enhanced Thunderclap', 0x003F3F);

-- ============================================================
-- Paladin Holy: spell_dbc entries (900200-900210)
-- ============================================================
-- SpellFamilyName=10 (Paladin)
-- Holy Shock SpellFamilyFlags[0]: 0x200000
-- Consecration SpellFamilyFlags[0]: 0x20 (verify in-game!)

DELETE FROM `spell_dbc` WHERE `ID` IN (900200, 900201, 900202, 900203, 900204, 900205, 900206, 900207, 900208, 900209, 900210);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellFamilyName`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900200: Holy Shock AoE damage passive (marker, C++ on -25912)
(900200, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Damage', 0x003F3F),
-- 900201: Holy Shock AoE heal passive (marker, C++ on -25914)
(900201, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Heal', 0x003F3F),
-- 900202: Holy Shock always both passive (marker, C++ on -25912 and -25914)
(900202, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS Both', 0x003F3F),
-- 900203: Holy Shock +50% damage/healing (DBC passive)
-- EffectSpellClassMaskA=0x200000 targets Holy Shock family
(900203, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x200000, 10, 156, 0, 'Holy: HS +50%', 0x003F3F),
-- 900204: Consecration also heals passive (marker, C++ on 48819)
(900204, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 51, 0, 'Holy: Consec Heal', 0x003F3F),
-- 900205: Consecration around you (DBC passive)
-- This is a marker; actual "follow caster" needs DBC edit on Consecration itself
-- or a C++ approach. For now: marker aura (DBC change on base spell needed separately)
(900205, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 51, 0, 'Holy: Consec Around', 0x003F3F),
-- 900206: Consecration +50% damage/heal (DBC passive)
-- EffectSpellClassMaskA=0x20 targets Consecration family (verify!)
(900206, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x20, 10, 51, 0, 'Holy: Consec +50%', 0x003F3F),
-- 900207: Consecration +5sec duration (DBC passive)
-- ADD_FLAT_MODIFIER (107), SPELLMOD_DURATION (17), BasePoints=5000 (ms)
-- EffectSpellClassMaskA=0x20 targets Consecration
(900207, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 5000, 1, 107, 17, 0, 0x20, 10, 51, 0, 'Holy: Consec +5s', 0x003F3F),
-- 900208: Helper - Holy Shock AoE damage (instant, Holy school, 10yd around target)
-- SchoolMask=2 (Holy), TARGET_UNIT_DEST_AREA_ENEMY (15)
(900208, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 100, 800, 15, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Burst', 0x003F3F),
-- 900209: Helper - Holy Shock AoE heal (instant, Holy school, 10yd around target)
-- SchoolMask=2 (Holy), TARGET_UNIT_DEST_AREA_ALLY (30)
(900209, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 100, 800, 30, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Radiance', 0x003F3F),
-- 900210: Helper - Consecration heal tick (instant AoE heal around caster)
-- Effect=HEAL(10), TARGET_UNIT_SRC_AREA_ALLY (31), 8yd radius
(900210, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 50, 200, 31, 0, 0, 0, 0, 10, 51, 2, 'Consecration Heal', 0x003F3F);

-- ============================================================
-- Paladin Prot: spell_dbc entries (900234-900241)
-- ============================================================
-- Avenger's Shield SpellFamilyFlags[0]: 0x4000 (verify in-game!)
-- Holy Shield SpellFamilyFlags[1]: 0x20 (verify in-game!)
-- Judgement SpellFamilyFlags[0]: 0x800000 (verify in-game!)

DELETE FROM `spell_dbc` WHERE `ID` IN (900234, 900235, 900236, 900237, 900238, 900239, 900240, 900241);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellFamilyName`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900234: Consecration around you (marker aura, shared concept with Holy 900205)
(900234, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 51, 0, 'PProt: Consec Around', 0x003F3F),
-- 900235: Avenger's Shield +9 jump targets (ADD_FLAT_MODIFIER + SPELLMOD_JUMP_TARGETS=17)
-- EffectSpellClassMaskA=0x4000 targets Avenger's Shield. BasePoints=9.
(900235, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x4000, 0, 10, 3477, 0, 'PProt: AS +9 Targets', 0x003F3F),
-- 900236: Avenger's Shield +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE=0)
-- EffectSpellClassMaskA=0x4000 targets Avenger's Shield.
(900236, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x4000, 0, 10, 3477, 0, 'PProt: AS +50%', 0x003F3F),
-- 900237: Holy Shield charges +99 (ADD_FLAT_MODIFIER + SPELLMOD_CHARGES=4)
-- EffectSpellClassMaskB=0x20 targets Holy Shield (flags[1]).
(900237, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 99, 1, 107, 4, 0, 0, 0x20, 10, 293, 0, 'PProt: HS +99 Charges', 0x003F3F),
-- 900238: Holy Shield +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE=0)
-- EffectSpellClassMaskB=0x20 targets Holy Shield (flags[1]).
(900238, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20, 10, 293, 0, 'PProt: HS +50%', 0x003F3F),
-- 900239: AS leaves Consecration (marker aura, logic in C++ on -48827)
(900239, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 3477, 0, 'PProt: AS Consec', 0x003F3F),
-- 900240: Judgement → free AS (marker aura, logic in C++ on 54158)
(900240, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 3015, 0, 'PProt: Judge AS', 0x003F3F),
-- 900241: Judgement cd -2sec (ADD_FLAT_MODIFIER + SPELLMOD_COOLDOWN=11)
-- EffectSpellClassMaskA=0x800000 targets Judgement spells. BasePoints=-2000 (ms).
(900241, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 10, 3015, 0, 'PProt: Judge -2s CD', 0x003F3F);

-- ============================================================
-- Paladin Ret: spell_proc entry
-- ============================================================

-- Exorcism buff proc (900274): procs on melee spell damage (CS/Judge/DS).
-- ProcFlags=0x10 (DONE_SPELL_MELEE_DMG_CLASS). C++ CheckProc filters to CS/Judge/DS.
DELETE FROM `spell_proc` WHERE `SpellId` = 900274;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900274, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- ============================================================
-- Paladin Ret: spell_dbc entries (900268-900275)
-- ============================================================
-- Divine Storm SpellFamilyFlags: flags[1]=0x20000 (verify in-game!)
-- Crusader Strike SpellFamilyFlags: flags[0]=0x1 (verify in-game!)
-- Exorcism SpellFamilyFlags: flags[0]=0x200000 (same bit as HS? verify!)
-- Judgement SpellFamilyFlags[0]: 0x800000

DELETE FROM `spell_dbc` WHERE `ID` IN (900268, 900269, 900270, 900271, 900272, 900273, 900274, 900275);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellFamilyName`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900268: Consecration around you (marker, shared with Holy/Prot)
(900268, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 51, 0, 0, 'Ret: Consec Around', 0x003F3F),
-- 900269: Judgement cd -2sec (same as Prot 900241 but separate ID for Ret)
(900269, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 10, 3015, 0, 0, 'Ret: Judge -2s CD', 0x003F3F),
-- 900270: Divine Storm +6 targets (marker aura; DS base spell needs MaxAffectedTargets DBC patch)
(900270, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 2292, 0, 0, 'Ret: DS +6 Targets', 0x003F3F),
-- 900271: Divine Storm +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- DS SpellFamilyFlags[1]=0x20000 (verify!)
(900271, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20000, 10, 2292, 0, 0, 'Ret: DS +50%', 0x003F3F),
-- 900272: Crusader Strike +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- CS SpellFamilyFlags[0]=0x1 (verify!)
(900272, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 10, 2286, 0, 0, 'Ret: CS +50%', 0x003F3F),
-- 900273: CS +9 targets (marker aura, logic in C++ on -35395)
(900273, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 2286, 0, 0, 'Ret: CS AoE', 0x003F3F),
-- 900274: Exorcism buff proc passive (DUMMY, proc via spell_proc + C++)
(900274, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 2286, 0, 0, 'Ret: Exorcism Proc', 0x003F3F),
-- 900275: Exorcism buff (stacking, +50% Exorcism damage per stack, max 10)
-- ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0), SpellClassMaskA for Exorcism
-- Duration: 30sec (DurationIndex=5), stackable (CumulativeAura=10)
-- NOT passive — it's a visible buff with stacks
(900275, 0x10000000, 0, 0, 0, 1, 5, 1, 6, 0, 50, 1, 108, 0, 0, 0x200000, 0, 10, 879, 0, 10, 'Exorcism Power', 0x003F3F);
