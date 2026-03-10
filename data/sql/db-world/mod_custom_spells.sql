-- ============================================================
--  spell_dbc: Define custom spells in the server's spell store
--  The binary Spell.dbc has these spells but with corrupted alignment,
--  so the server can't read them. These SQL entries override/supplement.
-- ============================================================

-- 900106: Paragon Strike - School Damage spell (scripted in C++)
-- 900107: Bladestorm CD Reduce - Passive proc, casts DUMMY on melee hit
-- 900114: Whirlwind proc aura - Procs 900113 on melee auto-attack
-- 900115: Bloody Whirlwind Buff - +50% Whirlwind damage, stacks 5x, 15s
-- 900116: Bloody Whirlwind Passive - Procs 900115 on Bloodthirst (melee spell) hit

DELETE FROM `spell_dbc` WHERE `ID` IN (900106, 900107, 900114, 900115, 900116);
INSERT INTO `spell_dbc` (`ID`,`Attributes`,`AttributesEx3`,`CastingTimeIndex`,`RecoveryTime`,
  `ProcTypeMask`,`ProcChance`,`ProcCharges`,`DurationIndex`,`RangeIndex`,`CumulativeAura`,
  `EquippedItemClass`,`Effect_1`,`Effect_2`,`EffectDieSides_1`,`EffectDieSides_2`,
  `EffectBasePoints_1`,`EffectBasePoints_2`,`EffectBasePoints_3`,
  `ImplicitTargetA_1`,`ImplicitTargetA_2`,
  `EffectAura_1`,`EffectAura_2`,
  `EffectMiscValue_1`,`EffectMiscValue_2`,
  `EffectTriggerSpell_1`,`EffectTriggerSpell_2`,
  `EffectSpellClassMaskA_2`,`EffectSpellClassMaskB_1`,
  `SpellIconID`,`ActiveIconID`,`SpellPriority`,
  `Name_Lang_enUS`,`Name_Lang_Mask`,
  `SpellClassSet`,`DefenseType`,`SchoolMask`) VALUES
-- 900106: Paragon Strike (Effect: School Damage on enemy target)
(900106, 0, 0, 1, 0,
  0, 0, 0, 0, 2, 0,
  -1, 2, 0, 1, 0,
  0, 0, 0,
  6, 0,
  0, 0,
  0, 0,
  0, 0,
  0, 0,
  83, 83, 50,
  'Paragon Strike', 0,
  4, 2, 1),
-- 900107: Bladestorm CD Reduce (Passive proc -> DUMMY on melee hit)
(900107, 0x40040, 0x4000000, 1, 0,
  0x14, 20, 0, 0, 1, 0,
  -1, 3, 0, 1, 0,
  0, 0, 0,
  1, 0,
  0, 0,
  0, 0,
  0, 0,
  0, 0,
  2782, 2782, 50,
  'Bladestorm Cooldown Reduction', 0,
  0, 0, 1),
-- 900114: Whirlwind proc aura (Proc Trigger Spell 900113 on auto-attack)
(900114, 0x40040, 0x4000000, 1, 500,
  0x4, 20, 0, 0, 1, 0,
  -1, 6, 0, 1, 0,
  0, 0, 0,
  1, 0,
  42, 0,
  0, 0,
  900113, 0,
  0, 0,
  83, 83, 50,
  'Whirlwind Proc Aura', 0,
  0, 0, 1),
-- 900115: Bloody Whirlwind Buff (+50% WW dmg/stack, 5 stacks max, 15s)
(900115, 0x10, 0, 1, 0,
  0, 100, 0, 8, 1, 5,
  -1, 6, 0, 0, 0,
  50, 0, 0,
  1, 0,
  108, 0,
  0, 0,
  0, 0,
  4, 0,
  83, 83, 50,
  'Bloody Whirlwind', 0,
  4, 0, 1),
-- 900116: Bloody Whirlwind Passive (Proc Trigger 900115 on melee spell hit)
--   EFFECT_0 = PROC_TRIGGER_SPELL -> 900115 (matches C++ AuraScript hook)
(900116, 0x1d0, 0, 1, 0,
  0x10, 100, 0, 0, 1, 0,
  -1, 6, 0, 0, 0,
  0, 0, 0,
  1, 0,
  42, 0,
  0, 0,
  900115, 0,
  0, 0,
  83, 83, 50,
  'Bloody Whirlwind Passive', 0,
  4, 0, 1);

-- ============================================================
--  spell_script_names: Link spell IDs to C++ SpellScript classes
-- ============================================================

-- Link custom spell IDs to their SpellScript names
DELETE FROM `spell_script_names` WHERE `spell_id` IN (900106, 900107, 900116, 1680);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(900106, 'spell_custom_paragon_strike'),
(900107, 'spell_custom_bladestorm_cd_reduce'),
(900116, 'spell_custom_bloody_whirlwind_passive'),
(1680, 'spell_custom_bloody_whirlwind_consume');

-- ICD for Whirlwind proc aura (900114): 500ms cooldown to prevent
-- recursive loop (Whirlwind hits count as auto-attacks → re-proc → crash)
DELETE FROM `spell_proc` WHERE `SpellId` = 900114;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900114, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 500, 0);

-- Bloody Whirlwind passive (900116): C++ AuraScript handles Bloodthirst filtering.
-- ProcFlags=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS) matches Bloodthirst's melee dmg class.
-- FamilyName/Mask=0 so the base proc check passes any melee spell; the script's
-- DoCheckProc narrows it to Bloodthirst (SpellFamilyFlags[0]=0x40000000).
DELETE FROM `spell_proc` WHERE `SpellId` = 900116;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900116, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);
