-- Warrior Arms/Fury 900100-900121: complete rows of the block that was made by
-- hand in the server and client Spell.dbc (March 2026). The table wins over the
-- file, so these rows are the server's truth now and the auditor sees them.
-- Fixes against the file rows (2026-10-08 rework, custom-spells/08):
--  * passives no longer carry DO_NOT_DISPLAY/DO_NOT_LOG (0x180): visible in the
--    spellbook like every other custom passive
--  * 900101: SPELLMOD_COOLDOWN -2000 ms on Mortal Strike; the file row had
--    +3 % damage plus a flat -333 damage and no cooldown change at all
--  * 900108/900112: the copy-pasted effect 3 (-50 % chain damage on Mortal
--    Strike) is gone; 900110's effect 3 now aims at Bloodthirst, like
--    900103/900104 aim theirs at Mortal Strike/Overpower
--  * passives get DurationIndex 21 (permanent) like the other custom passives;
--    the editor's leftovers in unused effects (masks, auras) are cleared
DELETE FROM `spell_dbc` WHERE `ID` IN (900100, 900101, 900102, 900103, 900104, 900105, 900106, 900107, 900108, 900109, 900110, 900111, 900112, 900113, 900114, 900115, 900116, 900117, 900118, 900119, 900120, 900121);
INSERT INTO `spell_dbc` (`ID`, `Category`, `DispelType`, `Mechanic`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx4`, `AttributesEx5`, `AttributesEx6`, `AttributesEx7`, `ShapeshiftMask`, `unk_320_2`, `ShapeshiftExclude`, `unk_320_3`, `Targets`, `TargetCreatureType`, `RequiresSpellFocus`, `FacingCasterFlags`, `CasterAuraState`, `TargetAuraState`, `ExcludeCasterAuraState`, `ExcludeTargetAuraState`, `CasterAuraSpell`, `TargetAuraSpell`, `ExcludeCasterAuraSpell`, `ExcludeTargetAuraSpell`, `CastingTimeIndex`, `RecoveryTime`, `CategoryRecoveryTime`, `InterruptFlags`, `AuraInterruptFlags`, `ChannelInterruptFlags`, `ProcTypeMask`, `ProcChance`, `ProcCharges`, `MaxLevel`, `BaseLevel`, `SpellLevel`, `DurationIndex`, `PowerType`, `ManaCost`, `ManaCostPerLevel`, `ManaPerSecond`, `ManaPerSecondPerLevel`, `RangeIndex`, `Speed`, `ModalNextSpell`, `CumulativeAura`, `Totem_1`, `Totem_2`, `Reagent_1`, `Reagent_2`, `Reagent_3`, `Reagent_4`, `Reagent_5`, `Reagent_6`, `Reagent_7`, `Reagent_8`, `ReagentCount_1`, `ReagentCount_2`, `ReagentCount_3`, `ReagentCount_4`, `ReagentCount_5`, `ReagentCount_6`, `ReagentCount_7`, `ReagentCount_8`, `EquippedItemClass`, `EquippedItemSubclass`, `EquippedItemInvTypes`, `Effect_1`, `Effect_2`, `Effect_3`, `EffectDieSides_1`, `EffectDieSides_2`, `EffectDieSides_3`, `EffectRealPointsPerLevel_1`, `EffectRealPointsPerLevel_2`, `EffectRealPointsPerLevel_3`, `EffectBasePoints_1`, `EffectBasePoints_2`, `EffectBasePoints_3`, `EffectMechanic_1`, `EffectMechanic_2`, `EffectMechanic_3`, `ImplicitTargetA_1`, `ImplicitTargetA_2`, `ImplicitTargetA_3`, `ImplicitTargetB_1`, `ImplicitTargetB_2`, `ImplicitTargetB_3`, `EffectRadiusIndex_1`, `EffectRadiusIndex_2`, `EffectRadiusIndex_3`, `EffectAura_1`, `EffectAura_2`, `EffectAura_3`, `EffectAuraPeriod_1`, `EffectAuraPeriod_2`, `EffectAuraPeriod_3`, `EffectMultipleValue_1`, `EffectMultipleValue_2`, `EffectMultipleValue_3`, `EffectChainTargets_1`, `EffectChainTargets_2`, `EffectChainTargets_3`, `EffectItemType_1`, `EffectItemType_2`, `EffectItemType_3`, `EffectMiscValue_1`, `EffectMiscValue_2`, `EffectMiscValue_3`, `EffectMiscValueB_1`, `EffectMiscValueB_2`, `EffectMiscValueB_3`, `EffectTriggerSpell_1`, `EffectTriggerSpell_2`, `EffectTriggerSpell_3`, `EffectPointsPerCombo_1`, `EffectPointsPerCombo_2`, `EffectPointsPerCombo_3`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `EffectSpellClassMaskB_1`, `EffectSpellClassMaskB_2`, `EffectSpellClassMaskB_3`, `EffectSpellClassMaskC_1`, `EffectSpellClassMaskC_2`, `EffectSpellClassMaskC_3`, `SpellVisualID_1`, `SpellVisualID_2`, `SpellIconID`, `ActiveIconID`, `SpellPriority`, `Name_Lang_enUS`, `Name_Lang_Mask`, `NameSubtext_Lang_enUS`, `NameSubtext_Lang_Mask`, `Description_Lang_enUS`, `Description_Lang_Mask`, `AuraDescription_Lang_enUS`, `AuraDescription_Lang_Mask`, `ManaCostPct`, `StartRecoveryCategory`, `StartRecoveryTime`, `MaxTargetLevel`, `SpellClassSet`, `SpellClassMask_1`, `SpellClassMask_2`, `SpellClassMask_3`, `MaxTargets`, `DefenseType`, `PreventionType`, `StanceBarOrder`, `EffectChainAmplitude_1`, `EffectChainAmplitude_2`, `EffectChainAmplitude_3`, `MinFactionID`, `MinReputation`, `RequiredAuraVision`, `RequiredTotemCategoryID_1`, `RequiredTotemCategoryID_2`, `RequiredAreasID`, `SchoolMask`, `RuneCostID`, `SpellMissileID`, `PowerDisplayID`, `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`, `SpellDescriptionVariableID`, `SpellDifficultyID`) VALUES
(900100, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 49, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 33554432, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 564, 0, 50, 'Improved Mortal Strike Damage', 16712190, 'Passive', 16712190, 'Increases the damage caused by your Mortal Strike ability by $s1%', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900101, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, -2000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 33554432, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 564, 0, 50, 'Improved Mortal Strike CDR', 16712190, 'Passive', 16712190, 'Reduces the cooldown of your Mortal Strike by $/1000;s1 sec.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900102, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 49, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 26, 50, 'Improved Overpower Damage', 16712190, 'Passive', 16712190, 'Increases the damage caused by your Overpower ability by $s1%', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900103, 0, 0, 0, 80, 0, 0, 524288, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 87376, 25, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 6, 1, 0, 1, 0.0, 0.0, 0.0, 9, 0, -51, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 107, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 20, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 33554432, 0, 0, 0, 0, 0, 33554432, 0, 0, 0, 0, 564, 564, 50, 'Mortal Strike Cleave', 16712190, 'Passive', 16712190, 'Mortal Strike now hit up to 10 enemies.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900104, 0, 0, 0, 80, 0, 0, 524288, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 87376, 25, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 6, 1, 0, 1, 0.0, 0.0, 0.0, 9, 0, -51, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 107, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 20, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 4, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 26, 26, 50, 'Overpower Cleave', 16712190, 'Passive', 16712190, 'Overpower now hit up to 10 enemies.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900105, 0, 0, 0, 262208, 0, 0, 67108864, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 20, 20, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 900106, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1989, 1989, 50, 'Critical Execution', 16712190, 'Passive', 16712190, 'Your physical abilities or auto attacks have a chance of 20% to trigger a Paragon Strike:

Base: 666

66% AP

1% bonus damage per PL', 16712190, '', 16712190, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900106, 0, 0, 0, 2424848, 0, 0, 262144, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 1, 0, 0, 0, 0, 2, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 2, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 2, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0, 1648, 0, 50, 'Critical Execution', 16712190, 'Proc', 16712190, 'Unknown String: 5216195', 16712190, '', 16712190, 0, 0, 0, 0, 4, 536870912, 0, 0, 0, 2, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.0, 0.0, 0.0, 0, 0),
(900107, 0, 0, 0, 262208, 0, 0, 67108864, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 20, 100, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2782, 2782, 50, 'Timed Attacks', 16712190, 'Passive', 16712190, 'Dealing physical damage reduces your Bladestorm CD by .5 sec', 16712190, '', 16712190, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900108, 0, 0, 0, 80, 0, 0, 524288, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 87376, 25, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 996, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 277, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 83, 50, 'Whirwind unlimited targets', 16712190, 'Passive', 16712190, 'Your Whirlwind now has no target limitation.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900109, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 49, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 38, 50, 'Improved Bloodthirst Damage', 16712190, 'Passive', 16712190, 'Increases the damage caused by your Bloodthirst ability by $s1%', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900110, 0, 0, 0, 80, 0, 0, 524288, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 87376, 25, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 6, 1, 0, 1, 0.0, 0.0, 0.0, 9, 0, -51, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 107, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 20, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 1024, 0, 0, 0, 0, 0, 1024, 0, 0, 0, 38, 38, 50, 'Bloodthirst Cleave', 16712190, 'Passive', 16712190, 'Bloodthirst now hit up to 10 enemies.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900111, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 49, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 83, 50, 'Improved Whirlwind Damage', 16712190, 'Passive', 16712190, 'Increases the damage caused by your Whirlwind ability by $s1%', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900112, 0, 0, 0, 80, 0, 0, 524288, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 87376, 25, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 998, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 4194304, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 277, 277, 50, 'Cleave unlimited targets', 16712190, 'Passive', 16712190, 'Your Cleave now has no target limitation.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 4194304, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900113, 891, 0, 0, 327696, 16, 0, 1024, 0, 32768, 0, 0, 262144, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 36, 36, 0, 1, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 173555, 0, 121, 64, 0, 1, 0, 0, 0.0, 0.0, 0.0, -1, 0, 0, 0, 0, 0, 22, 22, 0, 15, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 44949, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 223, 0, 83, 0, 0, 'Whirly Attacks', 16712190, 'Proc', 16712190, 'In a whirlwind of steel you attack up to $i enemies within $a1 yards, causing weapon damage from both melee weapons to each enemy.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 4, 0, 4, 2, 2, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.0, 0.0, 0.0, 0, 0),
(900114, 0, 0, 0, 262208, 0, 0, 67108864, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 500, 0, 0, 0, 0, 4, 20, 0, 0, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 900113, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 83, 50, 'Whirly Attacks', 16712190, 'Passive', 16712190, 'Your auto attacks have a chance of 20% to trigger a free Whirlwind.', 16712190, '', 16712190, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900115, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 69648, 100, 1, 0, 0, 0, 8, 0, 0, 0, 0, 0, 1, 0.0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 49, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 83, 50, 'Bloody Whirlwind', 16712190, 'Aura', 16712190, '', 16712190, 'Your next Whirlwind deals 50% more damage. Stacks 5 times.', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900116, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4112, 100, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 6, 0, 0, 1, 0, 0.0, 0.0, 0.0, 0, -1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 900115, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 83, 83, 50, 'Bloody Whirlwind', 16712190, 'Passive', 16712190, 'Following a Bloodthirst, your next Whirlwind deals 50% more damage. Stacks 5 times.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900117, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4112, 100, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 38, 50, 'Speedy Bloodthirst', 16712190, 'Passive', 16712190, 'Your Whirlwind resets the cooldown of your Bloodthirst.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900118, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4112, 100, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 26, 50, 'Whirwind: Overpower', 16712190, 'Passive', 16712190, 'If your Whirlwind only hits one target, you cast a boosted Overpower on the target.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900119, 0, 0, 0, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4112, 100, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 1, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 6, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, 38, 50, 'Whirwind: Bloodthirst', 16712190, 'Passive', 16712190, 'If your Whirlwind only hits one target, you cast a boosted Bloodthirst on the target.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0),
(900120, 65, 0, 0, 2424848, 403702272, 0, 1024, 0, 0, 1024, 0, 65536, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 5000, 0, 0, 0, 0, 101, 0, 0, 12, 12, 37, 1, 0, 0, 0, 0, 2, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 173555, 0, 121, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, -1, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 39, 0, 26, 0, 0, 'Whirwind: Overpower', 16712190, 'Proc', 16712190, 'Instantly overpower the enemy, causing weapon damage.  Only useable after the target dodges.  The Overpower cannot be blocked, dodged or parried.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 4, 0, 0, 0, 2, 2, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.0, 0.0, 0.0, 0, 0),
(900121, 971, 0, 0, 327696, 134218240, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 101, 0, 0, 40, 40, 0, 1, 0, 0, 0, 0, 2, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 173555, 0, 2, 0, 0, 1, 0, 0, 0.0, 0.0, 0.0, 49, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 372, 0, 38, 0, 50, 'Whirwind: Bloodthirst', 16712190, 'Proc', 16712190, 'Instantly attack the target causing ${$AP*$m1/100} damage.  In addition, the next $23885n successful melee attacks will restore $m2% of max health.  This effect lasts $23885d.  Damage is based on your attack power.', 16712190, '', 16712190, 0, 0, 0, 0, 4, 0, 1024, 0, 0, 2, 2, 0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1.0, 0.0, 0.0, 0, 0);

-- 900105 Critical Execution: concept = "your critical abilities have a 20 %
-- chance": DONE_SPELL_MELEE_DMG_CLASS (0x10), damage, hit phase, HitMask
-- CRITICAL (0x2); the DBC's 0x14 also took white swings and every hit.
-- Warrior Arms/Fury proc rows (900114/900116 lived only in the live DB until
-- 2026-10-08). 900116 procs from Bloodthirst only (family 4, flags1 0x400).
-- 900117 is consumed by the Whirlwind script, not by a proc: its row goes.
DELETE FROM `spell_proc` WHERE `SpellId` IN (900105, 900114, 900116, 900117);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900105, 0, 0, 0, 0, 0, 0x10, 1, 2, 0x2, 0, 0, 0, 20, 0, 0),
(900114, 0, 0, 0, 0, 0, 0x4, 0, 0, 0, 0, 0, 0, 20, 500, 0),
(900116, 0, 4, 0, 0x400, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Bladestorm CD Reduce passive (900107)
DELETE FROM `spell_proc` WHERE `SpellId` = 900107;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900107, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- 900172 is no longer a proc aura (reworked into the Devastate Lightning
-- cast hook); the bare DELETE clears the legacy Block->AoE row from live DBs.
DELETE FROM `spell_proc` WHERE `SpellId` = 900172;

-- Block -> Enhanced TC (900173): TAKEN_MELEE_AUTO_ATTACK (0x8). 10% chance, 3s ICD.
-- HitMask 0x2040 = PROC_HIT_BLOCK | PROC_HIT_FULL_BLOCK. With HitMask 0 the
-- TAKEN default (NORMAL|CRIT) applies, and a full block nullifies the damage,
-- so the event only carries BLOCK|FULL_BLOCK and the proc never fired.
DELETE FROM `spell_proc` WHERE `SpellId` = 900173;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900173, 0, 0, 0, 0, 0, 0x8, 0, 0, 0x2040, 0, 0, 0, 10, 3000, 0);

-- Warrior Fury: 900133 Whirlwind in any stance (concept "remove whirlwind stance
-- req", built 2026-10-08). Aura 275 SPELL_AURA_MOD_IGNORE_SHAPESHIFT on the
-- Whirlwind mask (family 4, flags1 0x4 -> EffectSpellClassMaskA_2): Spell::CheckCast
-- skips the stance check for an affected spell, and the client honours the aura
-- (Warbringer 57499 frees Charge/Intercept/Intervene the same way).
DELETE FROM `spell_dbc` WHERE `ID` = 900133;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectSpellClassMaskA_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900133, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 275, 0x4, 4, 83, 1, 'Whirlwind: Any Stance', 0x003F3F);

-- Warrior Prot: spell_dbc 900168-900176
-- Area helpers need EffectRadiusIndex (13 = 10yd) - without it the area
-- search has 0yd radius and hits nothing. TargetA 15 = enemies around the
-- caster (Enhanced TC is a self-centered burst).
DELETE FROM `spell_dbc` WHERE `ID` IN (900168, 900169, 900170, 900171, 900172, 900173, 900175, 900176);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900168, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x400, 4, 132, 'Prot: Revenge Damage', 0x003F3F),
(900169, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 4, 132, 'Prot: Revenge AoE', 0x003F3F),
(900170, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 4, 132, 'Prot: TC Rend Sunder', 0x003F3F),
(900171, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x80, 4, 132, 'Prot: TC Damage', 0x003F3F),
(900172, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 4, 132, 'Prot: Devastate Lightning', 0x003F3F),
(900173, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 4, 132, 'Prot: Block TC', 0x003F3F),
(900175, 0, 0, 0, 0, 1, 0, 1, -1, 2, 200, 1000, 15, 13, 0, 0, 0, 0, 4, 132, 'Enhanced Thunderclap', 0x003F3F),
(900176, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 4, 132, 'Revenge Bounce', 0x003F3F);

-- 900174 Lightning Strike (Devastate Lightning damage helper): nature school
-- damage, cast AT the Devastate target -> dest-centered TargetA 16 with 10yd
-- radius (13). DieSides 0: the C++ hook always overrides BasePoints with
-- 666 + 5/Paragon level, so the value must pass through exactly.
-- SpellVisualID 11666 = wotlk 'Lightning Strike' sky bolt (52944).
DELETE FROM `spell_dbc` WHERE `ID` = 900174;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `SpellVisualID_1`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900174, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 0, 0, 4, 62, 8, 11666, 'Lightning Strike', 0x003F3F);

-- Paladin Holy: spell_dbc 900200-900210
-- 900206 uses SPELLMOD_DOT (22): Consecration's ticks are periodic damage and
-- Unit::SpellDamageBonusDone reads op 22 for them (op 0 changed nothing).
-- 900203 covers both Holy Shock lines: damage flags0 0x200000 and heal flags1
-- 0x10000 (A_2) - the heal half was missing until 2026-10-08.
-- 900207 uses SPELLMOD_DURATION (1), not JUMP_TARGETS. The two Holy Shock
-- bursts are cast AT the shock target -> dest-centered targets (16/31) with
-- 10yd radius (13); the Consecration heal pulses around the caster (31 with
-- the cast on self).
DELETE FROM `spell_dbc` WHERE `ID` IN (900200, 900201, 900202, 900203, 900204, 900205, 900206, 900207, 900208, 900209, 900210);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900200, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Damage', 0x003F3F),
(900201, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Heal', 0x003F3F),
(900202, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 10, 156, 0, 'Holy: HS Both', 0x003F3F),
(900203, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x200000, 0x10000, 10, 156, 0, 'Holy: HS +50%', 0x003F3F),
(900204, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 10, 51, 0, 'Holy: Consec Heal', 0x003F3F),
(900205, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 10, 51, 0, 'Holy: Consec Around', 0x003F3F),
(900206, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 22, 0, 0x20, 0, 10, 51, 0, 'Holy: Consec +50%', 0x003F3F),
(900207, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 5000, 1, 0, 107, 1, 0, 0x20, 0, 10, 51, 0, 'Holy: Consec +5s', 0x003F3F),
(900208, 0, 0, 0, 0, 1, 0, 1, -1, 2, 100, 800, 16, 13, 0, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Burst', 0x003F3F),
(900209, 0, 0, 0, 0, 1, 0, 1, -1, 10, 100, 800, 31, 13, 0, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Radiance', 0x003F3F),
(900210, 0, 0, 0, 0, 1, 0, 1, -1, 10, 50, 200, 31, 13, 0, 0, 0, 0, 0, 10, 51, 2, 'Consecration Heal', 0x003F3F);

-- 900211/900212: Consecration tickers - periodic dummy self-auras, 1s ticks
-- over Consecration's 8s (DurationIndex 31; the cast hook re-applies the
-- player's duration mods). 900211 deals the carried per-tick amount around
-- the paladin (markers 900205/900234/900268), 900212 heals it (marker 900204).
DELETE FROM `spell_dbc` WHERE `ID` IN (900211, 900212);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900211, 0, 0, 0, 0, 1, 31, 1, -1, 6, 0, 0, 1, 226, 1000, 10, 51, 2, 'Mobile Consecration', 0x003F3F),
(900212, 0, 0, 0, 0, 1, 31, 1, -1, 6, 0, 0, 1, 226, 1000, 10, 51, 2, 'Mobile Consecration Heal', 0x003F3F);

-- Paladin Prot: spell_dbc 900234-900241
-- Masks verified against Spell.dbc SpellFamilyFlags: Avenger's Shield flags0=0x4000,
-- Holy Shield flags1=0x40 (goes in A_2), Judgement flags0=0x800000 + JoJustice flags2=0x8.
DELETE FROM `spell_dbc` WHERE `ID` IN (900234, 900235, 900236, 900237, 900238, 900239, 900240, 900241);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900234, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 10, 51, 0, 'PProt: Consec Around', 0x003F3F),
(900235, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 9, 1, 107, 17, 0, 0x4000, 0, 0, 10, 3477, 0, 'PProt: AS +9 Targets', 0x003F3F),
(900236, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x4000, 0, 0, 10, 3477, 0, 'PProt: AS +50%', 0x003F3F),
(900237, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 99, 1, 107, 4, 0, 0, 0x40, 0, 10, 293, 0, 'PProt: HS +99 Charges', 0x003F3F),
(900238, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x40, 0, 10, 293, 0, 'PProt: HS +50%', 0x003F3F),
(900239, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 10, 3477, 0, 'PProt: AS Consec', 0x003F3F),
(900240, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 10, 3015, 0, 'PProt: Judge AS', 0x003F3F),
(900241, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 0x8, 10, 3015, 0, 'PProt: Judge -2s CD', 0x003F3F);

-- Paladin Ret: spell_proc 900274
DELETE FROM `spell_proc` WHERE `SpellId` = 900274;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900274, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Paladin Ret: spell_dbc 900268-900276
-- Masks verified against Spell.dbc SpellFamilyFlags: Judgement flags0=0x800000 +
-- JoJustice flags2=0x8, Divine Storm flags1=0x20000, Crusader Strike flags1=0x8000,
-- Exorcism flags1=0x2 (the old 0x200000 in flags0 hit Holy Shock instead).
-- 900270 is SPELL_AURA_MOD_MAX_AFFECTED_TARGETS (277) on Divine Storm: the core's own
-- area cap 4 -> 10 (Spell::SelectImplicitAreaTargets), every target a real DS hit.
DELETE FROM `spell_dbc` WHERE `ID` IN (900268, 900269, 900270, 900271, 900272, 900273, 900274, 900275, 900276);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900268, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 10, 51, 0, 0, 'Ret: Consec Around', 0x003F3F),
(900269, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 0x8, 10, 3015, 0, 0, 'Ret: Judge -2s CD', 0x003F3F),
(900270, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 6, 1, 277, 0, 0, 0, 0x20000, 0, 10, 2292, 0, 0, 'Ret: DS +6 Targets', 0x003F3F),
(900271, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20000, 0, 10, 2292, 0, 0, 'Ret: DS +50%', 0x003F3F),
(900272, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x8000, 0, 10, 2286, 0, 0, 'Ret: CS +50%', 0x003F3F),
(900273, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0, 0x8000, 0, 10, 2286, 0, 0, 'Ret: CS +9 Targets', 0x003F3F),
(900274, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 10, 2286, 0, 0, 'Ret: Exorcism Proc', 0x003F3F),
(900275, 0, 0, 0, 0, 1, 5, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x2, 0, 10, 292, 0, 10, 'Exorcism Power', 0x003F3F),
(900276, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 10, 2286, 2, 0, 'CS Bounce', 0x003F3F);
