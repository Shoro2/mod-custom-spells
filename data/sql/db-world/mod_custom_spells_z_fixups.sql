-- Cross-file fixups for the custom spell_dbc rows. This file sorts LAST so it
-- runs after every per-class INSERT file.
--
-- EquippedItemClass: the spell_dbc table default is 0 (= ITEM_CLASS_CONSUMABLE),
-- but "no equipment requirement" is -1 in the DBC. With 0, the core's
-- equipped-item gate (Aura::GetProcEffectMask and Spell::CheckItems) demands an
-- equipped item of class 0, which cannot exist -> every proc and every cast of
-- an affected spell fails with "HasItemFitToSpellRequirements: Not handled
-- spell requirement for item class 0". This silently killed all proc-driven
-- custom spells and the area-helper casts (2026-07-18).
UPDATE `spell_dbc` SET `EquippedItemClass` = -1 WHERE `ID` BETWEEN 900000 AND 901199 AND `EquippedItemClass` = 0;

-- Legacy Warrior Fury leftovers (900138-900145 replaced by the real-DBC block
-- 900108-900121 in March 2026): the module never re-inserts them, but live DBs
-- that predate the removal still carry the rows and their proc entries.
DELETE FROM `spell_dbc` WHERE `ID` IN (900138, 900140, 900141, 900144, 900145);
DELETE FROM `spell_proc` WHERE `SpellId` IN (900138, 900140, 900141, 900144, 900145);
