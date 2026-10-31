-- Third concept list (operator, 2026-10-31), wave 3 (the large items), agent D:
-- Death Knight, Druid, Priest. Plan: share-public
-- custom-spells/records/2026-10-31-third-concept-list.md. Written offline (no
-- build, no database); the integrator builds and tests. The C++ is in
-- custom_spells_dk.cpp / custom_spells_druid.cpp / custom_spells_priest.cpp; the
-- spell_script_names bindings are NOT in this file (they go into
-- mod_custom_spells.sql; agent D's final report lists them).
--
-- Ids: DK Frost 900345-900346, DK Unholy 900379-900382, Druid Balance
-- 901010-901011, Priest Discipline 900912, Holy 900949-900950, Shadow
-- 900981-900983. NEW CREATURES (entry = the number of the spell that summons
-- it): 900381 Corpse Worm, 901010 Star Owl. Every passive below is a plain
-- class passive and takes part in the cursed-item pool (none is a player-castable
-- ability); the pool and the picker follow in wave 4.
-- Conventions as in the earlier files: DieSides 0, so a value = its BasePoints;
-- DurationIndex 21 = infinite, 1 = 10 s, 36 = 1 s; RangeIndex 1 = self, 4 =
-- 30 yd, 13 = anywhere; radius index 13 = 10 yd, 9 = 20 yd; EquippedItemClass -1
-- on every row. Attributes 0x10000040 + Ex3 0x10000000 = a passive that survives
-- death (the other script markers); Attributes 0x80 = a hidden aura that the core
-- removes on death (Unit::RemoveAllAurasOnDeath keeps passive auras).
--
-- Death Knight Frost
--   900345 Wyrm: Frozen Breath  marker; spell_custom_dkf_wyrm_breath_killing_machine
--                               on Killing Machine 51124 and ..._rime on Freezing Fog
--                               59052 (the aura's removal by a spent charge)
--   900346                      the wyrm's breath: Frost cone (TargetA 24, 20 yd), a
--                               1-s channel (AttributesEx 0x44, DurationIndex 36),
--                               666 by row, the cast sets 666 + 5 x the DK's Paragon level
-- Death Knight Unholy
--   900379 Gargoyle army        marker; spell_custom_dku_gargoyle_army on Summon Gargoyle 49206
--   900380 Corpse Blast         marker = a 1-s periodic dummy aura; spell_custom_dku_corpse_tick
--   900381 Corpse Worms         marker; spell_custom_dku_corpse_worm on Corpse Explosion (-49158)
--   900382                      the worm's explosion: Shadow, 10 yd round the worm, 666 by
--                               row, the cast sets 666 + 5 x the DK's Paragon level
-- Druid Balance
--   901010 Star Owls            marker; spell_custom_bal_star_owls on Force of Nature 33831
--   901011                      on every star owl: aura 13 (+damage done, all magic schools),
--                               the amount = the druid's spell power (set and kept by the AI)
-- Priest Discipline
--   900912 Penance: Four More   marker; spell_custom_pri_penance_plus4 on the Penance channels
--                               (-47758 damage, -47757 heal)
-- Priest Holy
--   900949 Spirit Casting       marker; spell_custom_pri_sor_spells on the spirit form 27827
--   900950                      aura 275 (SPELL_AURA_MOD_IGNORE_SHAPESHIFT), priest class set,
--                               class mask 0xFFFFFFFF in all three words (the client's
--                               SpellRec::IsAffectedBy needs a non-empty mask overlap, the
--                               server's IsAffected an overlap or an empty mask): the priest
--                               carries it only in the spirit form
-- Priest Shadow
--   900981 Shadowfiend: Dark Pull  marker; custom_pri_shadowfiend_creature gives the fiend 900982
--   900982                      on the fiend: effect 1 aura 133 +500 % maximum health, effect 2
--                               a 5-s periodic dummy (the pull); spell_custom_pri_shadowfiend_aura
--   900983                      the fiend's explosion: Shadow, 10 yd round the fiend, 666 by row,
--                               the cast sets 666 + 5 x the priest's Paragon level
DELETE FROM `spell_dbc` WHERE `ID` IN (900345, 900346, 900379, 900380, 900381, 900382, 901010, 901011, 900912, 900949, 900950, 900981, 900982, 900983);

-- Passive markers with one dummy effect (900380: the same effect as a 1-s periodic dummy).
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900345, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 15, 3223, 0, 'Frost Wyrm: Frozen Breath', 0x003F3F),
(900379, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 15, 2770, 0, 'Summon Gargoyle: Army', 0x003F3F),
(900380, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 226, 0, 1000, 15, 1987, 0, 'Corpse Explosion: Corpse Blast', 0x003F3F),
(900381, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 15, 1987, 0, 'Corpse Explosion: Corpse Worms', 0x003F3F),
(901010, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 7, 263, 0, 'Force of Nature: Star Owls', 0x003F3F),
(900912, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 6, 2818, 0, 'Penance: Four More', 0x003F3F),
(900949, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 6, 1654, 0, 'Spirit of Redemption: Spirit Casting', 0x003F3F),
(900981, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 6, 1552, 0, 'Shadowfiend: Dark Pull', 0x003F3F);

-- Helpers with one effect.
-- 901011 Star Owl: aura 13 (MOD_DAMAGE_DONE), misc 126 = every magic school; passive.
-- 900950 Spirit Casting: aura 275, the priest's whole class mask; passive, infinite.
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskA_2`, `EffectSpellClassMaskA_3`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(901011, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 13, 126, 0, 0, 0, 7, 263, 0, 'Star Owl: Spell Power', 0x003F3F),
(900950, 0x40, 1, 21, 1, -1, 6, 0, 0, 1, 275, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 6, 1654, 0, 'Spirit Casting', 0x003F3F);

-- 900346 Frost Wyrm: Frozen Breath. A channel of one second (AttributesEx 0x44 =
-- SPELL_ATTR1_CHANNELED_1 | _2, DurationIndex 36) whose only effect is the cone's
-- Frost damage (TargetA 24 = TARGET_UNIT_CONE_ENEMY, radius index 9 = 20 yd) at
-- the start of the channel; AttributesEx3 0x40000 (cannot miss), magic. The wyrm
-- casts it triggered (no cost, no range test); the script gives the damage
-- through the cast's BasePoints.
-- 900382 Corpse Worm Burst and 900983 Shadowfiend Burst: dest-centred Shadow
-- damage (TargetA 16, radius index 13 = 10 yd), the pattern of Bloodworm Burst
-- 900307; the C++ aims them at the unit's spot and sets 666 + 5 x Paragon level.
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `DefenseType`, `PreventionType`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900346, 0, 0x44, 0x40000, 1, 36, 4, -1, 2, 0, 666, 24, 9, 15, 3223, 16, 1, 1, 'Frozen Breath', 0x003F3F),
(900382, 0, 0, 0, 1, 0, 13, -1, 2, 0, 666, 16, 13, 15, 1987, 32, 0, 0, 'Corpse Worm Burst', 0x003F3F),
(900983, 0, 0, 0, 1, 0, 13, -1, 2, 0, 666, 16, 13, 6, 1552, 32, 0, 0, 'Shadowfiend Burst', 0x003F3F);

-- 900982 Shadowfiend: Dark Pull (on the fiend): effect 1 aura 133 +500 % maximum
-- health, effect 2 aura 226 (periodic dummy) every 5 s. Hidden (0x80), not passive;
-- DurationIndex 1 only as a starting value, the C++ sets the duration to the
-- summon's remaining clock less 0.3 s (the Parting Bloom fuse's way).
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectAuraPeriod_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900982, 0x80, 1, 1, 1, -1, 6, 0, 500, 1, 133, 6, 0, 0, 1, 226, 5000, 6, 1552, 0, 'Shadowfiend Dark Pull', 0x003F3F);

-- Creatures. Both are guardians the C++ summons (the Star Owl through Force of
-- Nature's own summon effect, the Corpse Worm with SummonCreature and the guardian
-- summon properties 61); the faction is set in code to the master's. Rows as the
-- Frost Wyrm 900333 / Healing Treant 901066.
-- 900381 Corpse Worm: the Bloodworm's model (display 15983, the client has it) at
-- 1.8 x; melee; ScriptName npc_custom_corpse_worm.
-- 901010 Star Owl: the Night Elf moonkin model (display 15374, creature 15313 uses
-- it), a mage-class caster; ScriptName npc_custom_star_owl.
DELETE FROM `creature_template` WHERE `entry` IN (900381, 901010);
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(900381, 0, 0, 0, 0, 0, 'Corpse Worm', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 2, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1.5, 1, 1, 0, 0, 0, 1, 0, 0, 'npc_custom_corpse_worm', 12340),
(901010, 0, 0, 0, 0, 0, 'Star Owl', '', '', 0, 80, 80, 2, 14, 0, 1, 1.14286, 1, 1, 20, 0, 0, 1, 2000, 2000, 1, 1, 8, 0, 2048, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 3, 3, 1, 0, 0, 0, 1, 0, 0, 'npc_custom_star_owl', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (900381, 901010);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900381, 0, 15983, 1.8, 1, 12340),
(901010, 0, 15374, 0.8, 1, 12340);
