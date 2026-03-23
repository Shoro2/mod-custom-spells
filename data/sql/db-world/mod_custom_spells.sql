-- Link custom spell IDs to their SpellScript names
DELETE FROM `spell_script_names` WHERE `spell_id` IN (900106, 900107, 900116, 900117, 900120, 900121, 1680, 57823, 47502, 900126, 900127);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(900106, 'spell_custom_paragon_strike'),
(900107, 'spell_custom_bladestorm_cd_reduce'),
(900116, 'spell_custom_bloody_whirlwind_passive'),
(900117, 'spell_custom_speedy_bloodthirst'),
(900120, 'spell_custom_ww_overpower'),
(900121, 'spell_custom_ww_slam'),
(1680, 'spell_custom_bloody_whirlwind_consume'),
-- Warrior Prot: Revenge AoE hook (on Revenge max rank)
(57823, 'spell_custom_prot_revenge_aoe'),
-- Warrior Prot: TC → Rend + Sunder hook (on Thunderclap max rank)
(47502, 'spell_custom_prot_tc_rend_sunder'),
-- Warrior Prot: Block → AoE damage (passive proc aura)
(900126, 'spell_custom_prot_block_aoe'),
-- Warrior Prot: Block → Enhanced TC (passive proc aura)
(900127, 'spell_custom_prot_block_tc');

-- ICD for Whirlwind proc aura (900114): 500ms cooldown to prevent
-- recursive loop (Whirlwind hits count as auto-attacks → re-proc → crash)
DELETE FROM `spell_proc` WHERE `SpellId` = 900114;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900114, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 500, 0);

-- Bladestorm CD Reduce passive (900107): procs on melee damage dealt.
-- ProcFlags=0x14 (DONE_MELEE_AUTO_ATTACK | DONE_SPELL_MELEE_DMG_CLASS).
-- C++ CheckProc filters: only when Bladestorm is on cooldown.
DELETE FROM `spell_proc` WHERE `SpellId` = 900107;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900107, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Bloody Whirlwind passive (900116): C++ AuraScript handles Bloodthirst filtering.
-- ProcFlags=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS) matches Bloodthirst's melee dmg class.
DELETE FROM `spell_proc` WHERE `SpellId` = 900116;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900116, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Speedy Bloodthirst (900117): Whirlwind resets Bloodthirst CD.
-- ProcFlags=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS), SpellPhaseMask=4 (FINISH).
DELETE FROM `spell_proc` WHERE `SpellId` = 900117;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900117, 0, 0, 0, 0, 0, 0x10, 1, 4, 0, 0, 0, 0, 100, 0, 0);

-- ============================================================
-- Warrior Prot: spell_proc entries
-- ============================================================

-- Block → AoE damage (900126): procs on TAKEN_MELEE_AUTO_ATTACK (0x2).
-- C++ HandleProc checks HitMask for BLOCK. 100% chance, 1s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900126;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900126, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 100, 1000, 0);

-- Block → Enhanced TC (900127): procs on TAKEN_MELEE_AUTO_ATTACK (0x2).
-- C++ HandleProc checks HitMask for BLOCK. 10% chance, 3s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900127;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900127, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 10, 3000, 0);

-- ============================================================
-- Warrior Prot: spell_dbc entries (900122-900129)
-- ============================================================

DELETE FROM `spell_dbc` WHERE `ID` IN (900122, 900123, 900124, 900125, 900126, 900127, 900128, 900129);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellFamilyName`, `SpellIconID`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900122: Revenge +50% damage passive (SPELL_AURA_ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA_1=0x400 targets Revenge SpellFamilyFlags[0]
(900122, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x400, 4, 132, 'Prot: Revenge Damage', 0x003F3F),
-- 900123: Revenge AoE passive (marker aura, logic in C++)
(900123, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Revenge AoE', 0x003F3F),
-- 900124: TC → Rend + Sunder passive (marker aura, logic in C++)
(900124, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: TC Rend Sunder', 0x003F3F),
-- 900125: Thunderclap +50% damage passive (SPELL_AURA_ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA_1=0x80 targets Thunderclap SpellFamilyFlags[0]
(900125, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x80, 4, 132, 'Prot: TC Damage', 0x003F3F),
-- 900126: Block → AoE damage proc passive (SPELL_AURA_DUMMY)
(900126, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block AoE', 0x003F3F),
-- 900127: Block → Enhanced TC proc passive (SPELL_AURA_DUMMY)
(900127, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block TC', 0x003F3F),
-- 900128: Helper - Block AoE damage (instant AoE physical, 8yd radius)
(900128, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 100, 500, 22, 0, 0, 0, 0, 4, 132, 'Block Shield Burst', 0x003F3F),
-- 900129: Helper - Enhanced Thunderclap (instant AoE physical, 10yd radius)
(900129, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 1000, 22, 0, 0, 0, 0, 4, 132, 'Enhanced Thunderclap', 0x003F3F);
