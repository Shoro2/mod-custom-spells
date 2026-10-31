-- Third concept list (operator, 2026-10-31), wave 1: the spell modifiers.
-- Plan and line-by-line comparison: share-public
-- custom-spells/records/2026-10-31-third-concept-list.md.
-- Every row is a passive marker (Attributes 0x40, self, infinite) carrying a
-- spell modifier on the named spell's family mask: aura 108 = percent, 107 =
-- flat; misc 0 = damage/healing (SPELLMOD_DAMAGE), 17 = jump targets, 22 =
-- periodic damage/healing (SPELLMOD_DOT). "+9 targets" on a spell without a
-- chain of its own is value 10 (10 targets in all), as Obliterate's 900335.
-- Masks (checked against the server Spell.dbc, 2026-10-31):
--   Death Strike     family 15 flags0 0x10      (main and off-hand strikes)
--   Frost Strike     family 15 flags1 0x4       (main and off-hand strikes)
--   Scourge Strike   family 15 flags1 0x8000000 (its Shadow part 70890 takes
--                                                its share of the boosted hit)
--   Death Coil       family 15 flags2 0x1       = the damage spell 47632 only:
--                    the cast 49895 is TARGET_UNIT_TARGET_ANY and its core
--                    script heals every friendly unit it reaches, so the
--                    chain must sit on the enemy-only damage spell
--   Stormstrike      family 11 flags2 0x1400    = the two weapon strikes
--                    32175/32176; the debuff cast 17364 stays single-target
--   Lava Lash        family 11 flags2 0x4
--   Maul             family 7  flags0 0x800
--   Claw             family 7  flags2 0x40000
--   Shred            family 7  flags0 0x8000
--   traps            family 9  flags0 0x4       = Explosive Trap Effect and
--                    Immolation Trap Effect (direct part op 0, burn op 22)
--   Curse of Agony   family 5  flags0 0x400     (DoT -> op 22)
--   Drain Life       family 5  flags0 0x8       (leech ticks -> op 22)
--   Soul Fire        family 5  flags1 0x80
--   Immolate         family 5  flags0 0x4       (its three effects share one
--                    target selection, so the DoT lands on every chain target)
--   Penance (heal)   family 6  flags1 0x10000   = the healing ticks only
--   Holy Nova        family 6  flags0 0x8400000 = damage 0x400000 + heal 0x8000000
--   Mind Sear        family 6  flags1 0x80000   = the damage tick 53022 (a
--                    direct damage spell, so op 0, not op 22)
-- Shadow Cleave (50581) has no family flags: 900853 / 900854 are dummy markers
-- read by spell_custom_wlk_shadow_cleave (mod_custom_spells.sql).
DELETE FROM `spell_dbc` WHERE `ID` IN (900309, 900310, 900343, 900344, 900375, 900376, 900377, 900378, 900441, 900442, 900443, 900444, 900573, 900807, 900808, 900853, 900854, 900878, 900879, 900880, 900907, 900944, 900977, 901042, 901043, 901057, 901058, 901059, 901060);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- Death Knight
(900309, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0x10, 0, 0, 15, 2751, 0, 'Death Strike: +50% Damage', 0x003F3F),
(900310, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0x10, 0, 0, 15, 2751, 0, 'Death Strike: +9 Targets', 0x003F3F),
(900343, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x4, 0, 15, 2740, 0, 'Frost Strike: +50% Damage', 0x003F3F),
(900344, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x4, 0, 15, 2740, 0, 'Frost Strike: +9 Targets', 0x003F3F),
(900375, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x8000000, 0, 15, 3143, 0, 'Scourge Strike: +50% Damage', 0x003F3F),
(900376, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x8000000, 0, 15, 3143, 0, 'Scourge Strike: +9 Targets', 0x003F3F),
(900377, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x1, 15, 88, 0, 'Death Coil: +50% Damage', 0x003F3F),
(900378, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0, 0x1, 15, 88, 0, 'Death Coil: +9 Targets', 0x003F3F),
-- Shaman Enhancement
(900441, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x1400, 11, 2562, 0, 'Stormstrike: +50% Damage', 0x003F3F),
(900442, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0, 0x1400, 11, 2562, 0, 'Stormstrike: +9 Targets', 0x003F3F),
(900443, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x4, 11, 3741, 0, 'Lava Lash: +50% Damage', 0x003F3F),
(900444, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0, 0x4, 11, 3741, 0, 'Lava Lash: +9 Targets', 0x003F3F),
-- Warlock
(900807, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 22, 0x400, 0, 0, 5, 544, 0, 'Curse of Agony: +50% Damage', 0x003F3F),
(900808, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 22, 0x8, 0, 0, 5, 546, 0, 'Drain Life: +50% Damage', 0x003F3F),
(900853, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 5, 2310, 0, 'Shadow Cleave: Unlimited Targets', 0x003F3F),
(900854, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 5, 2310, 0, 'Shadow Cleave: +50% Damage', 0x003F3F),
(900878, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x80, 0, 5, 184, 0, 'Soul Fire: +50% Damage', 0x003F3F),
(900879, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0x80, 0, 5, 184, 0, 'Soul Fire: +9 Targets', 0x003F3F),
(900880, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0x4, 0, 0, 5, 31, 0, 'Immolate: +9 Targets', 0x003F3F),
-- Priest
(900907, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x10000, 0, 6, 2818, 0, 'Penance: +50% Healing', 0x003F3F),
(900944, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0x8400000, 0, 0, 6, 1874, 0, 'Holy Nova: +50% Damage and Healing', 0x003F3F),
(900977, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0x80000, 0, 6, 2895, 0, 'Mind Sear: +50% Damage', 0x003F3F),
-- Druid
(901042, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0x800, 0, 0, 7, 261, 0, 'Maul: +50% Damage', 0x003F3F),
(901043, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0x800, 0, 0, 7, 261, 0, 'Maul: +9 Targets', 0x003F3F),
(901057, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0, 0, 0x40000, 7, 262, 0, 'Claw: +50% Damage', 0x003F3F),
(901058, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0, 0, 0x40000, 7, 262, 0, 'Claw: +9 Targets', 0x003F3F),
(901059, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0x8000, 0, 0, 7, 147, 0, 'Shred: +50% Damage', 0x003F3F),
(901060, 0x40, 1, 21, 1, -1, 6, 0, 10, 1, 107, 17, 0x8000, 0, 0, 7, 147, 0, 'Shred: +9 Targets', 0x003F3F);

-- Hunter Survival 900573 Traps +50 %: effect 1 the direct part (op 0), effect 2
-- the burn (op 22), both on flags0 0x4.
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900573, 0x40, 1, 21, 1, -1, 6, 0, 50, 1, 108, 0, 0x4, 6, 0, 50, 1, 108, 22, 0x4, 9, 37, 0, 'Traps: +50% Damage', 0x003F3F);
