-- DK Blood: Death Coil proc 900304
DELETE FROM `spell_proc` WHERE `SpellId` = 900304;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900304, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- DK Unholy: DoT->AoE proc 900366 (DONE_PERIODIC=0x40000)
DELETE FROM `spell_proc` WHERE `SpellId` = 900366;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900366, 0, 0, 0, 0, 0, 0x40000, 0, 2, 0, 0, 0, 0, 20, 2000, 0);

-- DK: spell_dbc 900300-900367
DELETE FROM `spell_dbc` WHERE `ID` IN (900300, 900301, 900302, 900303, 900304, 900305, 900333, 900366, 900367);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900300, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 15, 3735, 0, 0, 'DKB: 3 Rune Weapons', 0x003F3F),
(900301, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 15, 3735, 0, 0, 'DKB: Double Cast', 0x003F3F),
(900302, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x1000000, 0, 15, 3547, 0, 0, 'DKB: HS +50%', 0x003F3F),
(900303, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 9, 1, 0, 107, 17, 0, 0x1000000, 0, 15, 3547, 0, 0, 'DKB: HS +9 Targets', 0x003F3F),
(900304, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 15, 136, 0, 0, 'DKB: DC Proc', 0x003F3F),
(900305, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 15, 3547, 1, 0, 'HS Bounce', 0x003F3F),
(900333, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 15, 3223, 0, 0, 'DKF: Frost Wyrm', 0x003F3F),
(900366, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 15, 2770, 0, 0, 'DKU: DoT AoE Proc', 0x003F3F),
(900367, 0, 0, 0, 0, 1, 0, 1, -1, 2, 150, 600, 16, 13, 0, 0, 0, 0, 0, 15, 2770, 32, 0, 'Shadow Eruption', 0x003F3F);

-- DK concept spells built 2026-10-08 (operator: "ja neue spells im anschluss bauen").
-- Markers 900306 Bloodworm Burst, 900334/900335 Obliterate +50 % / +9 targets
-- (flags1 0x20000, value 10: Obliterate has no chain of its own), 900336 Howling
-- Blast +50 % (flags1 0x2), 900337 Howling Blast -> Frost Fever, 900338 Lichborne
-- leech, 900340 Improved Icy Talons -> Frozen Strikes, 900369 Ghoul Cleave,
-- 900371 Risen Army (proc -> 900372: one Army of the Dead ghoul for 5 s, the
-- summon of 42651 with its SummonProperties 687), 900373 Death and Decay around
-- you. Helpers: 900307 burst (dest 10 yd, Shadow), 900308 fuse (hidden, 19 s <
-- the worm's 20 s), 900339 leech buff (10 s = Lichborne), 900341 Frozen Strikes
-- (on a party member, TargetA 21), 900342 Frost hit, 900370 cleave (dest 5 yd),
-- 900374 mobile Death and Decay ticker (1 s, 10 s).
DELETE FROM `spell_dbc` WHERE `ID` IN (900306, 900307, 900308, 900334, 900335, 900336, 900337, 900338, 900339, 900340, 900341, 900342, 900369, 900370, 900371, 900372, 900373, 900374);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectMiscValueB_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `SpellVisualID_1`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900306, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 15, 1987, 0, 0, 'Bloodworm Burst', 0x003F3F),
(900307, 0, 1, 0, 13, -1, 2, 0, 666, 16, 13, 0, 0, 0, 0, 0, 0, 15, 1987, 32, 0, 'Bloodworm Burst', 0x003F3F),
(900308, 0x80, 1, 804, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 15, 1987, 0, 0, 'Bloodworm Fuse', 0x003F3F),
(900334, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0, 0, 0x20000, 15, 2639, 0, 0, 'Obliterate: +50% Damage', 0x003F3F),
(900335, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 0, 107, 17, 0, 0, 0, 0x20000, 15, 2639, 0, 0, 'Obliterate: +9 Targets', 0x003F3F),
(900336, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0, 0, 0x2, 15, 2131, 0, 0, 'Howling Blast: +50% Damage', 0x003F3F),
(900337, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 15, 3142, 0, 0, 'Howling Blast: Frost Fever', 0x003F3F),
(900338, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 15, 61, 0, 0, 'Lichborne: Leech', 0x003F3F),
(900339, 0, 1, 1, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 15, 61, 32, 0, 'Lichborne Leech', 0x003F3F),
(900340, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 15, 3785, 0, 0, 'Icy Talons: Frozen Strikes', 0x003F3F),
(900341, 0, 1, 21, 13, -1, 6, 0, 0, 21, 0, 4, 0, 0, 0, 0, 0, 15, 3785, 16, 0, 'Frozen Strikes', 0x003F3F),
(900342, 0, 1, 0, 13, -1, 2, 0, 666, 6, 0, 0, 0, 0, 0, 0, 0, 15, 3785, 16, 0, 'Frozen Strike', 0x003F3F),
(900369, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 15, 2718, 0, 0, 'Ghoul Cleave', 0x003F3F),
(900370, 0, 1, 0, 13, -1, 2, 0, 1, 16, 8, 0, 0, 0, 0, 0, 0, 15, 2718, 1, 0, 'Ghoul Cleave', 0x003F3F),
(900371, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 42, 0, 0, 900372, 0, 0, 15, 2718, 0, 0, 'Risen Army', 0x003F3F),
(900372, 0, 1, 28, 1, -1, 28, 0, 0, 72, 15, 0, 24207, 687, 0, 0, 0, 15, 2718, 32, 0, 'Risen Army', 0x003F3F),
(900373, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 15, 118, 0, 0, 'Death and Decay: Around You', 0x003F3F),
(900374, 0, 1, 1, 1, -1, 6, 0, 0, 1, 0, 226, 0, 0, 0, 0, 0, 15, 118, 32, 0, 'Death and Decay', 0x003F3F);
UPDATE `spell_dbc` SET `EffectAuraPeriod_1` = 1000 WHERE `ID` = 900374;

-- procs: 900339 leech (damage done: melee, ranged, spells, ticks), 900341 Frozen
-- Strikes (melee hits 10 %), 900371 Risen Army (melee abilities 10 %)
DELETE FROM `spell_proc` WHERE `SpellId` IN (900339, 900341, 900371);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900339, 0, 0, 0, 0, 0, 0x50154, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(900341, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 10, 0, 0),
(900371, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 10, 0, 0);

-- 900368: Frost Breath
DELETE FROM `spell_dbc` WHERE `ID` = 900368;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectRadiusIndex_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectRadiusIndex_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900368, 0, 0, 0, 0, 16, 18, 4, -1, 2, 1000, 5000, 24, 0, 0, 15, 6, 0, -50, 24, 11, 0, 15, 15, 3223, 16, 'Frost Breath', 0x003F3F);

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
(900405, 0, 11, 0x10000000, 0, 0, 0x40000, 0, 2, 0, 0, 0, 0, 15, 2000, 0);

-- Shaman Ele: spell_dbc 900400-900408
-- Masks verified against Spell.dbc SpellFamilyFlags: Chain Lightning flags0=0x2,
-- Lava Burst flags1=0x1000 (goes in A_2). 900407 is a dummy marker since
-- 2026-10-08 (concept: instant only while Clearcasting): the Clearcasting
-- AuraScript adds the hidden helper 900409 (SPELLMOD_CASTING_TIME -100 %).
-- 900400 also lifts Chain Lightning's per-jump falloff (concept: "+6 targets and no
-- damage reduce"): effect 2 = SPELLMOD_DAMAGE_MULTIPLIER (20) +43 % on the
-- 0.7 jump multiplier (0.7 x 1.43 = 1.0).
DELETE FROM `spell_dbc` WHERE `ID` IN (900400, 900401, 900402, 900403, 900404, 900405, 900406, 900407, 900408, 900409);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectSpellClassMaskB_1`) VALUES
(900400, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 6, 1, 107, 17, 0, 0x2, 0, 11, 62, 0, 0, 'Ele: CL +6 Targets', 0x003F3F, 6, 0, 43, 1, 108, 20, 0x2),
(900401, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Ele: Totem Follow', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900402, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 547, 0, 0, 'Ele: Ragnaros', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900403, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 2018, 0, 0, 'Ele: LO + LvB', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900404, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 0, 'Ele: LvB Spread FS', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900405, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 0, 'Ele: FS Reset LvB', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900406, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 2, 'Ele: LvB Charges', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900407, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3064, 0, 0, 'Ele: Instant LvB', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900408, 0, 0, 0, 0, 1, 0, 4, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 11, 62, 8, 0, 'Chain Lightning Arc', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900409, 0x40, 0, 0, 0, 1, 21, 1, -1, 6, 0, -100, 1, 108, 10, 0, 0, 0x1000, 11, 3064, 0, 0, 'Lava Burst: Instant', 0x003F3F, 0, 0, 0, 0, 0, 0, 0);

-- Shaman Elemental concept spells built 2026-10-08: Lightning Bolt +50 % / +9
-- targets (flags0 0x1, value 10: Lightning Bolt has no chain of its own), Chain
-- Lightning +50 % (flags0 0x2), Lava Burst +50 % / +9 targets (flags1 0x1000),
-- 900415 Elemental Resonance (proc: Nature damage -> 900416 +2 % Fire, Fire
-- damage -> 900417 +2 % Nature, 5 stacks, 10 s each), 900418 Lightning Shield
-- -> Chain Lightning (20 %, C++ on the shield's damage spells).
DELETE FROM `spell_dbc` WHERE `ID` IN (900410, 900411, 900412, 900413, 900414, 900415, 900416, 900417, 900418);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900410, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0x1, 0, 11, 62, 0, 0, 'Lightning Bolt: +50% Damage', 0x003F3F),
(900411, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0x1, 0, 11, 62, 0, 0, 'Lightning Bolt: +9 Targets', 0x003F3F),
(900412, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0x2, 0, 11, 165, 0, 0, 'Chain Lightning: +50% Damage', 0x003F3F),
(900413, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x1000, 11, 3064, 0, 0, 'Lava Burst: +50% Damage', 0x003F3F),
(900414, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x1000, 11, 3064, 0, 0, 'Lava Burst: +9 Targets', 0x003F3F),
(900415, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 11, 62, 0, 0, 'Elemental Resonance', 0x003F3F),
(900416, 0, 1, 1, 1, -1, 6, 0, 2, 1, 79, 4, 0, 0, 11, 3064, 4, 5, 'Charged Flames', 0x003F3F),
(900417, 0, 1, 1, 1, -1, 6, 0, 2, 1, 79, 8, 0, 0, 11, 62, 8, 5, 'Searing Charge', 0x003F3F),
(900418, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 11, 19, 0, 0, 'Lightning Shield: Chain Lightning', 0x003F3F);

DELETE FROM `spell_proc` WHERE `SpellId` = 900415;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900415, 0, 0, 0, 0, 0, 0x50000, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Shaman Enhance: Wolf summon proc 900436
DELETE FROM `spell_proc` WHERE `SpellId` = 900436;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900436, 0, 0, 0, 0, 0, 0x4, 0, 0, 0, 0, 0, 0, 10, 5000, 0);

-- Shaman Enhance + Resto: spell_dbc
-- 900435 also carries a +50 % damage spellmod on the totems' attacks (flags0
-- 0x40000000: Searing Totem's Attack, Magma Totem): a totem's damage takes its
-- OWNER's damage modifiers (Unit::SpellPctDamageModsDone), so the minion aura
-- 901109 on the totem changed nothing (bot run 435).
DELETE FROM `spell_dbc` WHERE `ID` IN (900433, 900434, 900435, 900436, 900437, 900438, 900439, 900440, 900466, 900467);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`) VALUES
(900433, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Enh: Totem Follow', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900434, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Maelstrom AoE', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900435, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 4, 0, 0, 0, 0x40000000, 11, 136, 0, 0, 'Enh: Summons +50%', 0x003F3F, 6, 0, 50, 1, 108, 0),
(900436, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Wolf Summon', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900437, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Wolf Haste', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900438, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 11, 62, 0, 0, 'Enh: Wolf CL', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900439, 0, 0, 0, 0, 1, 28, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Maelstrom Fury', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900440, 0, 0, 0, 0, 1, 0, 1, -1, 2, 200, 800, 15, 13, 0, 0, 0, 0, 0, 11, 3786, 1, 0, 'Spirit Howl', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900466, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Resto: Totem Follow', 0x003F3F, 0, 0, 0, 0, 0, 0),
(900467, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Resto: Mana Regen', 0x003F3F, 0, 0, 0, 0, 0, 0);

-- Spirit Wolf NPC 900436
DELETE FROM `creature_template` WHERE `entry` = 900436;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(900436, 0, 0, 0, 0, 0, 'Spirit Wolf', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 0.5, 1500, 2000, 1, 1, 1, 0, 2048, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 0.5, 0, 0.5, 0, 0, 0, 1, 0, 0, '', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 900436;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900436, 0, 27074, 1, 1, 12340);
