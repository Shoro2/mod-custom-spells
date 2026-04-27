-- Mage Arcane: Emergency Mana Shield 900708 (TAKEN_DAMAGE 0x100000)
DELETE FROM `spell_proc` WHERE `SpellId` = 900708;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900708, 0, 0, 0, 0, 0, 0x100000, 0, 0, 0, 0, 0, 0, 100, 60000, 0);

-- Mage Arcane: spell_dbc 900700-900713
DELETE FROM `spell_dbc` WHERE `ID` IN (900700, 900701, 900702, 900703, 900704, 900705, 900706, 900707, 900708, 900709, 900710, 900711, 900712, 900713);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900700, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Mana Regen', 0x003F3F),
(900701, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x1000000, 0, 3, 33, 0, 0, 'Arcane: Barrage +50%', 0x003F3F),
(900702, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0, 0x1000000, 0, 3, 33, 0, 0, 'Arcane: Barrage +9', 0x003F3F),
(900703, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -50, 1, 108, 14, 0, 0x20000000, 0, 0, 3, 33, 0, 0, 'Arcane: Blast -50% Cast', 0x003F3F),
(900704, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x20000000, 0, 0, 3, 33, 0, 0, 'Arcane: Blast +9', 0x003F3F),
(900705, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Charges x8', 0x003F3F),
(900706, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: AE Charges', 0x003F3F),
(900707, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Evoc Power', 0x003F3F),
(900708, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Emergency Shield', 0x003F3F),
(900709, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 258, 0, 0, 'Arcane: Blink Target', 0x003F3F),
(900710, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 33, 64, 0, 'Arcane Barrage Bounce', 0x003F3F),
(900711, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 33, 64, 0, 'Arcane Blast Bounce', 0x003F3F),
(900712, 0x10000000, 0, 0, 0, 1, 3, 1, 6, 0, 20, 1, 79, 127, 0, 0, 0, 0, 3, 33, 0, 4, 'Evocation Power', 0x003F3F),
(900713, 0x10000000, 0, 0, 0, 1, 0, 5, 3, 0, 0, 16, 0, 0, 0, 0, 0, 0, 3, 258, 0, 0, 'Targeted Blink', 0x003F3F);

-- Mage Fire: spell_dbc 900733-900740
DELETE FROM `spell_dbc` WHERE `ID` IN (900733, 900734, 900735, 900736, 900737, 900738, 900739, 900740);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900733, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 0, 3, 185, 4, 0, 'Fire: Fireball +50%', 0x003F3F),
(900734, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x1, 0, 0, 3, 185, 0, 0, 'Fire: Fireball +9', 0x003F3F),
(900735, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x400000, 0, 0, 3, 1726, 0, 0, 'Fire: Pyro +9', 0x003F3F),
(900736, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x400000, 0, 0, 3, 1726, 4, 0, 'Fire: Pyro +50%', 0x003F3F),
(900737, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 12, 0, 0, 'Fire: Blast Off GCD', 0x003F3F),
(900738, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 1726, 0, 0, 'Fire: Pyro Hot Streak', 0x003F3F),
(900739, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 185, 4, 0, 'Fireball Bounce', 0x003F3F),
(900740, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 1726, 4, 0, 'Pyroblast Bounce', 0x003F3F);

-- Mage Frost: spell_dbc 900766-900774
DELETE FROM `spell_dbc` WHERE `ID` IN (900766, 900767, 900768, 900769, 900770, 900771, 900772, 900773, 900774);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900766, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x20, 0, 0, 3, 188, 16, 0, 'Frost: Frostbolt +50%', 0x003F3F),
(900767, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x20, 0, 0, 3, 188, 0, 0, 'Frost: Frostbolt +9', 0x003F3F),
(900768, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x20000, 0, 0, 3, 2723, 16, 0, 'Frost: Ice Lance +50%', 0x003F3F),
(900769, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x20000, 0, 0, 3, 2723, 0, 0, 'Frost: Ice Lance +9', 0x003F3F),
(900770, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 2735, 0, 0, 'Frost: Perm Elemental', 0x003F3F),
(900771, 0x10000000, 0, 0, 0, 1, 0, 5, 3, 0, 0, 16, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frost Comet Shower', 0x003F3F),
(900772, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frostbolt Bounce', 0x003F3F),
(900773, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 2723, 16, 0, 'Ice Lance Bounce', 0x003F3F),
(900774, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 500, 3000, 6, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frost Comet', 0x003F3F);
