-- Link custom spell IDs to their SpellScript names
DELETE FROM `spell_script_names` WHERE `spell_id` IN (900106, 900107, 900116, 900117, 1680);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(900106, 'spell_custom_paragon_strike'),
(900107, 'spell_custom_bladestorm_cd_reduce'),
(900116, 'spell_custom_bloody_whirlwind_passive'),
(900117, 'spell_custom_speedy_bloodthirst'),
(1680, 'spell_custom_bloody_whirlwind_consume');

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
-- FamilyName/Mask=0 so the base proc check passes any melee spell; the script's
-- DoCheckProc narrows it to Bloodthirst (SpellFamilyFlags[0]=0x40000000).
DELETE FROM `spell_proc` WHERE `SpellId` = 900116;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900116, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Speedy Bloodthirst (900117): Whirlwind resets Bloodthirst CD.
-- ProcFlags=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS), SpellPhaseMask=4 (FINISH).
-- C++ CheckProc filters to Whirlwind (1680) only.
DELETE FROM `spell_proc` WHERE `SpellId` = 900117;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900117, 0, 0, 0, 0, 0, 0x10, 1, 4, 0, 0, 0, 0, 100, 0, 0);
