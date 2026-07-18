-- Bladestorm CD Reduce passive (900107)
DELETE FROM `spell_proc` WHERE `SpellId` = 900107;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900107, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Block -> AoE damage (900172): TAKEN_MELEE_AUTO_ATTACK (0x8). 100% chance, 1s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900172;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900172, 0, 0, 0, 0, 0, 0x8, 0, 0, 0, 0, 0, 0, 100, 1000, 0);

-- Block -> Enhanced TC (900173): TAKEN_MELEE_AUTO_ATTACK (0x8). 10% chance, 3s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900173;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900173, 0, 0, 0, 0, 0, 0x8, 0, 0, 0, 0, 0, 0, 10, 3000, 0);

-- Warrior Prot: spell_dbc 900168-900176
-- Area helpers need EffectRadiusIndex (13 = 10yd) - without it the area
-- search has 0yd radius and hits nothing. TargetA 15 = enemies around the
-- caster (both bursts are self-centered casts).
DELETE FROM `spell_dbc` WHERE `ID` IN (900168, 900169, 900170, 900171, 900172, 900173, 900174, 900175, 900176);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900168, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x400, 4, 132, 'Prot: Revenge Damage', 0x003F3F),
(900169, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 4, 132, 'Prot: Revenge AoE', 0x003F3F),
(900170, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 4, 132, 'Prot: TC Rend Sunder', 0x003F3F),
(900171, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x80, 4, 132, 'Prot: TC Damage', 0x003F3F),
(900172, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 4, 132, 'Prot: Block AoE', 0x003F3F),
(900173, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 4, 132, 'Prot: Block TC', 0x003F3F),
(900174, 0, 0, 0, 0, 1, 0, 1, -1, 2, 100, 500, 15, 13, 0, 0, 0, 0, 4, 132, 'Block Shield Burst', 0x003F3F),
(900175, 0, 0, 0, 0, 1, 0, 1, -1, 2, 200, 1000, 15, 13, 0, 0, 0, 0, 4, 132, 'Enhanced Thunderclap', 0x003F3F),
(900176, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 4, 132, 'Revenge Bounce', 0x003F3F);

-- Paladin Holy: spell_dbc 900200-900210
-- 900207 uses SPELLMOD_DURATION (1), not JUMP_TARGETS. The two Holy Shock
-- bursts are cast AT the shock target -> dest-centered targets (16/31) with
-- 10yd radius (13); the Consecration heal pulses around the caster (31 with
-- the cast on self).
DELETE FROM `spell_dbc` WHERE `ID` IN (900200, 900201, 900202, 900203, 900204, 900205, 900206, 900207, 900208, 900209, 900210);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900200, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Damage', 0x003F3F),
(900201, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Heal', 0x003F3F),
(900202, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS Both', 0x003F3F),
(900203, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x200000, 10, 156, 0, 'Holy: HS +50%', 0x003F3F),
(900204, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 10, 51, 0, 'Holy: Consec Heal', 0x003F3F),
(900205, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 10, 51, 0, 'Holy: Consec Around', 0x003F3F),
(900206, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 0, 108, 0, 0, 0x20, 10, 51, 0, 'Holy: Consec +50%', 0x003F3F),
(900207, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 5000, 1, 0, 107, 1, 0, 0x20, 10, 51, 0, 'Holy: Consec +5s', 0x003F3F),
(900208, 0, 0, 0, 0, 1, 0, 1, -1, 2, 100, 800, 16, 13, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Burst', 0x003F3F),
(900209, 0, 0, 0, 0, 1, 0, 1, -1, 10, 100, 800, 31, 13, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Radiance', 0x003F3F),
(900210, 0, 0, 0, 0, 1, 0, 1, -1, 10, 50, 200, 31, 13, 0, 0, 0, 0, 10, 51, 2, 'Consecration Heal', 0x003F3F);

-- 900211: Mobile Consecration (shared by 900205/900234/900268) - periodic
-- dummy self-aura, 1s ticks over Consecration's 8s (DurationIndex 31); the
-- C++ AuraScript deals the carried per-tick amount around the paladin.
DELETE FROM `spell_dbc` WHERE `ID` = 900211;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900211, 0, 0, 0, 0, 1, 31, 1, -1, 6, 0, 0, 1, 226, 1000, 10, 51, 2, 'Mobile Consecration', 0x003F3F);

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
DELETE FROM `spell_dbc` WHERE `ID` IN (900268, 900269, 900270, 900271, 900272, 900273, 900274, 900275, 900276);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900268, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 10, 51, 0, 0, 'Ret: Consec Around', 0x003F3F),
(900269, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 0x8, 10, 3015, 0, 0, 'Ret: Judge -2s CD', 0x003F3F),
(900270, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 10, 2292, 0, 0, 'Ret: DS +6 Targets', 0x003F3F),
(900271, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20000, 0, 10, 2292, 0, 0, 'Ret: DS +50%', 0x003F3F),
(900272, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x8000, 0, 10, 2286, 0, 0, 'Ret: CS +50%', 0x003F3F),
(900273, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 9, 1, 107, 17, 0, 0, 0x8000, 0, 10, 2286, 0, 0, 'Ret: CS +9 Targets', 0x003F3F),
(900274, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 10, 2286, 0, 0, 'Ret: Exorcism Proc', 0x003F3F),
(900275, 0, 0, 0, 0, 1, 5, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x2, 0, 10, 292, 0, 10, 'Exorcism Power', 0x003F3F),
(900276, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 10, 2286, 2, 0, 'CS Bounce', 0x003F3F);
