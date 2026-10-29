-- mod-custom-spells: Warlock Demonology 900858 "you can now have 2 demons active" - the remembered second demon
-- survives a logout (operator 2026-10-29). One row per character: the demon that stays as the second demon, as
-- custom_spells_second_pet.cpp's SerializeDemon writes it (pet number, entry, look, level, pet bar, passives | name).
-- Written when a summon remembers a demon, deleted when the second demon ends. Idempotent.
CREATE TABLE IF NOT EXISTS `mod_custom_spells_second_demon` (
  `guid` INT UNSIGNED NOT NULL COMMENT 'characters.guid',
  `data` TEXT NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
