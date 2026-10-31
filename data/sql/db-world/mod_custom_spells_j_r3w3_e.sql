-- Third concept list (operator, 2026-10-31), wave 3 agent E: the Shaman
-- large items. Plan and line-by-line comparison: share-public
-- custom-spells/records/2026-10-31-third-concept-list.md.
-- Written offline (no build, no database); the integrator builds and tests.
-- Script bindings (spell_script_names) are NOT in this file: they are added to
-- mod_custom_spells.sql by the integrator (agent E's final report lists them).
--
-- 1. "Totems replaced by creatures that follow/fight" (all three specs, answer
--    2 a): the passives 900401 / 900433 / 900466 are unchanged rows; the C++
--    (custom_spells_shaman.cpp, "Totem Spirits") now summons a creature body
--    next to each totem. The four bodies below are the creatures of the
--    elements. They have NO spell row: the passives summon them from C++, so
--    the entries come from the free ids of the Elemental block (900419-900422,
--    reserved in 06-custom-ids) instead of a summoning spell's id.
--      900419 Flame Spirit  (fire totems)   display 1204  = Greater Fire Elemental 15438
--      900420 Stone Spirit  (earth totems)  display 14511 = Greater Earth Elemental 15352
--      900421 Tide Spirit   (water totems)  display 525   = Water Elemental 510
--      900422 Wind Spirit   (air totems)    display 21130 = Lesser Air Elemental 28384
--    (display ids as stock creature_template_model rows; the client's CreatureDisplayInfo.dbc
--    has all four. The display's own scale (fire 1.5, earth 1.5, water 0.85, air 0.5) is
--    multiplied by the DisplayScale below: about 0.9 / 0.9 / 0.85 / 0.6 in all - a first
--    guess at shaman height, to be looked at in game.)
--    Template: as the proc wolf 900436 (a guardian of the shaman: faction 14 is
--    replaced by the shaman's, HealthModifier above 1 is capped by the core for
--    player-owned guardians), type Elemental, a melee fighter (DamageModifier
--    0.6, 2-s swing), run speed 1.5 so it keeps up with the shaman (a spirit
--    that falls 45 yd behind is brought back by the AI). ScriptName
--    npc_custom_totem_body is the AI (follow, pick the shaman's target, melee).
-- 2. 900472 "Earth Shield on unlimited targets": marker; spell_custom_rst_earth_unlimited
--    on Earth Shield (-974) takes the earlier applications out of the caster's
--    single-target list before a new one is made. No core change.
-- 3. Healing Stream: 900473 marker; spell_custom_rst_stream_surge on Healing
--    Wave (-331), Lesser Healing Wave (-8004), Chain Heal (-1064), Riptide
--    (-61295). 900474 is only the log id of the 75 % splash heal (never cast:
--    the C++ heals through Unit::HealBySpell). 900475 is the stacking buff
--    (30 s = DurationIndex 9, 10 stacks): aura 108 +25 % per stack, misc 0 =
--    damage/healing, on family 11 flags0 0x2000 = the Healing Stream auras
--    5672 .. 58765 that the totem casts (checked against the server Spell.dbc:
--    only the Healing Stream chain, its triggered heals 52041-52050 / 58759-58761
--    and the Corrupted Healing Stream / Brewmaiden's Blessing item procs carry
--    that bit in family 11). The totem's pulse is computed with the owner's
--    spell mods (spell_sha_healing_stream_totem -> owner->SpellHealingBonusDone
--    with the triggering aura spell), so a mod on the shaman reaches it.
--
-- Ids used: spells 900472-900475, creatures 900419-900422.
DELETE FROM `spell_dbc` WHERE `ID` IN (900472, 900473, 900474, 900475);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `Mechanic`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectAuraPeriod_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900472, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 11, 2015, 0, 0, 'Resto: Unlimited Earth Shield', 0x003F3F),
(900473, 0x40, 0, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 11, 1647, 0, 0, 'Resto: Stream Surge', 0x003F3F),
(900474, 0, 0, 1, 0, 13, -1, 10, 0, 0, 21, 0, 0, 0, 0, 0, 11, 1647, 8, 0, 'Healing Stream Splash', 0x003F3F),
(900475, 0, 0, 1, 9, 1, -1, 6, 0, 25, 1, 0, 108, 0, 0, 0x2000, 11, 1647, 8, 10, 'Stream Surge', 0x003F3F);

-- Totem spirits 900419-900422
DELETE FROM `creature_template` WHERE `entry` IN (900419, 900420, 900421, 900422);
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(900419, 0, 0, 0, 0, 0, 'Flame Spirit', 'Totem Spirit', '', 0, 80, 80, 2, 14, 0, 1, 1.5, 1, 1, 20, 0, 0, 0.6, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 'npc_custom_totem_body', 12340),
(900420, 0, 0, 0, 0, 0, 'Stone Spirit', 'Totem Spirit', '', 0, 80, 80, 2, 14, 0, 1, 1.5, 1, 1, 20, 0, 0, 0.6, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 'npc_custom_totem_body', 12340),
(900421, 0, 0, 0, 0, 0, 'Tide Spirit', 'Totem Spirit', '', 0, 80, 80, 2, 14, 0, 1, 1.5, 1, 1, 20, 0, 0, 0.6, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 'npc_custom_totem_body', 12340),
(900422, 0, 0, 0, 0, 0, 'Wind Spirit', 'Totem Spirit', '', 0, 80, 80, 2, 14, 0, 1, 1.5, 1, 1, 20, 0, 0, 0.6, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 'npc_custom_totem_body', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` IN (900419, 900420, 900421, 900422);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900419, 0, 1204, 0.6, 1, 12340),
(900420, 0, 14511, 0.6, 1, 12340),
(900421, 0, 525, 1, 1, 12340),
(900422, 0, 21130, 1.2, 1, 12340);
