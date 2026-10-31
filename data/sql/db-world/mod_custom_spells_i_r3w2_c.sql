-- Third concept list (operator, 2026-10-31), wave 2, agent C: Warlock and Priest
-- procs and stacking buffs. Plan: share-public
-- custom-spells/records/2026-10-31-third-concept-list.md. Ids: Warlock Affliction
-- 900809-900810, Demonology 900855-900857 / 900859-900860, Destruction
-- 900881-900882, Priest Discipline 900908-900911, Holy 900945-900948, Shadow
-- 900978-900980. The C++ is in custom_spells_warlock.cpp / custom_spells_priest.cpp,
-- the spell_script_names bindings are added by the integrator.
-- Conventions as in the earlier files: DieSides 0, so a value = its BasePoints;
-- DurationIndex 21 = infinite, 9 = 30 s, 1 = 10 s, 8 = 15 s, 27 = 3 s;
-- RangeIndex 1 = self, 13 = anywhere; EquippedItemClass -1 on every row.
-- Mask columns: EffectSpellClassMaskA_n = effect 1 word n-1, B_n = effect 2,
-- C_n = effect 3.
DELETE FROM `spell_dbc` WHERE `ID` IN (900809, 900810, 900855, 900856, 900857, 900859, 900860, 900881, 900882, 900908, 900909, 900910, 900911, 900945, 900946, 900947, 900948, 900978, 900979, 900980);

-- Passive markers read by C++ (Attributes 0x10000040 / Ex3 0x10000000 as the
-- other script markers), and the plain stacking buffs with one effect.
-- 900809 Warlock Affliction "Life Tap: Dark Tithe": Life Tap adds a stack of
-- 900810 (C++). 900810 Dark Tithe: aura 108 SPELLMOD_DOT (22) +10 % per stack
-- (5 stacks, 30 s) on Corruption flags0 0x2 + Curse of Agony 0x400 + Immolate
-- 0x4 (A_1 = 0x406) and Unstable Affliction flags1 0x100 + Seed of Corruption
-- flags1 0x10 (A_2 = 0x110); a DoT's damage is snapshotted when its aura is made.
-- 900857 Warlock Demonology "Demon Charge: Shockwave" (C++ on Demon Charge 54785).
-- 900881 Warlock Destruction "Incinerate: Stoked Soul Fire" (C++); 900882 Stoked
-- Flames: aura 108 op 0 +50 % per stack (10 stacks, 30 s) on Soul Fire flags1 0x80.
-- 900908 Priest Discipline "Mana Burn: Burnt Reserve" (C++ on Mana Burn 8129).
-- 900911 Inner Fire's healing: aura 136 (+% healing done), amount = the charge
-- count (C++ keeps it), passive so it shows no icon.
-- 900945 Priest Holy "Prayer of Mending: +50% Healing": op 0 +50 % on the heal
-- 33110 (family 6 flags1 0x20); DBC only. 900946 the second jump (C++).
-- 900947 "Divine Hymn: Waves of Light" (C++). 900978 "Mind Flay: Quickened Flay"
-- (C++: a stack of 900979 when a Mind Flay channel ends); 900980 "Mind Sear:
-- Three More" (C++).
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `CumulativeAura`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900809, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 5, 208, 0, 'Life Tap: Dark Tithe', 0x003F3F),
(900810, 0, 0, 1, 9, 1, -1, 6, 0, 10, 1, 108, 22, 0x406, 0x110, 0, 5, 5, 208, 0, 'Dark Tithe', 0x003F3F),
(900857, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 5, 129, 0, 'Demon Charge: Shockwave', 0x003F3F),
(900881, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 5, 184, 0, 'Incinerate: Stoked Soul Fire', 0x003F3F),
(900882, 0, 0, 1, 9, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x80, 0, 10, 5, 184, 0, 'Stoked Flames', 0x003F3F),
(900908, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 212, 0, 'Mana Burn: Burnt Reserve', 0x003F3F),
(900911, 0x40, 0, 1, 21, 1, -1, 6, 0, 1, 1, 136, 0, 0, 0, 0, 0, 6, 51, 0, 'Inner Fire: Healing', 0x003F3F),
(900945, 0x40, 0, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x20, 0, 0, 6, 2219, 0, 'Prayer of Mending: +50% Healing', 0x003F3F),
(900946, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 2219, 0, 'Prayer of Mending: Second Jump', 0x003F3F),
(900947, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 3209, 0, 'Divine Hymn: Waves of Light', 0x003F3F),
(900978, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 548, 0, 'Mind Flay: Quickened Flay', 0x003F3F),
(900980, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 2895, 0, 'Mind Sear: Three More', 0x003F3F);

-- 900855 Warlock Demonology "Felhunter: Spell Shackles": periodic dummy every
-- 1 s (C++ puts 900856 on the warlock's Felhunters).
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900855, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 226, 0, 1000, 5, 77, 0, 'Felhunter: Spell Shackles', 0x003F3F);

-- Multi-effect markers.
-- 900910 Priest Discipline "Inner Fire: Fifty Stacks": effect 1 aura 107
-- SPELLMOD_CHARGES (4) +30 on Inner Fire's flags0 0x2 (20 -> 50 charges),
-- effect 2 a dummy proc aura (spell_proc: every non-triggered cast of the
-- priest costs one charge; effect 1 takes no part, DisableEffectsMask 0x1).
-- 900979 Priest Shadow "Quickened Flay" (Mind Flay: +25 % damage and speed per
-- stack, 10 s, 2 stacks): effect 1 op 0 +25 % on the tick spell 58381 (flags0
-- 0x800000), effect 2 op 19 (SPELLMOD_ACTIVATION_TIME: the tick period) -17 %
-- and effect 3 op 1 (SPELLMOD_DURATION) -17 % on Mind Flay itself (flags2 0x40;
-- the ranks below 9 carry 0x440): one stack = x0.83, two = x0.66 of the time.
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectSpellClassMaskB_1`, `EffectSpellClassMaskB_2`, `EffectSpellClassMaskB_3`, `Effect_3`, `EffectDieSides_3`, `EffectBasePoints_3`, `ImplicitTargetA_3`, `EffectAura_3`, `EffectMiscValue_3`, `EffectSpellClassMaskC_1`, `EffectSpellClassMaskC_2`, `EffectSpellClassMaskC_3`, `CumulativeAura`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900910, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 30, 1, 107, 4, 0x2, 0, 0, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 51, 0, 'Inner Fire: Fifty Stacks', 0x003F3F),
(900979, 0, 0, 1, 1, 1, -1, 6, 0, 25, 1, 108, 0, 0x800000, 0, 0, 6, 0, -17, 1, 108, 19, 0, 0, 0x40, 6, 0, -17, 1, 108, 1, 0, 0, 0x40, 2, 6, 548, 0, 'Quickened Flay', 0x003F3F);

-- Helpers.
-- 900856 Felhunter: Spell Shackles aura: effect 129 (area aura, enemies of the
-- Felhunter), aura 65 casting speed -25 % (cast times x1.25), radius index 23 =
-- 40 yd, infinite, passive.
-- 900859 Demon Charge stun: a 3-s stun (aura 12, mechanic 12), SPELL_ATTR3_ALWAYS_HIT
-- (0x40000) so the shockwave's stun cannot miss; aimed at one enemy.
-- 900860 Demon Charge: Fire damage to one enemy, 666 by row, the script sets
-- 666 + 5 x the warlock's Paragon level; magic, cannot miss.
-- 900909 Burnt Reserve: a Holy absorb (aura 69, all schools, 15 s), the script
-- sets the points = the mana burnt; magic dispel type.
-- 900948 Divine Hymn: a wave of light: Holy damage 666 (the script sets 666 + 5 x
-- Paragon) to every enemy within 40 yd (radius 23) of the destination the script
-- gives it; cannot miss; the hymn's own range filter script keeps out idle mobs.
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectMechanic_1`, `Mechanic`, `DispelType`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `DefenseType`, `PreventionType`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900856, 0x40, 0, 1, 21, 1, -1, 129, 0, -25, 1, 23, 65, 0, 0, 0, 0, 5, 77, 32, 0, 0, 'Spell Shackles', 0x003F3F),
(900859, 0, 0x40000, 1, 27, 13, -1, 6, 0, 0, 6, 0, 12, 0, 12, 12, 0, 5, 129, 1, 1, 1, 'Demon Charge: Stun', 0x003F3F),
(900860, 0, 0x40000, 1, 0, 13, -1, 2, 0, 666, 6, 0, 0, 0, 0, 0, 0, 5, 129, 4, 1, 1, 'Shockwave of Flame', 0x003F3F),
(900909, 0, 0, 1, 8, 13, -1, 6, 0, 0, 21, 0, 69, 127, 0, 0, 1, 6, 212, 2, 1, 1, 'Burnt Reserve', 0x003F3F),
(900948, 0, 0x40000, 1, 0, 13, -1, 2, 0, 666, 16, 23, 0, 0, 0, 0, 0, 6, 3209, 2, 1, 1, 'Wave of Light', 0x003F3F);

-- spell_proc: 900910 Inner Fire: Fifty Stacks - proc effect 2 only
-- (DisableEffectsMask 0x1); every non-triggered cast of the priest, in the CAST
-- phase (SpellPhaseMask 1), of any damage class: melee spell 0x10, ranged spell
-- 0x100, no-class positive / negative 0x400 / 0x1000, magic positive / negative
-- 0x4000 / 0x10000 = 0x15510; chance 100, no cooldown.
DELETE FROM `spell_proc` WHERE `SpellId` IN (900910);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900910, 0, 0, 0, 0, 0, 0x15510, 0, 1, 0, 0, 0x1, 0, 100, 0, 0);
