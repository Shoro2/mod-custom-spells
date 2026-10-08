-- Hunter Surv: explosive trap proc 900566 (revision 2026-10-08: any Physical
-- damage the hunter deals). ProcFlags DONE_MELEE_AUTO 0x4 | DONE_SPELL_MELEE 0x10
-- | DONE_RANGED_AUTO 0x40 | DONE_SPELL_RANGED 0x100 = 0x154, SchoolMask 1
-- (Physical), PROC_ATTR_TRIGGERED_CAN_PROC (triggered attacks such as Barrage's
-- Multi-Shots count), 15 %, 2 s cooldown. The C++ drops the Explosive Trap Effect
-- (not the old helper 900567) at the target.
DELETE FROM `spell_proc` WHERE `SpellId` = 900566;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900566, 1, 0, 0, 0, 0, 0x154, 1, 2, 0, 0x2, 0, 0, 15, 2000, 0);

-- Hunter: spell_dbc 900500-900567
-- 900505 Beast Cleave: the C++ passes 666 + 5 x the hunter's Paragon level
-- (revision 2026-10-08); the row's 666 is the value without a Paragon level.
-- 900567 Explosive Burst is retired (900566 sets off the Explosive Trap Effect
-- since the revision); the DELETE below still clears its row from live DBs.
DELETE FROM `spell_dbc` WHERE `ID` IN (900500, 900501, 900502, 900503, 900504, 900505, 900506, 900533, 900534, 900535, 900536, 900566, 900567);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `RecoveryTime`) VALUES
(900500, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Hunt: Get Back Arrows', 0x003F3F, 0),
(900501, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Hunt: Multishot AoE', 0x003F3F, 0),
(900502, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet Damage +50%', 0x003F3F, 0),
(900503, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet Speed +50%', 0x003F3F, 0),
(900504, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet AoE Proc', 0x003F3F, 0),
(900505, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 0, 0, 0, 0, 9, 132, 1, 0, 'Beast Cleave', 0x003F3F, 0),
(900506, 0, 0, 0, 0, 1, 0, 4, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 9, 132, 1, 0, 'Multishot Bounce', 0x003F3F, 0),
(900533, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'MM: Autoshot Bounce', 0x003F3F, 0),
(900534, 0, 0, 0, 0, 1, 39, 1, -1, 6, 0, 0, 1, 0, 226, 0, 0, 0, 0, 100, 9, 132, 0, 0, 'MM: Barrage', 0x003F3F, 30000),
(900535, 0, 0, 0, 0, 1, 0, 4, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 9, 132, 1, 0, 'Ricochet Shot', 0x003F3F, 0),
(900536, 0, 0, 0, 0, 1, 39, 1, -1, 6, 0, -50, 1, 0, 33, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Barrage Slow', 0x003F3F, 0),
(900566, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Surv: Trap Proc', 0x003F3F, 0);

-- Druid Balance: Starfall CD reduce proc 901004
DELETE FROM `spell_proc` WHERE `SpellId` = 901004;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901004, 0, 7, 0, 0, 0, 0x10010, 1, 2, 0, 0, 0, 0, 100, 1000, 0);

-- Druid Resto: HoT->Treant proc 901066 (DONE_PERIODIC=0x40000)
DELETE FROM `spell_proc` WHERE `SpellId` = 901066;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901066, 0, 7, 0, 0, 0, 0x40000, 0, 2, 0, 0, 0, 0, 5, 5000, 0);

-- Druid: spell_dbc 901000-901073 (901002 has its own statement in the revision
-- section below: its modifier needs effect 1's third mask word)
-- Masks verified against Spell.dbc SpellFamilyFlags: Moonfire flags0=0x2,
-- Starfire flags0=0x4 (not 0x100), Rejuvenation 0x10 + Regrowth 0x40 = 0x50
-- (0x20 is Healing Touch, not a HoT). 901071 uses SPELLMOD_DOT (22).
DELETE FROM `spell_dbc` WHERE `ID` IN (901000, 901001, 901003, 901004, 901005, 901033, 901034, 901049, 901050, 901051, 901066, 901067, 901068, 901069, 901070, 901071, 901072, 901073);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `Effect_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskB_2`) VALUES
(901000, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 0, 107, 17, 0, 0x2, 0, 0, 7, 132, 0, 0, 'Bal: MF +9 Targets', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901001, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x2, 0x2, 0, 7, 132, 0, 0, 'Bal: MF +50%', 0x003F3F, 6, 108, 22, 50, 1, 0, 0),
(901003, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Bal: SF +50%', 0x003F3F, 0, 0, 0, 0, 0, 0x800000, 0),
(901004, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Bal: SF CD Reduce', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901005, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Bal: SF Stacks 10', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901033, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Bear Bleed', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901034, 0, 0, 0, 0, 1, 32, 1, -1, 6, 50, 300, 6, 0, 3, 0, 0, 0, 0, 3000, 7, 132, 1, 0, 'Swipe Bleed', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901049, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Cat Bleed', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901050, 0, 0, 0, 0, 1, 32, 1, -1, 6, 50, 300, 6, 0, 3, 0, 0, 0, 0, 3000, 7, 132, 1, 0, 'Rake Bleed', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901051, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 110, 3, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Energy +50%', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901066, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: HoT Treant', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901067, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Summon Scale', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901068, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Summon Heal', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901069, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Thorns Rejuv', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901070, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 22, 0, 0x50, 0, 0, 7, 132, 0, 0, 'Resto: HoTs +50%', 0x003F3F, 0, 0, 0, 0, 0, 0x4000010, 0),
(901071, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, -50, 1, 0, 108, 19, 0, 0x50, 0x50, 0, 7, 132, 0, 0, 'Resto: HoTs 2x', 0x003F3F, 6, 108, 1, 100, 1, 0x4000010, 0x4000010),
(901072, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Mana Regen', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(901073, 0, 0, 0, 0, 1, 0, 1, -1, 10, 500, 2000, 30, 13, 0, 0, 0, 0, 0, 0, 7, 132, 8, 0, 'Nature Bloom', 0x003F3F, 0, 0, 0, 0, 0, 0, 0);

-- Druid Resto Treant NPC 901066
DELETE FROM `creature_template` WHERE `entry` = 901066;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(901066, 0, 0, 0, 0, 0, 'Healing Treant', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 0.5, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.5, 0, 0.5, 0, 0, 0, 1, 0, 0, '', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 901066;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(901066, 0, 3889, 1, 1, 12340);

-- Rogue: Poison Nova proc 900603 (revision 2026-10-08: dealing damage with a
-- poison, 5 %, no cooldown). Rogue family flags0 Instant 0x2000 | Deadly 0x10000
-- | Wound 0x10000000 = 0x10012000 (the poisons only), Nature; ProcFlags
-- DONE_SPELL_MAGIC_DMG_CLASS_NEG 0x10000 (the poison hits) | DONE_PERIODIC
-- 0x40000 (Deadly Poison's ticks); PROC_ATTR_TRIGGERED_CAN_PROC: weapon poisons
-- are triggered casts. The nova's own damage matches neither the mask nor the
-- triggered rule, so a nova never sets off the next one.
DELETE FROM `spell_proc` WHERE `SpellId` = 900603;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900603, 8, 8, 0x10012000, 0, 0, 0x50000, 1, 2, 0, 0x2, 0, 0, 5, 0, 0);

-- Rogue: spell_dbc 900600-900669
-- Masks verified against Spell.dbc SpellFamilyFlags: Mutilate flags1=0x200000
-- (goes in A_2), poisons flags0 Instant 0x2000 | Deadly 0x10000 | Wound
-- 0x10000000 = 0x10012000, Sinister Strike flags0=0x2, Blade Flurry
-- flags1=0x800, Hemorrhage flags0=0x2800000 (0x2000000 overlaps, kept).
-- 900635 uses SPELLMOD_DURATION (1): +105s on the 15s base = 2 minutes.
-- 900604 Poison Nova: the C++ passes 666 + 5 x the rogue's Paragon level
-- (revision 2026-10-08); the row's 666 is the value without a Paragon level.
DELETE FROM `spell_dbc` WHERE `ID` IN (900600, 900601, 900602, 900603, 900604, 900633, 900634, 900635, 900636, 900637, 900638, 900666, 900667, 900668, 900669);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `Effect_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectSpellClassMaskB_1`) VALUES
(900600, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Assa: Energy +50%', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900601, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0, 0x200006, 0, 8, 132, 0, 0, 'Assa: Muti +50%', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900602, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x10012000, 0, 0, 8, 132, 0, 0, 'Assa: Poison +50%', 0x003F3F, 6, 108, 22, 50, 1, 0x10012000),
(900603, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 8, 132, 0, 0, 'Assa: Poison Nova', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900604, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 0, 0, 0, 0, 8, 132, 8, 0, 'Poison Nova', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900633, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x2, 0, 0, 8, 132, 0, 0, 'Combat: SS +50%', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900634, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 0, 107, 17, 0, 0x2, 0, 0, 8, 132, 0, 0, 'Combat: SS +9 Targets', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900635, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 105000, 1, 0, 107, 1, 0, 0, 0x800, 0, 8, 132, 0, 0, 'Combat: BF 2min', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900636, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 8, 132, 0, 0, 'Combat: BF +9 Targets', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900637, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Combat: Energy +50%', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900638, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 8, 132, 1, 0, 'Sinister Slash', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900666, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Sub: Energy +50%', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900667, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x2000000, 0, 0, 8, 132, 0, 0, 'Sub: Hemo +50%', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900668, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 0, 107, 17, 0, 0x2000000, 0, 0, 8, 132, 0, 0, 'Sub: Hemo +9 Targets', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900669, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 8, 132, 1, 0, 'Deep Cut', 0x003F3F, 0, 0, 0, 0, 0, 0);

-- Druid Feral (tank) concept spell built 2026-10-08: 901035 Maul +50 % on a
-- bleeding target (C++ on Maul, AURA_STATE_BLEEDING).
DELETE FROM `spell_dbc` WHERE `ID` = 901035;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `SpellClassSet`, `SpellIconID`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(901035, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 4, 7, 261, 'Maul: Bleeding Wounds', 0x003F3F);

-- Druid Resto helpers (2026-10-08): 901074 the summon health bonus 901067 grants
-- (aura 34 MOD_INCREASE_HEALTH, its amount = 10 x the druid's healing power, kept
-- current by the shared minion manager), 901075 the Parting Bloom fuse 901068 puts
-- on a timed summon (ends 0.3 s before the summon despawns; its expiry heals).
DELETE FROM `spell_dbc` WHERE `ID` IN (901074, 901075);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `SpellClassSet`, `SpellIconID`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(901074, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 34, 7, 1679, 'Nature''s Bond', 0x003F3F),
(901075, 0x80, 1, 21, 1, -1, 6, 0, 0, 1, 4, 7, 1679, 'Parting Bloom Fuse', 0x003F3F);

-- ============================================================================
-- Concept revision of 2026-10-08 (second list; share-public custom-spells/
-- 09-concept-revision-20261008.md). Passives: dummy markers (aura 4) the C++
-- gates on, or spell modifiers (107 flat / 108 pct) with the effect's own mask
-- words (A_1..A_3 = effect 1). "+9 targets" on a single-target spell is jump
-- target value 10. Damage helpers: EffectBasePoints is the value without a
-- Paragon level (666), the C++ passes 666 + 5 x the Paragon level.
-- ============================================================================

-- Hunter BM 900507 Pet Health +100 %: the passive is listed in spell_pet_auras,
-- so the core keeps 900508 (aura 133 MOD_INCREASE_HEALTH_PERCENT +100, passive)
-- on the hunter's pet for as long as the hunter has the passive.
-- Hunter MM 900537 Multi-Shot -> Serpent Sting (C++), 900538 / 900539 Chimera
-- Shot +9 targets / +50 % (flags2 0x1 -> A_3), 900540 Running Aim (C++) and its
-- buff 900541 (SPELLMOD_DAMAGE +50 % on Aimed Shot flags0 0x20000 per stack, 5
-- stacks, until the next Aimed Shot).
-- Hunter Surv 900568 Blade and Shot (C++ proc) with 900569 Blade Readiness / 900570
-- Shot Readiness (aura 79 +5 % Physical per stack, 10 stacks, 30 s; weapon-gated:
-- EquippedItemClass 2 with the melee subclasses 0xA5F3 = axe, 2h axe, mace, 2h
-- mace, polearm, sword, 2h sword, staff, fist, dagger / the ranged ones 0x5000C =
-- bow, gun, thrown, crossbow - the core applies such an aura only to the weapon
-- damage of attacks with a fitting weapon), 900571 / 900572 Raptor Strike +9
-- targets / +50 % (flags2 0x10000 -> A_3; flags0 0x2 is shared with Mongoose Bite).
DELETE FROM `spell_dbc` WHERE `ID` IN (900507, 900508, 900537, 900538, 900539, 900540, 900541, 900568, 900569, 900570, 900571, 900572);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `EquippedItemSubclass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_3`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900507, 0x10000040, 0x10000000, 1, 21, 1, -1, 0, 6, 0, 0, 1, 4, 0, 0, 0, 9, 960, 0, 0, 'BM: Pet Health +100%', 0x003F3F),
(900508, 0x40, 0, 1, 21, 1, -1, 0, 6, 0, 100, 1, 133, 0, 0, 0, 9, 960, 0, 0, 'Pet Vitality', 0x003F3F),
(900537, 0x10000040, 0x10000000, 1, 21, 1, -1, 0, 6, 0, 0, 1, 4, 0, 0, 0, 9, 536, 0, 0, 'MM: Multi-Shot Serpent Sting', 0x003F3F),
(900538, 0x10000040, 0x10000000, 1, 21, 1, -1, 0, 6, 0, 10, 1, 107, 17, 0, 0x1, 9, 3412, 0, 0, 'MM: Chimera +9 Targets', 0x003F3F),
(900539, 0x10000040, 0x10000000, 1, 21, 1, -1, 0, 6, 0, 50, 1, 108, 0, 0, 0x1, 9, 3412, 0, 0, 'MM: Chimera +50%', 0x003F3F),
(900540, 0x10000040, 0x10000000, 1, 21, 1, -1, 0, 6, 0, 0, 1, 4, 0, 0, 0, 9, 1629, 0, 0, 'MM: Running Aim', 0x003F3F),
(900541, 0, 0, 1, 21, 1, -1, 0, 6, 0, 50, 1, 108, 0, 0x20000, 0, 9, 1629, 0, 5, 'Running Aim', 0x003F3F),
(900568, 0x10000040, 0x10000000, 1, 21, 1, -1, 0, 6, 0, 0, 1, 4, 0, 0, 0, 9, 26, 0, 0, 'Surv: Blade and Shot', 0x003F3F),
(900569, 0, 0, 1, 9, 1, 2, 0xA5F3, 6, 0, 5, 1, 79, 1, 0, 0, 9, 26, 0, 10, 'Blade Readiness', 0x003F3F),
(900570, 0, 0, 1, 9, 1, 2, 0x5000C, 6, 0, 5, 1, 79, 1, 0, 0, 9, 85, 0, 10, 'Shot Readiness', 0x003F3F),
(900571, 0x10000040, 0x10000000, 1, 21, 1, -1, 0, 6, 0, 10, 1, 107, 17, 0, 0x10000, 9, 26, 0, 0, 'Surv: Raptor +9 Targets', 0x003F3F),
(900572, 0x10000040, 0x10000000, 1, 21, 1, -1, 0, 6, 0, 50, 1, 108, 0, 0, 0x10000, 9, 26, 0, 0, 'Surv: Raptor +50%', 0x003F3F);

DELETE FROM `spell_pet_auras` WHERE `spell` = 900507;
INSERT INTO `spell_pet_auras` (`spell`, `effectId`, `pet`, `aura`) VALUES
(900507, 0, 0, 900508);

-- procs: 900568 Blade and Shot (Physical damage done by melee and ranged attacks
-- and abilities, triggered ones included, every hit)
DELETE FROM `spell_proc` WHERE `SpellId` = 900568;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900568, 1, 0, 0, 0, 0, 0x154, 1, 2, 0, 0x2, 0, 0, 100, 0, 0);

-- Druid Balance 901002 Starfall area +50 % (was "+9 targets"; the id stays, cursed
-- items reference it): SPELLMOD_RADIUS (6) +50 % on the volley trigger's 30-yd
-- search (53196-53198, flags2 0x100 -> A_3) and the stars' 5-yd splash (50294,
-- 53188-53190, flags1 0x800000 -> A_2; the bit also marks Starfall and the stars,
-- which have no radius). 901006 Lunar Frenzy (C++ kill proc) and its buff 901007
-- (+1 % damage done, +1 % casting speed per stack, 100 stacks, 60 s; its effect 3
-- ticks every 3 s for the Starfire volley).
-- Druid Feral tank 901036 Swipe (Bear) +50 % (flags1 0x100000 -> A_2), 901037 Thorns:
-- Open Wounds (C++) and its strike 901038 (Physical, a bleed: no armor reduction;
-- any range, no line of sight needed), 901039 Ursine Bulwark (C++ proc on hits
-- taken) with its buff 901040 (+1 % maximum health per stack, 100 stacks, 30 s;
-- effect 2 ticks every second for the Barkskin upkeep) and the rage burst 901041
-- (Physical, 8 yd round the druid).
-- Druid Feral DPS 901052 Swipe (Cat) +50 % (flags2 0x400 -> A_3), 901053 Frontal
-- Assault and 901054 Berserk: Unleashed (markers for the shared rogue code), 901055
-- the speed aura (+100 %), 901056 the finisher burst (Physical, 8 yd).
DELETE FROM `spell_dbc` WHERE `ID` IN (901002, 901006, 901007, 901036, 901037, 901038, 901039, 901040, 901041, 901052, 901053, 901054, 901055, 901056);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `EffectMechanic_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `Effect_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectAuraPeriod_2`, `Effect_3`, `ImplicitTargetA_3`, `EffectAura_3`, `EffectAuraPeriod_3`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(901002, 0x10000040, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 0, 1, 0, 108, 6, 0, 0x800000, 0x100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Bal: SF +50% Area', 0x003F3F),
(901006, 0x10000040, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 111, 0, 0, 'Bal: Lunar Frenzy', 0x003F3F),
(901007, 0, 0, 0, 1, 3, 1, -1, 6, 0, 1, 0, 1, 0, 79, 127, 0, 0, 0, 6, 1, 1, 65, 0, 6, 1, 226, 3000, 7, 111, 0, 100, 'Lunar Frenzy', 0x003F3F),
(901036, 0x10000040, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 0, 1, 0, 108, 0, 0, 0x100000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1562, 0, 0, 'Feral: Swipe Bear +50%', 0x003F3F),
(901037, 0x10000040, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 53, 0, 0, 'Feral: Thorns Open Wounds', 0x003F3F),
(901038, 0, 0x4, 0, 1, 0, 13, -1, 2, 0, 666, 15, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 53, 1, 0, 'Open Wound', 0x003F3F),
(901039, 0x10000040, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 689, 0, 0, 'Feral: Ursine Bulwark', 0x003F3F),
(901040, 0, 0, 0, 1, 9, 1, -1, 6, 0, 1, 0, 1, 0, 133, 0, 0, 0, 0, 6, 0, 1, 226, 1000, 0, 0, 0, 0, 7, 689, 0, 100, 'Ursine Bulwark', 0x003F3F),
(901041, 0, 0, 0, 1, 0, 1, -1, 2, 0, 666, 0, 15, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1562, 1, 0, 'Ursine Quake', 0x003F3F),
(901052, 0x10000040, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 0, 1, 0, 108, 0, 0, 0, 0x400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1562, 0, 0, 'Feral: Swipe Cat +50%', 0x003F3F),
(901053, 0x10000040, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 147, 0, 0, 'Feral: Frontal Assault', 0x003F3F),
(901054, 0x10000040, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2852, 0, 0, 'Feral: Berserk Unleashed', 0x003F3F),
(901055, 0, 0, 0, 1, 8, 1, -1, 6, 0, 100, 0, 1, 0, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2852, 0, 0, 'Unleashed Speed', 0x003F3F),
(901056, 0, 0, 0, 1, 0, 1, -1, 2, 0, 666, 0, 15, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2852, 1, 0, 'Unleashed Fury', 0x003F3F);

-- The Feral bleeds 901034 (Swipe Bleed) and 901050 (Rake Bleed) carry the bleed
-- mechanic like Lacerate and Rake (EffectMechanic 15): only then does the target
-- count as bleeding (AURA_STATE_BLEEDING) for Maul: Bleeding Wounds 901035 and
-- Thorns: Open Wounds 901037, and armor no longer reduces their ticks.
UPDATE `spell_dbc` SET `EffectMechanic_1` = 15 WHERE `ID` IN (901034, 901050);

-- procs: 901006 Lunar Frenzy (every killing blow; the C++ checks Moonkin Form),
-- 901039 Ursine Bulwark (hits taken: melee 0x8 | melee spell 0x20 | ranged 0x80 |
-- ranged spell 0x200 | other harmful spell 0x2000 | magic spell 0x20000 = 0x222A8,
-- landed hits only, triggered ones included; the C++ checks Bear Form)
DELETE FROM `spell_proc` WHERE `SpellId` IN (901006, 901039);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901006, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 100, 0, 0),
(901039, 0, 0, 0, 0, 0, 0x222A8, 1, 0, 0, 0x2, 0, 0, 100, 0, 0);

-- Rogue Assa 900605 Mutilate +9 targets (flags1 0x6 -> A_2: the two hand strikes
-- 48665 / 48664 chain, not Mutilate 48666 - a strike triggered on a far chain
-- target of Mutilate fails its melee range check).
-- Rogue Combat 900639 Adrenaline Rush: Unleashed (marker for the shared code),
-- 900640 its speed aura (+100 %), 900641 the finisher burst (Physical, 8 yd),
-- 900642 Killing Spree: Fan of Knives (C++).
-- Rogue Sub 900670 / 900671 Ambush +9 targets / +50 % (flags0 0x200 -> A_1;
-- 0x800000 is shared with Backstab), 900672 Shadow Dance: Flow (C++) and its aura
-- 900673 (aura 72 power cost -100 % for every school; aura 107 SPELLMOD_GLOBAL_COOLDOWN
-- (21) flat -500 ms on every rogue spell, effect 2's masks B_1..B_3 - a rogue's global
-- cooldown is 1 s, so -50 %. Not a percent modifier: the core applies a percent
-- global cooldown modifier only to a cast whose cast time the same aura already
-- changed (its Backdraft rule in Player::ApplySpellMod), so -50 % would never
-- apply), 900674 Frontal Assault (marker for the shared code).
DELETE FROM `spell_dbc` WHERE `ID` IN (900605, 900639, 900640, 900641, 900642, 900670, 900671, 900672, 900673, 900674);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `Effect_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectSpellClassMaskB_1`, `EffectSpellClassMaskB_2`, `EffectSpellClassMaskB_3`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900605, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 0, 107, 17, 0, 0x6, 0, 0, 0, 0, 0, 0, 0, 0, 8, 2117, 0, 'Assa: Mutilate +9 Targets', 0x003F3F),
(900639, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 235, 0, 'Combat: Adrenaline Unleashed', 0x003F3F),
(900640, 0, 0, 1, 8, 1, -1, 6, 0, 100, 1, 0, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 235, 0, 'Adrenaline Sprint', 0x003F3F),
(900641, 0, 0, 1, 0, 1, -1, 2, 0, 666, 15, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 235, 1, 'Adrenaline Fury', 0x003F3F),
(900642, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 2907, 0, 'Combat: Spree of Knives', 0x003F3F),
(900670, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 0, 107, 17, 0x200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 856, 0, 'Sub: Ambush +9 Targets', 0x003F3F),
(900671, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0x200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 856, 0, 'Sub: Ambush +50%', 0x003F3F),
(900672, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 2959, 0, 'Sub: Shadow Dance Flow', 0x003F3F),
(900673, 0, 0, 1, 32, 1, -1, 6, 0, -100, 1, 0, 72, 127, 0, 0, 6, -500, 1, 107, 21, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 8, 2959, 0, 'Shadow Flow', 0x003F3F),
(900674, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 243, 0, 'Sub: Frontal Assault', 0x003F3F);
