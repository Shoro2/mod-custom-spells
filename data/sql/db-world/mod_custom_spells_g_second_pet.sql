-- Second pet (concept revision 2026-10-08): Hunter BM "you can now have 2 pets
-- active", Warlock Demonology "you can now have 2 demons active". The pets and
-- their control bar are C++ (src/custom_spells_second_pet.cpp, a PlayerScript
-- that reads the passives with HasAura) and AIO (lua/CustomSpells_PetBar_*.lua).
-- No spell_script_names rows: nothing here is a SpellScript.
--   900525 BM: Second Pet       - passive marker (picker, cursed-item pool)
--   900526 Second Pet           - hidden marker aura on every second pet; other
--                                 scripts recognise the second pet by it.
--                                 Effect 2: the hunter pet's happiness on its
--                                 copy - physical damage done (the weapon
--                                 damage only), the C++ sets +25 (happy) /
--                                 0 / -25 (unhappy), as a hunter pet's 125 /
--                                 100 / 75 % melee damage
--   900858 Demo: Second Demon   - passive marker (picker, cursed-item pool)
DELETE FROM `spell_dbc` WHERE `ID` IN (900525, 900526, 900858);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`, `EffectAura_2`, `EffectMiscValue_2`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`, `RecoveryTime`) VALUES
(900525, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Second Pet', 0x003F3F, 0),
(900526, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 0, 0, 1, 79, 1, 0, 132, 0, 0, 'Second Pet', 0x003F3F, 0),
(900858, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 132, 0, 0, 'Demo: Second Demon', 0x003F3F, 0);

-- 900525: the hunter's second pet. A copy of the hunter's pet: the C++ sets the
-- pet's model, size, name, level, spells and focus at the summon; the template
-- only gives it its own entry (the native pet bar acts on every controlled unit
-- of the pet's entry, so the copy must not share it), a beast's type and PetAI.
DELETE FROM `creature_template` WHERE `entry` = 900525;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
(900525, 0, 0, 0, 0, 0, 'Beast Companion', '', '', 0, 80, 80, 2, 35, 0, 1, 1.14286, 1, 1, 20, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'PetAI', 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, '', 12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 900525;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900525, 0, 604, 1, 1, 12340);
