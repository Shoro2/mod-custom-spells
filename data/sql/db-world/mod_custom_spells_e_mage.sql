-- Mage Arcane: Emergency Mana Shield 900708 (TAKEN_DAMAGE 0x100000)
DELETE FROM `spell_proc` WHERE `SpellId` = 900708;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900708, 0, 0, 0, 0, 0, 0x100000, 0, 0, 0, 0, 0, 0, 100, 60000, 0);

-- Mage Arcane: spell_dbc 900700-900713
-- Masks verified against Spell.dbc SpellFamilyFlags: Arcane Barrage flags1=0x8000
-- (goes in A_2, not 0x1000000), Arcane Blast flags0=0x20000000. 900703 uses
-- SPELLMOD_CASTING_TIME (10), not SPELLMOD_COST (14).
DELETE FROM `spell_dbc` WHERE `ID` IN (900700, 900701, 900702, 900703, 900704, 900705, 900706, 900707, 900708, 900709, 900710, 900711, 900712, 900713);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `Category`, `CategoryRecoveryTime`, `EffectSpellClassMaskA_3`) VALUES
(900700, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Mana Regen', 0x003F3F, 0, 0, 0),
(900701, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x8000, 0, 3, 33, 0, 0, 'Arcane: Barrage +50%', 0x003F3F, 0, 0, 0),
(900702, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0, 0x8000, 0, 3, 33, 0, 0, 'Arcane: Barrage +9', 0x003F3F, 0, 0, 0),
(900703, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, -50, 1, 108, 10, 0, 0x20000000, 0, 0, 3, 33, 0, 0, 'Arcane: Blast -50% Cast', 0x003F3F, 0, 0, 0),
(900704, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x20000000, 0, 0, 3, 33, 0, 0, 'Arcane: Blast +9', 0x003F3F, 0, 0, 0),
(900705, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 4, 1, 107, 31, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Charges x8', 0x003F3F, 0, 0, 0x4),
(900706, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: AE Charges', 0x003F3F, 0, 0, 0),
(900707, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Evoc Power', 0x003F3F, 0, 0, 0),
(900708, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Emergency Shield', 0x003F3F, 0, 0, 0),
(900709, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 258, 0, 0, 'Arcane: Blink Target', 0x003F3F, 0, 0, 0),
(900710, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 33, 64, 0, 'Arcane Barrage Bounce', 0x003F3F, 0, 0, 0),
(900711, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 33, 64, 0, 'Arcane Blast Bounce', 0x003F3F, 0, 0, 0),
(900712, 0, 0, 0, 0, 1, 3, 1, -1, 6, 0, 20, 1, 79, 127, 0, 0, 0, 0, 3, 33, 0, 4, 'Evocation Power', 0x003F3F, 0, 0, 0),
(900713, 0, 0, 0, 0, 1, 0, 5, -1, 3, 0, 0, 16, 0, 0, 0, 0, 0, 0, 3, 258, 0, 0, 'Targeted Blink', 0x003F3F, 44, 15000, 0);

-- Mage Fire: spell_dbc 900733-900740
DELETE FROM `spell_dbc` WHERE `ID` IN (900733, 900734, 900735, 900736, 900737, 900738, 900739, 900740);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `Effect_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectSpellClassMaskB_1`, `EffectDieSides_2`) VALUES
(900733, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 0, 3, 185, 4, 0, 'Fire: Fireball +50%', 0x003F3F, 6, 108, 22, 50, 1, 0x1, 0),
(900734, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x1, 0, 0, 3, 185, 0, 0, 'Fire: Fireball +9', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900735, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x400000, 0, 0, 3, 1726, 0, 0, 'Fire: Pyro +9', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900736, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x400000, 0, 0, 3, 1726, 4, 0, 'Fire: Pyro +50%', 0x003F3F, 6, 108, 22, 50, 1, 0x400000, 0),
(900737, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, -1500, 1, 107, 21, 0, 0x2, 0, 0, 3, 12, 0, 0, 'Fire: Blast Off GCD', 0x003F3F, 6, 107, 7, 100, 1, 0x2, 0),
(900738, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 1726, 0, 0, 'Fire: Pyro Hot Streak', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900739, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 185, 4, 0, 'Fireball Bounce', 0x003F3F, 0, 0, 0, 0, 0, 0, 0),
(900740, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 1726, 4, 0, 'Pyroblast Bounce', 0x003F3F, 0, 0, 0, 0, 0, 0, 0);

-- Mage Frost: spell_dbc 900766-900774
DELETE FROM `spell_dbc` WHERE `ID` IN (900766, 900767, 900768, 900769, 900770, 900771, 900772, 900773, 900774);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `RecoveryTime`) VALUES
(900766, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x20, 0, 0, 3, 188, 16, 0, 'Frost: Frostbolt +50%', 0x003F3F, 0),
(900767, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x20, 0, 0, 3, 188, 0, 0, 'Frost: Frostbolt +9', 0x003F3F, 0),
(900768, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x20000, 0, 0, 3, 2723, 16, 0, 'Frost: Ice Lance +50%', 0x003F3F, 0),
(900769, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x20000, 0, 0, 3, 2723, 0, 0, 'Frost: Ice Lance +9', 0x003F3F, 0),
(900770, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 2735, 0, 0, 'Frost: Perm Elemental', 0x003F3F, 0),
(900771, 0, 0, 0, 0, 1, 0, 5, -1, 3, 0, 0, 16, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frost Comet Shower', 0x003F3F, 30000),
(900772, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frostbolt Bounce', 0x003F3F, 0),
(900773, 0, 0, 0, 0, 1, 0, 1, -1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 2723, 16, 0, 'Ice Lance Bounce', 0x003F3F, 0),
(900774, 0, 0, 0, 0, 1, 0, 1, -1, 2, 500, 3000, 6, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frost Comet', 0x003F3F, 0);

-- Dest-targeted dummy actives need a radius for implicit target selection
-- (runs last in this file so both INSERT blocks above are already applied).
UPDATE `spell_dbc` SET `EffectRadiusIndex_1` = 13 WHERE `ID` IN (900713, 900771);

-- Mage concept spells built 2026-10-08: 900714 Arcane Overflow (every 10,000 mana
-- spent -> 900718, Arcane Explosion at twice the damage, + Blink reset), 900715
-- Mirror Shield (hit taken while an own absorb shield is up: 10 %, 6 s cooldown ->
-- one Mirror Image 58831), 900716 Mirror Images: Splash (the images' Frostbolt /
-- Fire Blast also hit everything within 8 yd of their target: 900717 / 900719),
-- 900741 Meteor (ACTIVE, picker only: 8 yd round the aimed point, 900742 impact
-- 3,500-4,100 Fire + 900743 burn 200 every 2 sec for 6 sec; 45 s cooldown).
-- Targets 0x40 (TARGET_FLAG_DEST_LOCATION) gives Meteor the client's ground cursor,
-- as Flamestrike and Blizzard carry it; Comet Shower and Targeted Blink get it too.
DELETE FROM `spell_dbc` WHERE `ID` IN (900714, 900715, 900716, 900717, 900718, 900719, 900741, 900742, 900743);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Targets`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectAuraPeriod_1`, `EffectBonusMultiplier_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `SpellVisualID_1`, `RecoveryTime`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900714, 0x40, 1, 21, 1, -1, 0, 6, 0, 0, 1, 0, 4, 0, 0.0, 3, 122, 0, 0, 0, 'Arcane Overflow', 0x003F3F),
(900715, 0x40, 1, 21, 1, -1, 0, 6, 0, 0, 1, 0, 4, 0, 0.0, 3, 331, 0, 0, 0, 'Mirror Shield', 0x003F3F),
(900716, 0x40, 1, 21, 1, -1, 0, 6, 0, 0, 1, 0, 4, 0, 0.0, 3, 331, 0, 0, 0, 'Mirror Images: Splash', 0x003F3F),
(900717, 0, 1, 0, 13, -1, 0, 2, 0, 1, 16, 14, 0, 0, 0.0, 3, 188, 16, 0, 0, 'Mirror Splash', 0x003F3F),
(900718, 0, 1, 0, 1, -1, 0, 2, 88, 1076, 15, 13, 0, 0, 0.428, 3, 122, 64, 965, 0, 'Empowered Arcane Explosion', 0x003F3F),
(900719, 0, 1, 0, 13, -1, 0, 2, 0, 1, 16, 14, 0, 0, 0.0, 3, 185, 4, 0, 0, 'Mirror Splash', 0x003F3F),
(900741, 0, 1, 0, 5, -1, 0x40, 3, 0, 0, 16, 14, 0, 0, 0.0, 3, 184, 4, 0, 45000, 'Meteor', 0x003F3F),
(900742, 0, 1, 0, 13, -1, 0, 2, 601, 3499, 6, 0, 0, 0, 0.0, 3, 184, 4, 7479, 0, 'Meteor', 0x003F3F),
(900743, 0, 1, 32, 13, -1, 0, 6, 1, 199, 6, 0, 3, 2000, 0.0, 3, 184, 4, 0, 0, 'Meteor Burn', 0x003F3F);
UPDATE `spell_dbc` SET `Targets` = 0x40 WHERE `ID` IN (900713, 900771);

-- Hits TAKEN only (0x222A8): SpellPhaseMask stays 0 - the core applies a phase mask
-- to DONE procs only and logs one it would ignore at every start (HOST9 boot, 2026-10-08).
DELETE FROM `spell_proc` WHERE `SpellId` = 900715;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900715, 0, 0, 0, 0, 0, 0x222A8, 1, 0, 0x403, 0, 0, 0, 10, 6000, 0);
