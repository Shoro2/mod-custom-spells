-- Third concept list (operator, 2026-10-31), wave 2 agent A: procs and stacking
-- buffs of the Paladin, Death Knight and Shaman lines. Plan and line-by-line
-- comparison: share-public custom-spells/records/2026-10-31-third-concept-list.md.
-- Written offline (no build, no database); the integrator builds and tests.
-- Script bindings (spell_script_names) are NOT in this file: they are added to
-- mod_custom_spells.sql by the integrator (agent A's final report lists them).
--
-- Every marker is a passive (Attributes 0x40, self, infinite, one dummy aura)
-- that the scripts and spell_proc rows read; helpers carry their own rows.
-- Value convention: DieSides 0, so the value is BasePoints (the C++ casts
-- override BasePoints of the bursts with 666 + 5 x Paragon level).
-- Ranges: RangeIndex 1 = self (a burst aimed at a unit turns into a destination
-- cast, as 900208 does), 13 = unlimited (a unit-targeted helper cast triggered).
-- Radius index 13 = 10 yd. DurationIndex 1 = 10 s, 27 = 3 s, 21 = infinite.
--
-- Paladin Holy
--   900213 Enlightened Heals  marker; spell_custom_holy_enlighten on Holy Light
--                             (-635), Flash of Light (-19750), Holy Shock heal (-25914)
--   900214 Light Erupts       marker; spell_custom_holy_hoj_erupt on Holy Shock damage (-25912)
--   900215 / 900216           the two Holy bursts (10 yd, dest-centred, 666 + 5 x PL);
--                             900216 is bound to spell_custom_exclude_anchor_target
--                             (the shocked unit took the shock already), 900215 is
--                             centred on a friendly unit and needs no exclusion
-- Paladin Protection
--   900242 Wave of Light      proc marker (spell_proc below), per-target 1 s in C++
--   900243                    the wave (Holy, 10 yd, 666 + 5 x PL), anchor excluded
-- Death Knight Blood
--   900311 Disease retaliation  proc marker: 5 % of damage taken
--   900312 Rune Tap shadow      marker; spell_custom_dkb_rune_tap_shadow on Rune Tap (48982)
--   900313                      the buff: aura 79 +50 %, misc 32 = Shadow, 10 s
-- Shaman Enhancement
--   900445 Ghost Wolf: Storm Rush  marker; spell_custom_enh_ghost_wolf on Ghost Wolf (2645)
--   900446                         +140 % run speed (aura 31 takes the highest speed
--                                  aura: Ghost Wolf's own +40 % plus the line's +100 %), 3 s
--   900447                         hidden Storm Charge: infinite, one spell_proc charge;
--                                  C++ cuts it to a 4-s window when Ghost Wolf ends
--   900448                         the eruption (Nature, 10 yd, 666 + 5 x PL), anchor excluded
-- Shaman Restoration
--   900468 heals launch strikes   marker; spell_custom_rst_heal_strikes on Healing Wave
--                                 (-331), Lesser Healing Wave (-8004), Chain Heal (-1064),
--                                 Riptide (-61295)
--   900469 Earth Shield spikes    marker; spell_custom_rst_earth_spikes on Earth Shield (-974)
--   900470 shield stacks          marker; spell_custom_rst_shield_stacks on the same four heals
--   900471                        the spike: Nature damage on the attacker, unit-targeted
DELETE FROM `spell_dbc` WHERE `ID` IN (900213, 900214, 900215, 900216, 900242, 900243, 900311, 900312, 900313, 900445, 900446, 900447, 900448, 900468, 900469, 900470, 900471);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- Paladin Holy
(900213, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 10, 70, 0, 'Holy: Enlightened Heals', 0x003F3F),
(900214, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 10, 156, 0, 'Holy: Light Erupts', 0x003F3F),
(900215, 0, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 10, 70, 2, 'Enlightenment', 0x003F3F),
(900216, 0, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 10, 156, 2, 'Light Erupts', 0x003F3F),
-- Paladin Protection
(900242, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 10, 302, 0, 'PProt: Wave of Light', 0x003F3F),
(900243, 0, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 10, 302, 2, 'Wave of Light', 0x003F3F),
-- Death Knight Blood
(900311, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 15, 3142, 0, 'DKB: Disease Retaliation', 0x003F3F),
(900312, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 15, 2726, 0, 'DKB: Rune Tap Shadow', 0x003F3F),
(900313, 0, 1, 1, 1, -1, 6, 0, 50, 1, 0, 79, 32, 15, 2726, 32, 'Rune Tap: Shadow Power', 0x003F3F),
-- Shaman Enhancement
(900445, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 11, 67, 0, 'Enh: Ghost Wolf Storm Rush', 0x003F3F),
(900446, 0, 1, 27, 1, -1, 6, 0, 140, 1, 0, 31, 0, 11, 67, 8, 'Storm Rush', 0x003F3F),
(900447, 0x80, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 11, 67, 8, 'Storm Charge', 0x003F3F),
(900448, 0, 1, 0, 1, -1, 2, 0, 666, 16, 13, 0, 0, 11, 62, 8, 'Storm Eruption', 0x003F3F),
-- Shaman Restoration
(900468, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 11, 62, 0, 'Resto: Healing Lightning', 0x003F3F),
(900469, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 11, 2015, 0, 'Resto: Earth Shield Spikes', 0x003F3F),
(900470, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 11, 2015, 0, 'Resto: Shield Stacks', 0x003F3F),
(900471, 0, 1, 0, 13, -1, 2, 0, 666, 6, 0, 0, 0, 11, 2015, 8, 'Earth Shield Spikes', 0x003F3F);

-- spell_proc
-- 900242 Wave of Light: damage DONE by melee auto attacks 0x4, melee abilities
--   0x10 and harmful spells 0x10000 (no ticks); damage type, hit phase;
--   PROC_ATTR_TRIGGERED_CAN_PROC (0x2): Judgement's damage spells and Avenger's
--   Shield are triggered casts. 100 %, no aura cooldown: the 1-s cooldown is per
--   target and lives in C++ (the DataMap, as Devastate Lightning's).
-- 900311 Disease retaliation: every kind of damage TAKEN - melee auto 0x8, melee
--   ability 0x20, ranged auto 0x80, ranged ability 0x200, no-class harmful 0x2000,
--   harmful spells 0x20000 (the 0x222A8 of Mirror Shield and Ursine Bulwark); 5 %,
--   no cooldown; triggered enemy hits count (0x2). HitMask 0: normal and critical
--   hits, a miss or a dodge is no hit.
-- 900447 Storm Charge: as 900242 (done melee / abilities / spells, triggered
--   hits too), 100 %, Charges 1: the core drops the charge with the proc that is
--   taken and removes the aura; the C++ CheckProc refuses a hit while the shaman
--   is still in Ghost Wolf form.
DELETE FROM `spell_proc` WHERE `SpellId` IN (900242, 900311, 900447);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900242, 0, 0, 0, 0, 0, 0x10014, 1, 2, 0, 0x2, 0, 0, 100, 0, 0),
(900311, 0, 0, 0, 0, 0, 0x222A8, 1, 2, 0, 0x2, 0, 0, 5, 0, 0),
(900447, 0, 0, 0, 0, 0, 0x10014, 1, 2, 0, 0x2, 0, 0, 100, 0, 1);
