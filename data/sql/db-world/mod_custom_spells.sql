-- Link custom spell IDs to their SpellScript names
DELETE FROM `spell_script_names` WHERE `spell_id` IN (900106, 900107);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(900106, 'spell_custom_paragon_strike'),
(900107, 'spell_custom_bladestorm_cd_reduce');
