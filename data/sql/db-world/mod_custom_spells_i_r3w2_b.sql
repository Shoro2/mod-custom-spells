-- Third concept list (operator, 2026-10-31), wave 2 (procs and stacking buffs),
-- agent B: Hunter Survival, Druid Balance, Rogue Assassination / Combat, Mage.
-- Plan and line-by-line comparison: share-public
-- custom-spells/records/2026-10-31-third-concept-list.md.
-- Markers are passive rows (Attributes 0x40, self, infinite) that the C++ looks
-- for with HasAura; helpers are the auras and bursts those scripts cast. Value
-- = BasePoints (DieSides 0). EquippedItemClass -1 is inline (0 = a consumable
-- requirement). Durations: 9 = 30 s, 21 = infinite, 32 = 6 s. Radius 13 = 10 yd.
--   900574 Surv: Random Traps    5-s periodic dummy (aura 226), the C++ casts a
--                                random known trap in combat
--   900575 Surv: Trap Echo       dummy + proc on the trap activation event
--   901008 Wrath: Starfire Surge marker; 901009 its buff: +10 % Starfire damage
--                                per stack (spellmod aura 108, op 0, family 7
--                                flags0 0x4), 10 stacks, 30 s
--   900606 Assa: Bloody Contagion  dummy + kill proc (C++)
--   900643 Combat: Fan Bleed     marker; 900644 / 900645 the bleeds of the two
--                                Fan of Knives hits (periodic damage, 6 s, one
--                                tick every 2 s, bleed mechanic 15, tick value
--                                passed by the C++)
--   900720 Arcane: Blink Image   marker (C++ summons the image)
--   900721 Arcane: Image Power   marker; 900722 the aura on each image: +5 %
--                                damage done (aura 79, all schools) per stack,
--                                stacks = images alive (set by the C++)
--   900744 Fire: Dragon's Breath Bomb, 900745 Fire: Molten Flamestrike markers
--   900775 / 900776 Frost: Ice Block lances / detonation markers; 900777 the
--                                0.5-s ticker (aura 226) that lives with Ice
--                                Block; 900778 the detonation (Frost, 10 yd
--                                round the mage, value passed by the C++)
DELETE FROM `spell_dbc` WHERE `ID` IN (900574, 900575, 901008, 901009, 900606, 900643, 900644, 900645, 900720, 900721, 900722, 900744, 900745, 900775, 900776, 900777, 900778);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `Mechanic`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectAuraPeriod_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- Hunter Survival
(900574, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 226, 5000, 0, 0, 9, 37, 0, 0, 'Surv: Random Traps', 0x003F3F),
(900575, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 9, 37, 0, 0, 'Surv: Trap Echo', 0x003F3F),
-- Druid Balance
(901008, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 7, 263, 0, 0, 'Balance: Wrath Surge', 0x003F3F),
(901009, 0, 0, 1, 9, 1, -1, 6, 0, 10, 1, 0, 108, 0, 0, 0x4, 7, 1485, 0, 10, 'Wrath Surge', 0x003F3F),
-- Rogue Assassination
(900606, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 8, 2961, 0, 0, 'Assa: Bloody Contagion', 0x003F3F),
-- Rogue Combat
(900643, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 8, 2904, 0, 0, 'Combat: Serrated Fan', 0x003F3F),
(900644, 0, 15, 1, 32, 13, -1, 6, 0, 1, 6, 0, 3, 2000, 0, 0, 8, 2904, 1, 0, 'Serrated Blades', 0x003F3F),
(900645, 0, 15, 1, 32, 13, -1, 6, 0, 1, 6, 0, 3, 2000, 0, 0, 8, 2904, 1, 0, 'Serrated Blades', 0x003F3F),
-- Mage Arcane
(900720, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 3, 1499, 0, 0, 'Arcane: Blink Image', 0x003F3F),
(900721, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 3, 331, 0, 0, 'Arcane: Image Power', 0x003F3F),
(900722, 0, 0, 1, 21, 1, -1, 6, 0, 5, 1, 0, 79, 0, 127, 0, 3, 331, 0, 30, 'Strength in Numbers', 0x003F3F),
-- Mage Fire
(900744, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 3, 1548, 0, 0, 'Fire: Dragon''s Breath Bomb', 0x003F3F),
(900745, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 3, 2307, 0, 0, 'Fire: Molten Flamestrike', 0x003F3F),
-- Mage Frost
(900775, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 3, 14, 0, 0, 'Frost: Ice Block Lances', 0x003F3F),
(900776, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 3, 14, 0, 0, 'Frost: Ice Block Detonation', 0x003F3F),
(900777, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 226, 500, 0, 0, 3, 14, 0, 0, 'Ice Block Lances', 0x003F3F),
(900778, 0, 0, 1, 0, 1, -1, 2, 0, 666, 15, 13, 0, 0, 0, 0, 3, 14, 16, 0, 'Ice Block Detonation', 0x003F3F);

-- Procs. 900575: the trap activation event (PROC_FLAG_DONE_TRAP_ACTIVATION
-- 0x200000), finish phase (4) = once per trap cast, 25 %; the trap's effect is a
-- triggered cast, so PROC_ATTR_TRIGGERED_CAN_PROC (0x2). 900606: the kill proc
-- (PROC_FLAG_KILL 0x2), 100 %. 900745: damage done by spells (magic 0x10000),
-- DoT ticks (0x40000) and wand shots (0x40), damage only (SpellTypeMask 1), hit
-- phase, 5 %, 1 s cooldown.
DELETE FROM `spell_proc` WHERE `SpellId` IN (900575, 900606, 900745);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900575, 0, 0, 0, 0, 0, 0x200000, 0, 4, 0, 0x2, 0, 0, 25, 0, 0),
(900606, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 100, 0, 0),
(900745, 0, 0, 0, 0, 0, 0x50040, 1, 2, 0, 0, 0, 0, 5, 1000, 0);
