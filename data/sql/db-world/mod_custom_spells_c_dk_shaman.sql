-- DK Blood: Death Coil proc 900304
DELETE FROM `spell_proc` WHERE `SpellId` = 900304;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900304, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- DK Unholy: DoT->AoE proc 900366 (DONE_PERIODIC=0x40000)
DELETE FROM `spell_proc` WHERE `SpellId` = 900366;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900366, 0, 0, 0, 0, 0, 0x40000, 0, 0, 0, 0, 0, 0, 20, 2000, 0);

-- DK: spell_dbc 900300-900367
DELETE FROM `spell_dbc` WHERE `ID` IN (900300, 900301, 900302, 900303, 900304, 900305, 900333, 900366, 900367);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900300, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 3735, 0, 0, 'DKB: 3 Rune Weapons', 0x003F3F),
(900301, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 3735, 0, 0, 'DKB: Double Cast', 0x003F3F),
(900302, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x2000000, 0, 15, 3547, 0, 0, 'DKB: HS +50%', 0x003F3F),
(900303, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x2000000, 0, 15, 3547, 0, 0, 'DKB: HS +9 Targets', 0x003F3F),
(900304, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 136, 0, 0, 'DKB: DC Proc', 0x003F3F),
(900305, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 15, 3547, 1, 0, 'HS Bounce', 0x003F3F),
(900333, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 3223, 0, 0, 'DKF: Frost Wyrm', 0x003F3F),
(900366, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 2770, 0, 0, 'DKU: DoT AoE Proc', 0x003F3F),
(900367, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 150, 600, 15, 0, 0, 0, 0, 0, 15, 2770, 32, 0, 'Shadow Eruption', 0x003F3F);

-- 900368: Frost Breath
DELETE FROM `spell_dbc` WHERE `ID` = 900368;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectRadiusIndex_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectRadiusIndex_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900368, 0x10000000, 0, 0, 0, 16, 18, 4, 2, 1000, 5000, 24, 0, 0, 15, 6, 0, -50, 24, 11, 0, 15, 15, 3223, 16, 'Frost Breath', 0x003F3F);

-- DK Frost Wyrm NPC 900333
DELETE FROM `creature_template` WHERE `entry` = 900333;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(900333, 0, 0, 0, 0, 0, 'Frost Wyrm', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 1, 4, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 6, 12288, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 2, 1, 1, 0, 0, 0, 1, 0, 0, 'npc_custom_frost_wyrm', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 900333;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900333, 0, 26752, 1, 1, 12340);

-- Shaman Ele: Flame Shock proc 900405 (DONE_PERIODIC=0x40000)
DELETE FROM `spell_proc` WHERE `SpellId` = 900405;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900405, 0, 11, 0x10000000, 0, 0, 0x40000, 0, 0, 0, 0, 0, 0, 15, 2000, 0);

-- Shaman Ele: spell_dbc 900400-900408
DELETE FROM `spell_dbc` WHERE `ID` IN (900400, 900401, 900402, 900403, 900404, 900405, 900406, 900407, 900408);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900400, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 6, 1, 107, 17, 0, 0x2, 0, 11, 62, 0, 0, 'Ele: CL +6 Targets', 0x003F3F),
(900401, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Ele: Totem Follow', 0x003F3F),
(900402, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 547, 0, 0, 'Ele: Ragnaros', 0x003F3F),
(900403, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 2018, 0, 0, 'Ele: LO + LvB', 0x003F3F),
(900404, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 0, 'Ele: LvB Spread FS', 0x003F3F),
(900405, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 0, 'Ele: FS Reset LvB', 0x003F3F),
(900406, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 2, 'Ele: LvB Charges', 0x003F3F),
(900407, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -100, 1, 108, 14, 0, 0x1000, 0, 11, 16164, 0, 0, 'Ele: Instant LvB', 0x003F3F),
(900408, 0x10000000, 0, 0, 0, 1, 0, 4, 2, 0, 0, 6, 0, 0, 0, 0, 0, 11, 62, 8, 0, 'Chain Lightning Arc', 0x003F3F);

-- Shaman Enhance: Wolf summon proc 900436
DELETE FROM `spell_proc` WHERE `SpellId` = 900436;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900436, 0, 0, 0, 0, 0, 0x4, 0, 0, 0, 0, 0, 0, 10, 5000, 0);

-- Shaman Enhance + Resto: spell_dbc
DELETE FROM `spell_dbc` WHERE `ID` IN (900433, 900434, 900435, 900436, 900437, 900438, 900439, 900440, 900466, 900467);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900433, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Enh: Totem Follow', 0x003F3F),
(900434, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Maelstrom AoE', 0x003F3F),
(900435, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Enh: Summons +50%', 0x003F3F),
(900436, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Wolf Summon', 0x003F3F),
(900437, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Wolf Haste', 0x003F3F),
(900438, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 62, 0, 0, 'Enh: Wolf CL', 0x003F3F),
(900439, 0x10000000, 0, 0, 0, 1, 18, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Maelstrom Fury', 0x003F3F),
(900440, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 22, 0, 0, 0, 0, 0, 11, 3786, 1, 0, 'Spirit Howl', 0x003F3F),
(900466, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Resto: Totem Follow', 0x003F3F),
(900467, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Resto: Mana Regen', 0x003F3F);

-- Spirit Wolf NPC 900436
DELETE FROM `creature_template` WHERE `entry` = 900436;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(900436, 0, 0, 0, 0, 0, 'Spirit Wolf', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 0.5, 1500, 2000, 1, 1, 1, 0, 2048, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.5, 0, 0.5, 0, 0, 0, 1, 0, 0, '', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 900436;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900436, 0, 27074, 1, 1, 12340);
