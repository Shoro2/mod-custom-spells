-- Hunter Surv: Trap proc 900566 (DONE_RANGED_AUTO 0x40 | DONE_SPELL_RANGED 0x100 = 0x140)
DELETE FROM `spell_proc` WHERE `SpellId` = 900566;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900566, 0, 0, 0, 0, 0, 0x140, 1, 2, 0, 0, 0, 0, 15, 2000, 0);

-- Hunter: spell_dbc 900500-900567
DELETE FROM `spell_dbc` WHERE `ID` IN (900500, 900501, 900502, 900503, 900504, 900505, 900533, 900534, 900535, 900536, 900566, 900567);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900500, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Hunt: Get Back Arrows', 0x003F3F),
(900501, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Hunt: Multishot AoE', 0x003F3F),
(900502, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet Damage +50%', 0x003F3F),
(900503, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet Speed +50%', 0x003F3F),
(900504, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet AoE Proc', 0x003F3F),
(900505, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 15, 0, 0, 0, 0, 0, 0, 9, 132, 1, 0, 'Beast Cleave', 0x003F3F),
(900533, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'MM: Autoshot Bounce', 0x003F3F),
(900534, 0x10000000, 0, 0, 0, 1, 4, 1, 6, 0, 0, 1, 226, 0, 0, 0, 0, 100, 9, 132, 0, 0, 'MM: Barrage', 0x003F3F),
(900535, 0x10000000, 0, 0, 0, 1, 0, 4, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 9, 132, 1, 0, 'Ricochet Shot', 0x003F3F),
(900536, 0x10000000, 0, 0, 0, 1, 4, 1, 6, 0, -50, 1, 11, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Barrage Slow', 0x003F3F),
(900566, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Surv: Trap Proc', 0x003F3F),
(900567, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 1000, 15, 0, 0, 0, 0, 0, 0, 9, 132, 4, 0, 'Explosive Burst', 0x003F3F);

-- Druid Balance: Starfall CD reduce proc 901004
DELETE FROM `spell_proc` WHERE `SpellId` = 901004;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901004, 0, 7, 0, 0, 0, 0x10010, 1, 2, 0, 0, 0, 0, 100, 1000, 0);

-- Druid Resto: HoT->Treant proc 901066 (DONE_PERIODIC=0x40000)
DELETE FROM `spell_proc` WHERE `SpellId` = 901066;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901066, 0, 7, 0, 0, 0, 0x40000, 0, 0, 0, 0, 0, 0, 5, 5000, 0);

-- Druid: spell_dbc 901000-901073
DELETE FROM `spell_dbc` WHERE `ID` IN (901000, 901001, 901002, 901003, 901004, 901005, 901033, 901034, 901049, 901050, 901051, 901066, 901067, 901068, 901069, 901070, 901071, 901072, 901073);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(901000, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x2, 0, 0, 7, 132, 0, 0, 'Bal: MF +9 Targets', 0x003F3F),
(901001, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x2, 0, 0, 7, 132, 0, 0, 'Bal: MF +50%', 0x003F3F),
(901002, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x100, 0, 0, 7, 132, 0, 0, 'Bal: SF +9 Targets', 0x003F3F),
(901003, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x100, 0, 0, 7, 132, 0, 0, 'Bal: SF +50%', 0x003F3F),
(901004, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Bal: SF CD Reduce', 0x003F3F),
(901005, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 4, 0, 0x100, 0, 0, 7, 132, 0, 0, 'Bal: SF Stacks 10', 0x003F3F),
(901033, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Bear Bleed', 0x003F3F),
(901034, 0x10000000, 0, 0, 0, 1, 32, 1, 6, 50, 300, 6, 3, 0, 0, 0, 0, 3000, 7, 132, 1, 0, 'Swipe Bleed', 0x003F3F),
(901049, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Cat Bleed', 0x003F3F),
(901050, 0x10000000, 0, 0, 0, 1, 32, 1, 6, 50, 300, 6, 3, 0, 0, 0, 0, 3000, 7, 132, 1, 0, 'Rake Bleed', 0x003F3F),
(901051, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 110, 3, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Energy +50%', 0x003F3F),
(901066, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: HoT Treant', 0x003F3F),
(901067, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Summon Scale', 0x003F3F),
(901068, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Summon Heal', 0x003F3F),
(901069, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Thorns Rejuv', 0x003F3F),
(901070, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x30, 0, 0, 7, 132, 0, 0, 'Resto: HoTs +50%', 0x003F3F),
(901071, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 100, 1, 108, 17, 0, 0x30, 0, 0, 7, 132, 0, 0, 'Resto: HoTs 2x', 0x003F3F),
(901072, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Mana Regen', 0x003F3F),
(901073, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 500, 2000, 30, 0, 0, 0, 0, 0, 0, 7, 132, 8, 0, 'Nature Bloom', 0x003F3F);

-- Druid Resto Treant NPC 901066
DELETE FROM `creature_template` WHERE `entry` = 901066;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(901066, 0, 0, 0, 0, 0, 'Healing Treant', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 0.5, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.5, 0, 0.5, 0, 0, 0, 1, 0, 0, '', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 901066;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(901066, 0, 3889, 1, 1, 12340);

-- Rogue: Poison Nova proc 900603
DELETE FROM `spell_proc` WHERE `SpellId` = 900603;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900603, 8, 8, 0, 0, 0, 0x10000, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- Rogue: spell_dbc 900600-900669
DELETE FROM `spell_dbc` WHERE `ID` IN (900600, 900601, 900602, 900603, 900604, 900633, 900634, 900635, 900636, 900637, 900638, 900666, 900667, 900668, 900669);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900600, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Assa: Energy +50%', 0x003F3F),
(900601, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x200000, 0, 8, 132, 0, 0, 'Assa: Muti +50%', 0x003F3F),
(900602, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x8000, 0x10000, 0, 8, 132, 0, 0, 'Assa: Poison +50%', 0x003F3F),
(900603, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 8, 132, 0, 0, 'Assa: Poison Nova', 0x003F3F),
(900604, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 15, 0, 0, 0, 0, 0, 0, 8, 132, 8, 0, 'Poison Nova', 0x003F3F),
(900633, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x2, 0, 0, 8, 132, 0, 0, 'Combat: SS +50%', 0x003F3F),
(900634, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x2, 0, 0, 8, 132, 0, 0, 'Combat: SS +9 Targets', 0x003F3F),
(900635, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 105000, 1, 107, 17, 0, 0, 0x800, 0, 8, 132, 0, 0, 'Combat: BF 2min', 0x003F3F),
(900636, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0, 0x800, 0, 8, 132, 0, 0, 'Combat: BF +9 Targets', 0x003F3F),
(900637, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Combat: Energy +50%', 0x003F3F),
(900638, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 8, 132, 1, 0, 'Sinister Slash', 0x003F3F),
(900666, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Sub: Energy +50%', 0x003F3F),
(900667, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x2000000, 0, 0, 8, 132, 0, 0, 'Sub: Hemo +50%', 0x003F3F),
(900668, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x2000000, 0, 0, 8, 132, 0, 0, 'Sub: Hemo +9 Targets', 0x003F3F),
(900669, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 8, 132, 1, 0, 'Deep Cut', 0x003F3F);
