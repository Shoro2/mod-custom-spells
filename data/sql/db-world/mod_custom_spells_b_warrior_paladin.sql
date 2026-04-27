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
DELETE FROM `spell_dbc` WHERE `ID` IN (900168, 900169, 900170, 900171, 900172, 900173, 900174, 900175, 900176);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900168, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x400, 4, 132, 'Prot: Revenge Damage', 0x003F3F),
(900169, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Revenge AoE', 0x003F3F),
(900170, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: TC Rend Sunder', 0x003F3F),
(900171, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x80, 4, 132, 'Prot: TC Damage', 0x003F3F),
(900172, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block AoE', 0x003F3F),
(900173, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block TC', 0x003F3F),
(900174, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 100, 500, 22, 0, 0, 0, 0, 4, 132, 'Block Shield Burst', 0x003F3F),
(900175, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 1000, 22, 0, 0, 0, 0, 4, 132, 'Enhanced Thunderclap', 0x003F3F),
(900176, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 4, 132, 'Revenge Bounce', 0x003F3F);

-- Paladin Holy: spell_dbc 900200-900210
DELETE FROM `spell_dbc` WHERE `ID` IN (900200, 900201, 900202, 900203, 900204, 900205, 900206, 900207, 900208, 900209, 900210);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900200, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Damage', 0x003F3F),
(900201, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Heal', 0x003F3F),
(900202, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS Both', 0x003F3F),
(900203, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x200000, 10, 156, 0, 'Holy: HS +50%', 0x003F3F),
(900204, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 51, 0, 'Holy: Consec Heal', 0x003F3F),
(900205, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 51, 0, 'Holy: Consec Around', 0x003F3F),
(900206, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x20, 10, 51, 0, 'Holy: Consec +50%', 0x003F3F),
(900207, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 5000, 1, 107, 17, 0, 0x20, 10, 51, 0, 'Holy: Consec +5s', 0x003F3F),
(900208, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 100, 800, 15, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Burst', 0x003F3F),
(900209, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 100, 800, 30, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Radiance', 0x003F3F),
(900210, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 50, 200, 31, 0, 0, 0, 0, 10, 51, 2, 'Consecration Heal', 0x003F3F);

-- Paladin Prot: spell_dbc 900234-900241
DELETE FROM `spell_dbc` WHERE `ID` IN (900234, 900235, 900236, 900237, 900238, 900239, 900240, 900241);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900234, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 51, 0, 'PProt: Consec Around', 0x003F3F),
(900235, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x4000, 0, 10, 3477, 0, 'PProt: AS +9 Targets', 0x003F3F),
(900236, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x4000, 0, 10, 3477, 0, 'PProt: AS +50%', 0x003F3F),
(900237, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 99, 1, 107, 4, 0, 0, 0x20, 10, 293, 0, 'PProt: HS +99 Charges', 0x003F3F),
(900238, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20, 10, 293, 0, 'PProt: HS +50%', 0x003F3F),
(900239, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 3477, 0, 'PProt: AS Consec', 0x003F3F),
(900240, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 3015, 0, 'PProt: Judge AS', 0x003F3F),
(900241, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 10, 3015, 0, 'PProt: Judge -2s CD', 0x003F3F);

-- Paladin Ret: spell_proc 900274
DELETE FROM `spell_proc` WHERE `SpellId` = 900274;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900274, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Paladin Ret: spell_dbc 900268-900276
DELETE FROM `spell_dbc` WHERE `ID` IN (900268, 900269, 900270, 900271, 900272, 900273, 900274, 900275, 900276);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900268, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 51, 0, 0, 'Ret: Consec Around', 0x003F3F),
(900269, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 10, 3015, 0, 0, 'Ret: Judge -2s CD', 0x003F3F),
(900270, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 2292, 0, 0, 'Ret: DS +6 Targets', 0x003F3F),
(900271, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20000, 10, 2292, 0, 0, 'Ret: DS +50%', 0x003F3F),
(900272, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 10, 2286, 0, 0, 'Ret: CS +50%', 0x003F3F),
(900273, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x1, 0, 10, 2286, 0, 0, 'Ret: CS +9 Targets', 0x003F3F),
(900274, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 2286, 0, 0, 'Ret: Exorcism Proc', 0x003F3F),
(900275, 0x10000000, 0, 0, 0, 1, 5, 1, 6, 0, 50, 1, 108, 0, 0, 0x200000, 0, 10, 879, 0, 10, 'Exorcism Power', 0x003F3F),
(900276, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 10, 2286, 2, 0, 'CS Bounce', 0x003F3F);
