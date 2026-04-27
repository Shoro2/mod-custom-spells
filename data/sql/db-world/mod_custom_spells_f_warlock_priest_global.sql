-- Warlock Affli: DoT procs 900800/900802 (DONE_PERIODIC=0x40000)
DELETE FROM `spell_proc` WHERE `SpellId` IN (900800, 900802);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900800, 0, 5, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 20, 2000, 0),
(900802, 0, 5, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- Warlock Affli: spell_dbc 900800-900803
DELETE FROM `spell_dbc` WHERE `ID` IN (900800, 900801, 900802, 900803);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900800, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Affl: DoT AoE', 0x003F3F),
(900801, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 0, 108, 0, 0, 0x2, 0, 0, 5, 313, 0, 0, 'Affl: Corruption +50%', 0x003F3F),
(900802, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Affl: DoT Spread', 0x003F3F),
(900803, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 15, 13, 0, 0, 0, 0, 0, 0, 5, 313, 32, 0, 'Shadow Eruption', 0x003F3F);

-- Warlock Demo: spell_dbc 900833-900844
DELETE FROM `spell_dbc` WHERE `ID` IN (900833, 900834, 900835, 900836, 900837, 900838, 900839, 900840, 900841, 900842, 900843, 900844);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900833, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Meta Kill Extend', 0x003F3F),
(900834, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 226, 0, 0, 0, 0, 3000, 5, 313, 0, 0, 'Demo: Meta AoE+Heal', 0x003F3F),
(900835, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Lesser Spawn', 0x003F3F),
(900836, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Imp FB +50%', 0x003F3F),
(900837, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Imp FB AoE', 0x003F3F),
(900838, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: FG Unlim', 0x003F3F),
(900839, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: FG +50%', 0x003F3F),
(900840, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Sacrifice All', 0x003F3F),
(900841, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 5, 313, 4, 0, 'Imp Firebolt Bounce', 0x003F3F),
(900842, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 600, 22, 13, 0, 0, 0, 0, 0, 0, 5, 313, 32, 0, 'Meta Shadow Burst', 0x003F3F),
(900843, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 100, 400, 1, 0, 0, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Meta Demon Heal', 0x003F3F),
(900844, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 5, 313, 1, 0, 'FG Cleave Hit', 0x003F3F);

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
DELETE FROM `spell_dbc` WHERE `ID` IN (900866, 900867, 900868, 900869, 900870, 900871, 900872);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900866, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x1, 0, 0, 5, 313, 0, 0, 'Destro: SB +9', 0x003F3F),
(900867, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 0, 5, 313, 0, 0, 'Destro: SB +50%', 0x003F3F),
(900868, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x1000000, 0, 5, 313, 0, 0, 'Destro: CB +50%', 0x003F3F),
(900869, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -2000, 1, 107, 11, 0, 0, 0x1000000, 0, 5, 313, 0, 0, 'Destro: CB CD -2s', 0x003F3F),
(900870, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0, 0x1000000, 0, 5, 313, 0, 0, 'Destro: CB +9', 0x003F3F),
(900871, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 5, 313, 32, 0, 'Shadow Bolt Bounce', 0x003F3F),
(900872, 0x10100000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 5, 313, 4, 0, 'Chaos Bolt Bounce', 0x003F3F);

-- Priest Disc: spell_dbc 900900-900903
DELETE FROM `spell_dbc` WHERE `ID` IN (900900, 900901, 900902, 900903);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900900, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Disc: Shield Explode', 0x003F3F),
(900901, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 0, 108, 0, 0, 0x1, 0, 0, 6, 566, 0, 0, 'Disc: Shield +50%', 0x003F3F),
(900903, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 15, 13, 0, 0, 0, 0, 0, 0, 6, 566, 2, 0, 'Shield Explosion', 0x003F3F);

-- Priest Holy: spell_dbc 900933
DELETE FROM `spell_dbc` WHERE `ID` = 900933;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900933, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Holy: Heal Fire', 0x003F3F);

-- Priest Shadow: spell_dbc 900966-900968
DELETE FROM `spell_dbc` WHERE `ID` IN (900966, 900967, 900968);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900966, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Shadow: DoT AoE', 0x003F3F),
(900967, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Shadow: DoT Spread', 0x003F3F),
(900968, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 15, 13, 0, 0, 0, 0, 0, 0, 6, 566, 32, 0, 'Shadow Eruption', 0x003F3F);

-- Priest: spell_proc - 900933 fixed to 0x4000 (DONE_SPELL_MAGIC_DMG_CLASS_POS for direct heals)
DELETE FROM `spell_proc` WHERE `SpellId` IN (900933, 900966, 900967);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900933, 0, 6, 0, 0, 0, 0x4000, 2, 2, 0, 0, 0, 0, 20, 3000, 0),
(900966, 0, 6, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 20, 2000, 0),
(900967, 0, 6, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- Global: spell_dbc 901100-901108
DELETE FROM `spell_dbc` WHERE `ID` IN (901100, 901101, 901102, 901103, 901104, 901105, 901106, 901107, 901108);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(901100, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Cast Moving', 0x003F3F),
(901101, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Kill Heal', 0x003F3F),
(901102, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Extra Attack', 0x003F3F),
(901103, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Cleave Proc', 0x003F3F),
(901104, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Counter', 0x003F3F),
(901105, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Kill Heal', 0x003F3F),
(901106, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Cleave Hit', 0x003F3F),
(901107, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Counter Strike', 0x003F3F),
(901108, 0x10000000, 0, 0, 0, 1, 0, 1, 32, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Extra Attack Hit', 0x003F3F);

-- Global: spell_proc - corrected ProcFlags (KILL=0x2, TAKEN_MELEE_AUTO=0x8)
DELETE FROM `spell_proc` WHERE `SpellId` IN (901101, 901102, 901103, 901104);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901101, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 100, 0, 0),
(901102, 0, 0, 0, 0, 0, 0x14, 0, 2, 0, 0, 0, 0, 25, 1000, 0),
(901103, 0, 0, 0, 0, 0, 0x10014, 1, 2, 0, 0, 0, 0, 10, 1000, 0),
(901104, 0, 0, 0, 0, 0, 0x8, 0, 0, 0, 0, 0, 0, 100, 2000, 0);
