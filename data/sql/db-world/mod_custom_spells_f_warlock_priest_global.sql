-- Warlock Affli: DoT procs 900800/900802 (DONE_PERIODIC=0x40000), 900804
-- Withering Harvest (2026-10-08): every warlock periodic damage tick (chance
-- 100, no cooldown) adds a stack; its effect 1 is the 5-s Drain Life pulse and
-- takes no part in the proc (DisableEffectsMask 0x2).
DELETE FROM `spell_proc` WHERE `SpellId` IN (900800, 900802, 900804);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900800, 0, 5, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 20, 2000, 0),
(900802, 0, 5, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 15, 3000, 0),
(900804, 0, 5, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0x2, 0, 100, 0, 0);

-- Warlock Affli: spell_dbc 900800-900803
-- 900803 Shadow Eruption (concept revision 2026-10-08, "dmg dependent on
-- paragon lvl"): the proc casts it with 666 + 5 x the warlock's Paragon level
-- as its points (custom_spells_warlock.cpp); the row's own 666 (no die) is
-- what a cast without them deals. SPELL_ATTR3_ALWAYS_HIT (0x40000): a
-- proc burst does not miss.
DELETE FROM `spell_dbc` WHERE `ID` IN (900800, 900801, 900802, 900803);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900800, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Affl: DoT AoE', 0x003F3F),
(900801, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 22, 0, 0x2, 0, 0, 5, 313, 0, 0, 'Affl: Corruption +50%', 0x003F3F),
(900802, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Affl: DoT Spread', 0x003F3F),
(900803, 0, 0, 0, 0x40000, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 0, 0, 0, 0, 5, 313, 32, 0, 'Shadow Eruption', 0x003F3F);

-- Warlock Affli: Withering Harvest (2026-10-08). 900804 marker: effect 0 dummy
-- (the proc above), effect 1 periodic dummy every 5 s (the Drain Life pulse).
-- 900805 the stacks: aura 108 SPELLMOD_DOT (22) +1 % on every warlock spell
-- (all three mask words), 100 stacks, 30 s. 900806 the free Drain Life: Drain
-- Life rank 9's numbers (aura 53, 133 every 1 s for 5 s, 0.143 spell power, a
-- heal of 100 % = EffectMultipleValue 1.0, family flags 0x8) as an aura on the
-- enemy - no channel, so no cast bar; any range (the script picks within
-- 30 yd).
DELETE FROM `spell_dbc` WHERE `ID` IN (900804, 900805, 900806);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectAuraPeriod_2`, `EffectSpellClassMaskB_1`, `CumulativeAura`, `SpellClassSet`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900804, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 0, 0, 1, 226, 0, 5000, 0, 0, 5, 0, 546, 0, 'Withering Harvest', 0x003F3F),
(900805, 0, 0, 0, 1, 9, 1, -1, 6, 0, 1, 1, 108, 22, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0, 0, 100, 5, 0, 546, 0, 'Withering Harvest', 0x003F3F);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `ImplicitTargetB_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectMultipleValue_1`, `EffectBonusMultiplier_1`, `EffectChainAmplitude_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectMechanic_2`, `EffectChainAmplitude_2`, `Effect_3`, `EffectBasePoints_3`, `ImplicitTargetA_3`, `EffectAura_3`, `EffectMiscValue_3`, `SpellClassSet`, `SpellClassMask_1`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `DefenseType`, `PreventionType`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900806, 1, 0, 0, 1, 28, 13, -1, 6, 0, 133, 6, 0, 0, 53, 0, 1000, 1.0, 0.143, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0x8, 12655, 546, 32, 1, 1, 'Drain Life', 0x003F3F);

-- Warlock Demo: spell_dbc 900833-900844
-- 900834 reworked 2026-10-08 (concept "while in demon form your immolation heals
-- yourself and your demon get +100% damage"): a dummy marker now - the heal is
-- spell_custom_wlk_fel_vigor on Immolation 50590, the demons' +100 % is minion
-- aura 900845 (custom_spells_global.cpp). Its old helpers 900842 (Meta Shadow
-- Burst) and 900843 (Meta Demon Heal) are retired: kept in the DELETE so live
-- databases drop them, no longer inserted.
DELETE FROM `spell_dbc` WHERE `ID` IN (900833, 900834, 900835, 900836, 900837, 900838, 900839, 900840, 900841, 900842, 900843, 900844);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900833, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Meta Kill Extend', 0x003F3F),
(900834, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 4, 0, 'Metamorphosis: Fel Vigor', 0x003F3F),
(900835, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Lesser Spawn', 0x003F3F),
(900836, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Imp FB +50%', 0x003F3F),
(900837, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Imp FB AoE', 0x003F3F),
(900838, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: FG Unlim', 0x003F3F),
(900839, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: FG +50%', 0x003F3F),
(900840, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Sacrifice All', 0x003F3F),
(900841, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 5, 313, 4, 0, 'Imp Firebolt Bounce', 0x003F3F),
(900844, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 5, 313, 1, 0, 'FG Cleave Hit', 0x003F3F);

-- Warlock Demo, concept revision 2026-10-08 (900845-900857; 900858-900865 are
-- reserved for the two-demon line, built elsewhere). Minion auras 900845 /
-- 900847 / 900849 are passive (0x40) and handed out by the minion manager
-- (custom_spells_global.cpp): 900845 Fel Fury aura 79 +100 % all schools on
-- every minion while the warlock has Metamorphosis (900834); 900847 aura 65
-- +50 % casting speed on the Imp (900846: Firebolt's 2.5 s cast and the Imp's
-- autocast rate x1.5; 900836's +50 % damage stays as it is); 900849 on the
-- Voidwalker (900848): aura 133 +500 % maximum health and a 10-s periodic dummy
-- (Suffering in combat). 900850 Mass Seduction: SPELLMOD_JUMP_TARGETS (17)
-- value 10 on Seduction's family flag flags1 0x10000000 (A_2; Seduction 6358 is
-- the only warlock spell with it), the succubus casts with its owner's
-- modifiers; 900851 its debuff, aura 87 +25 % damage taken (all schools), 30 s,
-- 4 stacks. 900852 Felhunter (operator 2026-10-08: "Spell Lock is aoe now,
-- Devour Magic also"): a marker, the C++ repeats the cast within 10 yd.
DELETE FROM `spell_dbc` WHERE `ID` IN (900845, 900846, 900847, 900848, 900849, 900850, 900851, 900852);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectAuraPeriod_2`, `EffectSpellClassMaskB_1`, `CumulativeAura`, `SpellClassSet`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900845, 0, 0x40, 0, 1, 21, 1, -1, 6, 0, 100, 1, 79, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 3314, 0, 'Fel Fury', 0x003F3F),
(900846, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 18, 0, 'Imp: Firebolt 50% Faster', 0x003F3F),
(900847, 0, 0x40, 0, 1, 21, 1, -1, 6, 0, 50, 1, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 18, 0, 'Quickened Firebolt', 0x003F3F),
(900848, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 217, 0, 'Voidwalker: Void Bulwark', 0x003F3F),
(900849, 0, 0x40, 0, 1, 21, 1, -1, 6, 0, 500, 1, 133, 0, 0, 0, 0, 0, 6, 0, 0, 1, 226, 0, 10000, 0, 0, 5, 0, 217, 0, 'Void Bulwark', 0x003F3F),
(900850, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0, 0x10000000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 48, 0, 'Succubus: Mass Seduction', 0x003F3F),
(900851, 1, 0, 0, 1, 9, 13, -1, 6, 0, 25, 6, 87, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 0, 48, 32, 'Seductive Torment', 0x003F3F),
(900852, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 77, 0, 'Felhunter: Area Spell Lock and Devour Magic', 0x003F3F);

-- Warlock Lesser Demons creature_template
DELETE FROM `creature_template` WHERE `entry` IN (900835, 900836, 900837);
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(900835, 0, 0, 0, 0, 0, 'Lesser Imp', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 0.5, 1500, 2000, 1, 1, 1, 0, 2048, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.3, 0.3, 0.3, 0, 0, 0, 1, 0, 0, '', 12340),
(900836, 0, 0, 0, 0, 0, 'Lesser Felguard', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 0.5, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.5, 0.3, 0.5, 0, 0, 0, 1, 0, 0, '', 12340),
(900837, 0, 0, 0, 0, 0, 'Lesser Voidwalker', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 0.3, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.6, 0.3, 0.6, 0, 0, 0, 1, 0, 0, '', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (900835, 900836, 900837);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900835, 0, 4449, 0.6, 1, 12340),
(900836, 0, 18399, 0.7, 1, 12340),
(900837, 0, 1132, 0.6, 1, 12340);

-- Warlock Destro: spell_dbc 900866-900872
-- Masks verified against Spell.dbc SpellFamilyFlags: Shadow Bolt flags0=0x1,
-- Conflagrate/Chaos Bolt line flags1=0x800000 (goes in A_2, not 0x1000000).
DELETE FROM `spell_dbc` WHERE `ID` IN (900866, 900867, 900868, 900869, 900870, 900871, 900872);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900866, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x1, 0, 0, 5, 313, 0, 0, 'Destro: SB +9', 0x003F3F),
(900867, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 0, 5, 313, 0, 0, 'Destro: SB +50%', 0x003F3F),
(900868, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20000, 0, 5, 313, 0, 0, 'Destro: CB +50%', 0x003F3F),
(900869, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, -2000, 1, 107, 11, 0, 0, 0x20000, 0, 5, 313, 0, 0, 'Destro: CB CD -2s', 0x003F3F),
(900870, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0, 0x20000, 0, 5, 313, 0, 0, 'Destro: CB +9', 0x003F3F),
(900871, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 5, 313, 32, 0, 'Shadow Bolt Bounce', 0x003F3F),
(900872, 0x100000, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 5, 313, 4, 0, 'Chaos Bolt Bounce', 0x003F3F);

-- Warlock Destro, concept revision 2026-10-08: Hellfire and Rain of Fire without
-- a channel (custom_spells_warlock.cpp). 900873 / 900875 markers. 900874 the
-- Hellfire ticker (positive, cancellable: a right-click ends Hellfire): periodic
-- dummy every 1 s, 15 s here and the cast channel's duration at runtime; its
-- points carry the cast rank. 900876 the Rain of Fire ticker: every 2 s, 8 s.
-- 900877 Rain of Fire's tick round the warlock: Fire damage to every enemy
-- within 8 yd (22/15, radius 14), the script sets the cast rank's tick points,
-- 0.286 spell power and Rain of Fire's family flag 0x20 as the stock tick 47818.
DELETE FROM `spell_dbc` WHERE `ID` IN (900873, 900874, 900875, 900876, 900877);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectAuraPeriod_2`, `EffectSpellClassMaskB_1`, `CumulativeAura`, `SpellClassSet`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900873, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 937, 0, 'Hellfire: Free Movement', 0x003F3F),
(900874, 0, 0, 0, 1, 8, 1, -1, 6, 0, 0, 1, 226, 0, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5423, 937, 4, 'Hellfire', 0x003F3F),
(900875, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 547, 0, 'Rain of Fire: Around You', 0x003F3F),
(900876, 0, 0, 0, 1, 31, 1, -1, 6, 0, 0, 1, 226, 0, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 547, 4, 'Rain of Fire', 0x003F3F);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `ImplicitTargetB_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectMultipleValue_1`, `EffectBonusMultiplier_1`, `EffectChainAmplitude_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectMechanic_2`, `EffectChainAmplitude_2`, `Effect_3`, `EffectBasePoints_3`, `ImplicitTargetA_3`, `EffectAura_3`, `EffectMiscValue_3`, `SpellClassSet`, `SpellClassMask_1`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `DefenseType`, `PreventionType`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900877, 0, 0, 0, 1, 0, 1, -1, 2, 0, 675, 22, 15, 14, 0, 0, 0, 0, 0.286, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0x20, 10045, 547, 4, 1, 1, 'Rain of Fire', 0x003F3F);

-- Priest Disc: spell_dbc 900900-900903
-- 900903 Shield Explosion (concept revision 2026-10-08, "dmg depended on paragon
-- lvl"): dest-centred now (16, 10 yd, any range) - the script aims it at the
-- shielded unit and sets 666 + 5 x the priest's Paragon level; the row's 666 is
-- what a cast without points deals. Holy, SPELL_ATTR3_ALWAYS_HIT.
DELETE FROM `spell_dbc` WHERE `ID` IN (900900, 900901, 900902, 900903);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900900, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Disc: Shield Explode', 0x003F3F),
(900901, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 8, 0, 0x1, 0, 0, 6, 566, 0, 0, 'Disc: Shield +50%', 0x003F3F),
(900902, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Disc: Weakened Soul CD', 0x003F3F),
(900903, 0, 0, 0, 0x40000, 1, 0, 13, -1, 2, 0, 666, 16, 13, 0, 0, 0, 0, 0, 0, 6, 566, 2, 0, 'Shield Explosion', 0x003F3F);

-- Priest Disc, concept revision 2026-10-08: 900904 Atonement (marker, proc below),
-- 900905 Smite +9 targets (SPELLMOD_JUMP_TARGETS value 10: Smite has no chain of
-- its own), 900906 Smite +50 % - Smite flags0 0x80.
DELETE FROM `spell_dbc` WHERE `ID` IN (900904, 900905, 900906);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectAuraPeriod_2`, `EffectSpellClassMaskB_1`, `CumulativeAura`, `SpellClassSet`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900904, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 2818, 0, 'Atonement', 0x003F3F),
(900905, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 237, 0, 'Smite: +9 Targets', 0x003F3F),
(900906, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 237, 0, 'Smite: +50% Damage', 0x003F3F);

-- Priest Holy: spell_dbc 900933
DELETE FROM `spell_dbc` WHERE `ID` = 900933;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900933, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Holy: Heal Fire', 0x003F3F);

-- Priest Holy, concept revision 2026-10-08: 900935 Holy Fire +9 targets (value 10
-- on flags0 0x100000), 900936 Holy Fire +50 % (direct damage op 0 and its DoT op
-- 22, each effect with its own mask), 900937 Lightwell: Auto Renew (periodic
-- dummy every 1 s), 900938 Spirit of Redemption: Guardian (marker, death proc
-- below), 900939 Empowered Redemption (+50 % casting speed aura 65, damage aura
-- 79, healing aura 136; the script sets the spirit form's duration), 900940
-- Redemption Spent (5 min; cannot be cancelled 0x80000000, persists through
-- death 0x100000), 900941 Holy Nova +50 % radius (SPELLMOD_RADIUS 6 on the damage
-- 0x400000 and the heal 0x8000000), 900942 Holy Nova: Kindled Fire (marker) and
-- 900943 its stacks (SPELLMOD_DAMAGE +50 % on Holy Fire, 10 stacks, 30 s; Holy
-- Fire's cast removes them).
DELETE FROM `spell_dbc` WHERE `ID` IN (900935, 900936, 900937, 900938, 900939, 900940, 900941, 900942, 900943);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectAuraPeriod_2`, `EffectSpellClassMaskB_1`, `CumulativeAura`, `SpellClassSet`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900935, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x100000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 156, 0, 'Holy Fire: +9 Targets', 0x003F3F),
(900936, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x100000, 0, 0, 6, 0, 50, 1, 108, 22, 0, 0x100000, 0, 6, 0, 156, 0, 'Holy Fire: +50% Damage', 0x003F3F),
(900937, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 226, 0, 1000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1878, 0, 'Lightwell: Auto Renew', 0x003F3F),
(900938, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1654, 0, 'Spirit of Redemption: Guardian', 0x003F3F),
(900940, 0, 0x80000000, 0x100000, 1, 5, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 241, 0, 'Redemption Spent', 0x003F3F),
(900941, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 6, 0, 0x8400000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1874, 0, 'Holy Nova: +50% Radius', 0x003F3F),
(900942, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1874, 0, 'Holy Nova: Kindled Fire', 0x003F3F),
(900943, 0, 0, 0, 1, 9, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x100000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 6, 0, 156, 0, 'Kindled Fire', 0x003F3F);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `ImplicitTargetB_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectMultipleValue_1`, `EffectBonusMultiplier_1`, `EffectChainAmplitude_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectMechanic_2`, `EffectChainAmplitude_2`, `Effect_3`, `EffectBasePoints_3`, `ImplicitTargetA_3`, `EffectAura_3`, `EffectMiscValue_3`, `SpellClassSet`, `SpellClassMask_1`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `DefenseType`, `PreventionType`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900939, 0, 0, 0, 1, 1, 1, -1, 6, 0, 50, 1, 0, 0, 65, 0, 0, 0, 0, 0, 6, 0, 50, 1, 79, 127, 0, 0, 6, 50, 1, 136, 0, 6, 0, 0, 1654, 0, 0, 0, 'Empowered Redemption', 0x003F3F);

-- Priest Shadow: spell_dbc 900966-900968
-- 900968 Shadow Eruption: as 900803 (Paragon points from the proc, 666 by row,
-- no die, SPELL_ATTR3_ALWAYS_HIT).
DELETE FROM `spell_dbc` WHERE `ID` IN (900966, 900967, 900968);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900966, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Shadow: DoT AoE', 0x003F3F),
(900967, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Shadow: DoT Spread', 0x003F3F),
(900968, 0, 0, 0, 0x40000, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 0, 0, 0, 0, 6, 566, 32, 0, 'Shadow Eruption', 0x003F3F);

-- Priest Shadow, concept revision 2026-10-08: 900969 Shadow Word: Pain +50 % (a pure
-- DoT: SPELLMOD_DOT 22, flags0 0x8000), 900970 Mind Blast +50 % and 900971 +9
-- targets (value 10; flags0 0x2000), 900972 Shadowform: Soul Feast (periodic
-- dummy every 5 s = the Mind Flay pulse; the kill stacks are a PlayerScript),
-- 900973 its stacks (aura 79 +1 % Shadow damage, 100 stacks, 60 s), 900974 the
-- free Mind Flay (Mind Flay rank 9's tick 196 every 1 s for 3 s, 0.257 spell
-- power, Mind Flay's damage flag 0x800000, and its 50 % slow) as an aura on the
-- enemy - no channel, so no cast bar, 900975 Shadowform: Lesser Tentacles
-- (marker), 900976 the tentacle's lash (401-600 Shadow, 15 yd, generic family:
-- none of the priest's modifiers).
DELETE FROM `spell_dbc` WHERE `ID` IN (900969, 900970, 900971, 900972, 900973, 900974, 900975, 900976);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectAuraPeriod_2`, `EffectSpellClassMaskB_1`, `CumulativeAura`, `SpellClassSet`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900969, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 22, 0, 0x8000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 234, 0, 'Shadow Word: Pain: +50% Damage', 0x003F3F),
(900970, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x2000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 95, 0, 'Mind Blast: +50% Damage', 0x003F3F),
(900971, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x2000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 95, 0, 'Mind Blast: +9 Targets', 0x003F3F),
(900972, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 226, 0, 5000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1552, 0, 'Shadowform: Soul Feast', 0x003F3F),
(900973, 0, 0, 0, 1, 3, 1, -1, 6, 0, 1, 1, 79, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 6, 0, 1552, 0, 'Soul Feast', 0x003F3F),
(900975, 0, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 2296, 0, 'Shadowform: Lesser Tentacles', 0x003F3F);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `ImplicitTargetB_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `EffectMultipleValue_1`, `EffectBonusMultiplier_1`, `EffectChainAmplitude_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectMechanic_2`, `EffectChainAmplitude_2`, `Effect_3`, `EffectBasePoints_3`, `ImplicitTargetA_3`, `EffectAura_3`, `EffectMiscValue_3`, `SpellClassSet`, `SpellClassMask_1`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `DefenseType`, `PreventionType`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900974, 1, 0, 0, 1, 27, 13, -1, 6, 0, 196, 6, 0, 0, 3, 0, 1000, 0, 0.257, 1.0, 6, 0, -50, 6, 33, 0, 11, 1.0, 0, 0, 0, 0, 0, 6, 0x800000, 12637, 548, 32, 1, 1, 'Mind Flay', 0x003F3F),
(900976, 0, 0, 0, 1, 0, 11, -1, 2, 200, 400, 6, 0, 0, 0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3057, 95, 32, 1, 1, 'Tentacle Lash', 0x003F3F);

-- Priest: spell_proc. 900933 (concept revision 2026-10-08): direct priest heals
-- (DONE_SPELL_MAGIC_DMG_CLASS_POS 0x4000, heal type 2, hit phase), 50 %, no
-- cooldown (was 20 %, 3 s). 900904 Atonement: melee 0x4, melee abilities 0x10,
-- ranged/wand 0x40, ranged abilities 0x100, no-class spells 0x1000, spells
-- 0x10000, ticks 0x40000; damage, hit phase; triggered damage counts too
-- (PROC_ATTR_TRIGGERED_CAN_PROC 0x2). 900938 Spirit of Redemption: Guardian: PROC_FLAG_DEATH 0x01000000 (runs
-- in Unit::Kill before Spirit of Redemption's CombatStop).
DELETE FROM `spell_proc` WHERE `SpellId` IN (900904, 900933, 900938, 900966, 900967);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900904, 0, 0, 0, 0, 0, 0x51154, 1, 2, 0, 0x2, 0, 0, 100, 0, 0),
(900933, 0, 6, 0, 0, 0, 0x4000, 2, 2, 0, 0, 0, 0, 50, 0, 0),
(900938, 0, 0, 0, 0, 0, 0x01000000, 0, 0, 0, 0, 0, 0, 100, 0, 0),
(900966, 0, 6, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 20, 2000, 0),
(900967, 0, 6, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- Priest creatures (concept revision 2026-10-08). 900964 Redemption Guardian
-- (900938): a passive decoy (NullCreatureAI: never attacks, never evades) with
-- three times a level-80 mob's health; the script summons it for the spirit
-- form's duration with the priest's faction and engages the priest's enemies with
-- it. Model: the Guardian Spirit angel (12824). 900999 Lesser Tentacle (900975):
-- rooted, lashes the nearest enemy in combat (npc_custom_lesser_tentacle), 15 s.
-- Model: C'Thun's Eye Tentacle (15788) at half size.
DELETE FROM `creature_template` WHERE `entry` IN (900964, 900999);
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(900964, 0, 0, 0, 0, 0, 'Redemption Guardian', '', '', 0, 80, 80, 2, 35, 0, 1, 1.14286, 1, 1, 20, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 'NullCreatureAI', 0, 1, 3, 1, 1, 0, 0, 0, 1, 0, 0, '', 12340),
(900999, 0, 0, 0, 0, 0, 'Lesser Tentacle', '', '', 0, 80, 80, 2, 35, 0, 1, 1.14286, 1, 1, 20, 0, 5, 0.3, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.3, 1, 0.5, 0, 0, 0, 1, 0, 0, 'npc_custom_lesser_tentacle', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (900964, 900999);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900964, 0, 12824, 1.6, 1, 12340),
(900999, 0, 15788, 0.5, 1, 12340);

-- Global: spell_dbc 901100-901111 (901111 = concept "increase damage done by
-- 5 %", aura 79 all schools, built 2026-10-08). 901109/901110 are the minion auras the
-- shared minion manager puts on summons and pets (+50 % damage done /
-- +50 % melee haste) for 900435, 900502, 900503, 900836, 900839.
DELETE FROM `spell_dbc` WHERE `ID` IN (901100, 901101, 901102, 901103, 901104, 901105, 901106, 901107, 901108, 901109, 901110, 901111);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(901100, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Cast Moving', 0x003F3F),
(901101, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Kill Heal', 0x003F3F),
(901102, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Extra Attack', 0x003F3F),
(901103, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Cleave Proc', 0x003F3F),
(901104, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Counter', 0x003F3F),
(901105, 0, 0, 0, 0, 1, 0, 1, -1, 10, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Kill Heal', 0x003F3F),
(901106, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Cleave Hit', 0x003F3F),
(901107, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Counter Strike', 0x003F3F),
(901108, 0, 0, 0, 0, 1, 0, 1, -1, 19, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Extra Attack Hit', 0x003F3F),
(901109, 0x40, 0, 0, 0, 1, 21, 1, -1, 6, 0, 50, 1, 79, 127, 0, 0, 0, 0, 0, 136, 0, 0, 'Empowered Minion', 0x003F3F),
(901110, 0x40, 0, 0, 0, 1, 21, 1, -1, 6, 0, 50, 1, 138, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Hastened Minion', 0x003F3F),
(901111, 0x40, 0, 0, 0, 1, 21, 1, -1, 6, 0, 5, 1, 79, 127, 0, 0, 0, 0, 0, 1661, 0, 0, 'Global: Empowered', 0x003F3F);

-- Global: spell_proc - corrected ProcFlags (KILL=0x2, TAKEN_MELEE_AUTO=0x8)
DELETE FROM `spell_proc` WHERE `SpellId` IN (901101, 901102, 901103, 901104);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901101, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 100, 0, 0),
(901102, 0, 0, 0, 0, 0, 0x14, 0, 2, 0, 0, 0, 0, 25, 1000, 0),
(901103, 0, 0, 0, 0, 0, 0x10154, 1, 2, 0, 0, 0, 0, 10, 1000, 0),
(901104, 0, 0, 0, 0, 0, 0x8, 0, 0, 0x2074, 0, 0, 0, 100, 2000, 0);

-- 900934 Holy Fire for Holy Fire Heals (900933): Holy Fire 48135 needs the priest to
-- face its target (FacingCasterFlags 1) and a 30-yd range from the priest, so the
-- proc reached only the enemies in front of the priest. Same damage and DoT
-- (48135 rank 11), no facing, any range; logged under its own name.
DELETE FROM `spell_dbc` WHERE `ID` = 900934;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectBonusMultiplier_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectAuraPeriod_2`, `EffectBonusMultiplier_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `SpellVisualID_1`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900934, 0, 1, 165, 13, -1, 2, 241, 889, 6, 0.571, 6, 1, 49, 6, 3, 1000, 0.024, 6, 156, 2, 3400, 'Holy Fire', 0x003F3F);
