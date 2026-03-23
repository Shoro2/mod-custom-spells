-- Link custom spell IDs to their SpellScript names
DELETE FROM `spell_script_names` WHERE `spell_id` IN (900106, 900107, 900140, 900141, 900144, 900145, 1680, 57823, 47502, 900172, 900173, -25912, -25914, 48819, -48827, 54158, -35395, 900274, 48801, 49028, -55050, 900304, 46584, 900366, 900368, -49271, 2894, -51505, 900405, 900406, 53817, 900436, 51533, -49048, 75, 900534, 900566, -48463, 901004, -48562, 62078, 901066, 900603, -48638, -48660, -44781, -42897, -42921, 12051, 900708, 900713, -42833, -42891, -42842, -42914, 31687, 900771, 900800, 900802, 900834, -47964, 47994, 18788, -47809, -59172, -48066, 900933, 900966, 900967, 901101, 901102, 901103, 901104);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(900106, 'spell_custom_paragon_strike'),
(900107, 'spell_custom_bladestorm_cd_reduce'),
(900140, 'spell_custom_bloody_whirlwind_passive'),
(900141, 'spell_custom_speedy_bloodthirst'),
(900144, 'spell_custom_ww_overpower'),
(900145, 'spell_custom_ww_slam'),
(1680, 'spell_custom_bloody_whirlwind_consume'),
-- Warrior Prot
(57823, 'spell_custom_prot_revenge_aoe'),
(47502, 'spell_custom_prot_tc_rend_sunder'),
(900172, 'spell_custom_prot_block_aoe'),
(900173, 'spell_custom_prot_block_tc'),
-- Paladin Holy: Holy Shock damage (all ranks via negative ID)
(-25912, 'spell_custom_holy_hs_aoe_dmg'),
(-25912, 'spell_custom_holy_hs_both_dmg'),
-- Paladin Holy: Holy Shock heal (all ranks via negative ID)
(-25914, 'spell_custom_holy_hs_aoe_heal'),
(-25914, 'spell_custom_holy_hs_both_heal'),
-- Paladin Holy: Consecration heal hook (max rank)
(48819, 'spell_custom_holy_consec_heal'),
-- Paladin Prot: AS leaves Consecration (all ranks via negative ID)
(-48827, 'spell_custom_pprot_as_consec'),
-- Paladin Prot: Judgement → free AS (on Judgement damage spell)
(54158, 'spell_custom_pprot_judge_as'),
-- Paladin Ret: CS +9 targets (all ranks via negative ID)
(-35395, 'spell_custom_ret_cs_aoe'),
-- Paladin Ret: Exorcism buff proc passive
(900274, 'spell_custom_ret_exorcism_proc'),
-- Paladin Ret: Exorcism consume stacks (on Exorcism max rank)
(48801, 'spell_custom_ret_exorcism_consume'),
-- DK Blood: 3 Rune Weapons + Double Cast (on DRW spell 49028)
(49028, 'spell_custom_dkb_3_rune_weapons'),
(49028, 'spell_custom_dkb_double_cast'),
-- DK Blood: Heart Strike +9 targets (all ranks via negative ID)
(-55050, 'spell_custom_dkb_hs_aoe'),
-- DK Blood: Death Coil proc passive
(900304, 'spell_custom_dkb_deathcoil_proc'),
-- DK Frost: Frost Wyrm (on Raise Dead 46584)
(46584, 'spell_custom_dkf_frost_wyrm'),
-- DK Frost: Frost Breath damage handler
(900368, 'spell_custom_frost_breath'),
-- DK Unholy: DoT → Shadow AoE proc passive
(900366, 'spell_custom_dku_dot_aoe'),
-- Shaman Ele: Chain Lightning +6 targets (on all CL ranks)
(-49271, 'spell_custom_ele_cl_aoe'),
-- Shaman Ele: Totem follow (900401 is marker aura, PlayerScript handles logic)
-- Shaman Ele: Ragnaros (on Fire Elemental Totem 2894)
(2894, 'spell_custom_ele_ragnaros'),
-- Shaman Ele: LO + LvB, LvB spread FS, LvB charges (on all LvB ranks)
(-51505, 'spell_custom_ele_overload_lvb'),
(-51505, 'spell_custom_ele_lvb_spread_fs'),
(-51505, 'spell_custom_ele_lvb_charges'),
-- Shaman Ele: FS ticks → reset LvB CD (proc aura on 900405)
(900405, 'spell_custom_ele_fs_reset_lvb'),
-- Shaman Enhance: Maelstrom AoE (on Maelstrom Weapon 53817)
(53817, 'spell_custom_enh_maelstrom_aoe'),
-- Shaman Enhance: Wolf summon proc (on 900436)
(900436, 'spell_custom_enh_wolf_summon'),
-- Shaman Enhance: Wolf haste (on Feral Spirit 51533)
(51533, 'spell_custom_enh_wolf_haste'),
-- Hunter: Multi-Shot unlimited targets (all ranks via negative ID)
(-49048, 'spell_custom_hunt_multishot_aoe'),
-- Hunter MM: Auto Shot bounces +9 targets
(75, 'spell_custom_hunt_autoshot_bounce'),
-- Hunter MM: Multi-Shot Barrage (active spell, AuraScript)
(900534, 'spell_custom_hunt_barrage'),
-- Hunter Surv: Explosive trap proc passive
(900566, 'spell_custom_hunt_surv_trap_proc'),
-- Druid Balance: Moonfire +9 targets (all ranks via negative ID)
(-48463, 'spell_custom_bal_mf_aoe'),
-- Druid Balance: Spell dmg reduces Starfall CD (proc aura)
(901004, 'spell_custom_bal_sf_cd_reduce'),
-- Druid Feral: Swipe Bear applies bleed (all ranks via negative ID)
(-48562, 'spell_custom_feral_bear_swipe_bleed'),
-- Druid Feral: Swipe Cat applies bleed
(62078, 'spell_custom_feral_cat_swipe_bleed'),
-- Druid Resto: HoTs summon treant (proc aura)
(901066, 'spell_custom_drst_hot_treant'),
-- Rogue Assa: Poison Nova proc passive
(900603, 'spell_custom_rog_poison_nova'),
-- Rogue Combat: SS +9 targets (all ranks via negative ID)
(-48638, 'spell_custom_rog_ss_aoe'),
-- Rogue Sub: Hemorrhage +9 targets (all ranks via negative ID)
(-48660, 'spell_custom_rog_hemo_aoe'),
-- Mage Arcane: Arcane Barrage +9 targets (all ranks via negative ID)
(-44781, 'spell_custom_mage_abarr_aoe'),
-- Mage Arcane: Arcane Blast +9 targets + charges to 8 (all ranks via negative ID)
(-42897, 'spell_custom_mage_ab_aoe'),
(-42897, 'spell_custom_mage_ab_charges'),
-- Mage Arcane: Arcane Explosion generates charge (all ranks via negative ID)
(-42921, 'spell_custom_mage_ae_charges'),
-- Mage Arcane: Evocation increases spell damage (hooked on Evocation)
(12051, 'spell_custom_mage_evocation_power'),
-- Mage Arcane: Emergency Mana Shield proc passive
(900708, 'spell_custom_mage_emergency_shield'),
-- Mage Arcane: Targeted Blink (active spell)
(900713, 'spell_custom_mage_targeted_blink'),
-- Mage Fire: Fireball +9 targets (all ranks via negative ID)
(-42833, 'spell_custom_mage_fb_aoe'),
-- Mage Fire: Pyroblast +9 targets + Hot Streak (all ranks via negative ID)
(-42891, 'spell_custom_mage_pyro_aoe'),
(-42891, 'spell_custom_mage_pyro_hotstreak'),
-- Mage Frost: Frostbolt +9 targets (all ranks via negative ID)
(-42842, 'spell_custom_mage_frostbolt_aoe'),
-- Mage Frost: Ice Lance +9 targets (all ranks via negative ID)
(-42914, 'spell_custom_mage_icelance_aoe'),
-- Mage Frost: Water Elemental permanent (on Summon Water Elemental)
(31687, 'spell_custom_mage_permanent_water_ele'),
-- Mage Frost: Comet Shower (active spell)
(900771, 'spell_custom_mage_comet_shower'),
-- Warlock Affliction: DoT ticks deal Shadow AoE (proc aura)
(900800, 'spell_custom_wlk_dot_aoe'),
-- Warlock Affliction: DoT ticks spread to 2 additional targets (proc aura)
(900802, 'spell_custom_wlk_dot_spread'),
-- Warlock Demonology: Meta periodic AoE+Heal (own aura script)
(900834, 'spell_custom_wlk_meta_aoe_heal'),
-- Warlock Demonology: Imp Firebolt +9 targets (hooked on all ranks)
(-47964, 'spell_custom_wlk_imp_fb_aoe'),
-- Warlock Demonology: Felguard Cleave unlimited targets
(47994, 'spell_custom_wlk_fg_unlim'),
-- Warlock Demonology: Demonic Sacrifice grants ALL bonuses
(18788, 'spell_custom_wlk_sacrifice_all'),
-- Warlock Destruction: Shadow Bolt +9 targets (hooked on all ranks)
(-47809, 'spell_custom_wlk_sb_aoe'),
-- Warlock Destruction: Chaos Bolt +9 targets (hooked on all ranks)
(-59172, 'spell_custom_wlk_cb_aoe'),
-- Priest Discipline: Shields explode on break/fade (hooked on PW:S all ranks)
(-48066, 'spell_custom_pri_shield_explode'),
-- Priest Holy: Direct heals → Holy Fire on enemies (proc aura)
(900933, 'spell_custom_pri_heal_fire'),
-- Priest Shadow: DoT ticks → Shadow AoE (proc aura)
(900966, 'spell_custom_pri_dot_aoe'),
-- Priest Shadow: DoT ticks → Spread to 2 targets (proc aura)
(900967, 'spell_custom_pri_dot_spread'),
-- Global: Kill enemy → heal 5% HP (proc aura)
(901101, 'spell_custom_global_kill_heal'),
-- Global: Extra Attack 25% (proc aura)
(901102, 'spell_custom_global_extra_attack'),
-- Global: 10% chance cleave to all enemies (proc aura)
(901103, 'spell_custom_global_cleave_proc'),
-- Global: Avoid attack → counter attack (proc aura)
(901104, 'spell_custom_global_counter_attack');
-- 901100: Cast while moving → DBC only (SPELL_AURA_CAST_WHILE_WALKING)
-- 900901: Shields +50% → DBC only
-- 900902: Weakened Soul 5s → DBC override on existing spell
-- 900867: Shadow Bolt +50% → DBC only
-- 900868: Chaos Bolt +50% → DBC only
-- 900869: Chaos Bolt CD -2s → DBC only
-- 900833: Meta Kill Extend → PlayerScript (no spell_script_names needed)
-- 900835: Lesser Demon Spawn → UnitScript (no spell_script_names needed)
-- 900836: Imp Firebolt +50% → DBC only
-- 900839: Felguard +50% → DBC only
-- 900500: Get back arrows → PlayerScript (no spell_script_names needed)
-- 900501: Multi-Shot AoE → checked via HasAura (hooked on Multi-Shot via -49048 above)
-- 900502/900503/900504: Pet passives → UnitScript/PlayerScript (no spell_script_names needed)
-- 900533: Auto Shot bounce → checked via HasAura (hooked on Auto Shot via 75 above)
-- 901067/901068/901069/901072: Druid Resto → PlayerScript/UnitScript (no spell_script_names needed)
-- 901001/901002/901003/901005/901051/901070/901071: DBC-only passives
-- 900433/900466: Totem follow → PlayerScript (no spell_script_names needed)
-- 900435: Summons +50% damage → DBC-only
-- 900437/900438: Wolf haste/CL → checked via HasAura / UnitScript
-- 900467: Mana regen → PlayerScript (no spell_script_names needed)
-- 900406: LvB charges checked via HasAura (hooked on LvB via -51505 above)
-- 900407: Clearcasting → instant LvB (DBC-only, no script needed)
-- 900408: CL AoE helper (no script needed, pure DBC damage)
-- 900400, 900402-900404, 900406-900408: marker auras / DBC-only, no script registration needed

-- ICD for Whirlwind proc aura (900138): 500ms cooldown to prevent
-- recursive loop (Whirlwind hits count as auto-attacks → re-proc → crash)
DELETE FROM `spell_proc` WHERE `SpellId` = 900138;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900138, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 500, 0);

-- Bladestorm CD Reduce passive (900107): procs on melee damage dealt.
-- ProcFlags=0x14 (DONE_MELEE_AUTO_ATTACK | DONE_SPELL_MELEE_DMG_CLASS).
-- C++ CheckProc filters: only when Bladestorm is on cooldown.
DELETE FROM `spell_proc` WHERE `SpellId` = 900107;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900107, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Bloody Whirlwind passive (900140): C++ AuraScript handles Bloodthirst filtering.
-- ProcFlags=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS) matches Bloodthirst's melee dmg class.
DELETE FROM `spell_proc` WHERE `SpellId` = 900140;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900140, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Speedy Bloodthirst (900141): Whirlwind resets Bloodthirst CD.
-- ProcFlags=0x10 (PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS), SpellPhaseMask=4 (FINISH).
DELETE FROM `spell_proc` WHERE `SpellId` = 900141;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900141, 0, 0, 0, 0, 0, 0x10, 1, 4, 0, 0, 0, 0, 100, 0, 0);

-- ============================================================
-- Warrior Prot: spell_proc entries
-- ============================================================

-- Block → AoE damage (900172): procs on TAKEN_MELEE_AUTO_ATTACK (0x2).
-- C++ HandleProc checks HitMask for BLOCK. 100% chance, 1s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900172;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900172, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 100, 1000, 0);

-- Block → Enhanced TC (900173): procs on TAKEN_MELEE_AUTO_ATTACK (0x2).
-- C++ HandleProc checks HitMask for BLOCK. 10% chance, 3s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900173;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900173, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 10, 3000, 0);

-- ============================================================
-- Warrior Prot: spell_dbc entries (900168-900175)
-- ============================================================

DELETE FROM `spell_dbc` WHERE `ID` IN (900168, 900169, 900170, 900171, 900172, 900173, 900174, 900175);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900168: Revenge +50% damage passive (SPELL_AURA_ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA_1=0x400 targets Revenge SpellFamilyFlags[0]
(900168, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x400, 4, 132, 'Prot: Revenge Damage', 0x003F3F),
-- 900169: Revenge AoE passive (marker aura, logic in C++)
(900169, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Revenge AoE', 0x003F3F),
-- 900170: TC → Rend + Sunder passive (marker aura, logic in C++)
(900170, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: TC Rend Sunder', 0x003F3F),
-- 900171: Thunderclap +50% damage passive (SPELL_AURA_ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA_1=0x80 targets Thunderclap SpellFamilyFlags[0]
(900171, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x80, 4, 132, 'Prot: TC Damage', 0x003F3F),
-- 900172: Block → AoE damage proc passive (SPELL_AURA_DUMMY)
(900172, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block AoE', 0x003F3F),
-- 900173: Block → Enhanced TC proc passive (SPELL_AURA_DUMMY)
(900173, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 4, 132, 'Prot: Block TC', 0x003F3F),
-- 900174: Helper - Block AoE damage (instant AoE physical, 8yd radius)
(900174, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 100, 500, 22, 0, 0, 0, 0, 4, 132, 'Block Shield Burst', 0x003F3F),
-- 900175: Helper - Enhanced Thunderclap (instant AoE physical, 10yd radius)
(900175, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 1000, 22, 0, 0, 0, 0, 4, 132, 'Enhanced Thunderclap', 0x003F3F);

-- ============================================================
-- Paladin Holy: spell_dbc entries (900200-900210)
-- ============================================================
-- SpellFamilyName=10 (Paladin)
-- Holy Shock SpellFamilyFlags[0]: 0x200000
-- Consecration SpellFamilyFlags[0]: 0x20 (verify in-game!)

DELETE FROM `spell_dbc` WHERE `ID` IN (900200, 900201, 900202, 900203, 900204, 900205, 900206, 900207, 900208, 900209, 900210);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900200: Holy Shock AoE damage passive (marker, C++ on -25912)
(900200, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Damage', 0x003F3F),
-- 900201: Holy Shock AoE heal passive (marker, C++ on -25914)
(900201, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS AoE Heal', 0x003F3F),
-- 900202: Holy Shock always both passive (marker, C++ on -25912 and -25914)
(900202, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 156, 0, 'Holy: HS Both', 0x003F3F),
-- 900203: Holy Shock +50% damage/healing (DBC passive)
-- EffectSpellClassMaskA=0x200000 targets Holy Shock family
(900203, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x200000, 10, 156, 0, 'Holy: HS +50%', 0x003F3F),
-- 900204: Consecration also heals passive (marker, C++ on 48819)
(900204, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 51, 0, 'Holy: Consec Heal', 0x003F3F),
-- 900205: Consecration around you (DBC passive)
-- This is a marker; actual "follow caster" needs DBC edit on Consecration itself
-- or a C++ approach. For now: marker aura (DBC change on base spell needed separately)
(900205, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 10, 51, 0, 'Holy: Consec Around', 0x003F3F),
-- 900206: Consecration +50% damage/heal (DBC passive)
-- EffectSpellClassMaskA=0x20 targets Consecration family (verify!)
(900206, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x20, 10, 51, 0, 'Holy: Consec +50%', 0x003F3F),
-- 900207: Consecration +5sec duration (DBC passive)
-- ADD_FLAT_MODIFIER (107), SPELLMOD_DURATION (17), BasePoints=5000 (ms)
-- EffectSpellClassMaskA=0x20 targets Consecration
(900207, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 5000, 1, 107, 17, 0, 0x20, 10, 51, 0, 'Holy: Consec +5s', 0x003F3F),
-- 900208: Helper - Holy Shock AoE damage (instant, Holy school, 10yd around target)
-- SchoolMask=2 (Holy), TARGET_UNIT_DEST_AREA_ENEMY (15)
(900208, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 100, 800, 15, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Burst', 0x003F3F),
-- 900209: Helper - Holy Shock AoE heal (instant, Holy school, 10yd around target)
-- SchoolMask=2 (Holy), TARGET_UNIT_DEST_AREA_ALLY (30)
(900209, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 100, 800, 30, 0, 0, 0, 0, 10, 156, 2, 'Holy Shock Radiance', 0x003F3F),
-- 900210: Helper - Consecration heal tick (instant AoE heal around caster)
-- Effect=HEAL(10), TARGET_UNIT_SRC_AREA_ALLY (31), 8yd radius
(900210, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 50, 200, 31, 0, 0, 0, 0, 10, 51, 2, 'Consecration Heal', 0x003F3F);

-- ============================================================
-- Paladin Prot: spell_dbc entries (900234-900241)
-- ============================================================
-- Avenger's Shield SpellFamilyFlags[0]: 0x4000 (verify in-game!)
-- Holy Shield SpellFamilyFlags[1]: 0x20 (verify in-game!)
-- Judgement SpellFamilyFlags[0]: 0x800000 (verify in-game!)

DELETE FROM `spell_dbc` WHERE `ID` IN (900234, 900235, 900236, 900237, 900238, 900239, 900240, 900241);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900234: Consecration around you (marker aura, shared concept with Holy 900205)
(900234, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 51, 0, 'PProt: Consec Around', 0x003F3F),
-- 900235: Avenger's Shield +9 jump targets (ADD_FLAT_MODIFIER + SPELLMOD_JUMP_TARGETS=17)
-- EffectSpellClassMaskA=0x4000 targets Avenger's Shield. BasePoints=9.
(900235, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x4000, 0, 10, 3477, 0, 'PProt: AS +9 Targets', 0x003F3F),
-- 900236: Avenger's Shield +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE=0)
-- EffectSpellClassMaskA=0x4000 targets Avenger's Shield.
(900236, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x4000, 0, 10, 3477, 0, 'PProt: AS +50%', 0x003F3F),
-- 900237: Holy Shield charges +99 (ADD_FLAT_MODIFIER + SPELLMOD_CHARGES=4)
-- EffectSpellClassMaskB=0x20 targets Holy Shield (flags[1]).
(900237, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 99, 1, 107, 4, 0, 0, 0x20, 10, 293, 0, 'PProt: HS +99 Charges', 0x003F3F),
-- 900238: Holy Shield +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE=0)
-- EffectSpellClassMaskB=0x20 targets Holy Shield (flags[1]).
(900238, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20, 10, 293, 0, 'PProt: HS +50%', 0x003F3F),
-- 900239: AS leaves Consecration (marker aura, logic in C++ on -48827)
(900239, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 3477, 0, 'PProt: AS Consec', 0x003F3F),
-- 900240: Judgement → free AS (marker aura, logic in C++ on 54158)
(900240, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 3015, 0, 'PProt: Judge AS', 0x003F3F),
-- 900241: Judgement cd -2sec (ADD_FLAT_MODIFIER + SPELLMOD_COOLDOWN=11)
-- EffectSpellClassMaskA=0x800000 targets Judgement spells. BasePoints=-2000 (ms).
(900241, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 10, 3015, 0, 'PProt: Judge -2s CD', 0x003F3F);

-- ============================================================
-- Paladin Ret: spell_proc entry
-- ============================================================

-- Exorcism buff proc (900274): procs on melee spell damage (CS/Judge/DS).
-- ProcFlags=0x10 (DONE_SPELL_MELEE_DMG_CLASS). C++ CheckProc filters to CS/Judge/DS.
DELETE FROM `spell_proc` WHERE `SpellId` = 900274;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900274, 0, 0, 0, 0, 0, 0x10, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- ============================================================
-- Paladin Ret: spell_dbc entries (900268-900275)
-- ============================================================
-- Divine Storm SpellFamilyFlags: flags[1]=0x20000 (verify in-game!)
-- Crusader Strike SpellFamilyFlags: flags[0]=0x1 (verify in-game!)
-- Exorcism SpellFamilyFlags: flags[0]=0x200000 (same bit as HS? verify!)
-- Judgement SpellFamilyFlags[0]: 0x800000

DELETE FROM `spell_dbc` WHERE `ID` IN (900268, 900269, 900270, 900271, 900272, 900273, 900274, 900275);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900268: Consecration around you (marker, shared with Holy/Prot)
(900268, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 51, 0, 0, 'Ret: Consec Around', 0x003F3F),
-- 900269: Judgement cd -2sec (same as Prot 900241 but separate ID for Ret)
(900269, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -2000, 1, 107, 11, 0, 0x800000, 0, 10, 3015, 0, 0, 'Ret: Judge -2s CD', 0x003F3F),
-- 900270: Divine Storm +6 targets (marker aura; DS base spell needs MaxAffectedTargets DBC patch)
(900270, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 2292, 0, 0, 'Ret: DS +6 Targets', 0x003F3F),
-- 900271: Divine Storm +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- DS SpellFamilyFlags[1]=0x20000 (verify!)
(900271, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x20000, 10, 2292, 0, 0, 'Ret: DS +50%', 0x003F3F),
-- 900272: Crusader Strike +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- CS SpellFamilyFlags[0]=0x1 (verify!)
(900272, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 10, 2286, 0, 0, 'Ret: CS +50%', 0x003F3F),
-- 900273: CS +9 targets (marker aura, logic in C++ on -35395)
(900273, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 2286, 0, 0, 'Ret: CS AoE', 0x003F3F),
-- 900274: Exorcism buff proc passive (DUMMY, proc via spell_proc + C++)
(900274, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 10, 2286, 0, 0, 'Ret: Exorcism Proc', 0x003F3F),
-- 900275: Exorcism buff (stacking, +50% Exorcism damage per stack, max 10)
-- ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0), SpellClassMaskA for Exorcism
-- Duration: 30sec (DurationIndex=5), stackable (CumulativeAura=10)
-- NOT passive — it's a visible buff with stacks
(900275, 0x10000000, 0, 0, 0, 1, 5, 1, 6, 0, 50, 1, 108, 0, 0, 0x200000, 0, 10, 879, 0, 10, 'Exorcism Power', 0x003F3F);

-- ============================================================
-- DK Blood: spell_proc entries
-- ============================================================

-- Death Coil proc (900304): procs on melee + spell damage dealt.
-- ProcFlags=0x14 (DONE_MELEE_AUTO_ATTACK | DONE_SPELL_MELEE_DMG_CLASS).
-- 15% chance, 3s ICD to prevent spam.
DELETE FROM `spell_proc` WHERE `SpellId` = 900304;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900304, 0, 0, 0, 0, 0, 0x14, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- ============================================================
-- DK Unholy: spell_proc entries
-- ============================================================

-- DoT → Shadow AoE proc (900366): procs on periodic damage.
-- ProcFlags=0x400000 (DONE_PERIODIC). 20% chance, 2s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900366;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900366, 0, 0, 0, 0, 0, 0x400000, 1, 0, 0, 0, 0, 0, 20, 2000, 0);

-- ============================================================
-- DK: spell_dbc entries (900300-900304, 900333, 900366-900367)
-- ============================================================
-- SpellFamilyName=15 (DK)
-- Heart Strike SpellFamilyFlags: verify in Spell.dbc!

DELETE FROM `spell_dbc` WHERE `ID` IN (900300, 900301, 900302, 900303, 900304, 900333, 900366, 900367);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900300: 3 Rune Weapons (marker, C++ on 49028)
(900300, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 3735, 0, 0, 'DKB: 3 Rune Weapons', 0x003F3F),
-- 900301: Rune Weapon double casts (marker, C++ on 49028)
(900301, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 3735, 0, 0, 'DKB: Double Cast', 0x003F3F),
-- 900302: Heart Strike +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- Heart Strike SpellFamilyFlags[0]=0x2000000 (verify!)
(900302, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x2000000, 0, 15, 3547, 0, 0, 'DKB: HS +50%', 0x003F3F),
-- 900303: Heart Strike +9 targets (marker, C++ on -55050)
(900303, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 3547, 0, 0, 'DKB: HS AoE', 0x003F3F),
-- 900304: Death Coil proc (DUMMY, proc via spell_proc + C++)
(900304, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 136, 0, 0, 'DKB: DC Proc', 0x003F3F),
-- 900333: Replace Ghoul with Frost Wyrm (marker, C++ on 46584)
(900333, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 3223, 0, 0, 'DKF: Frost Wyrm', 0x003F3F),
-- 900366: DoT → Shadow AoE proc (DUMMY, proc via spell_proc + C++)
(900366, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 15, 2770, 0, 0, 'DKU: DoT AoE Proc', 0x003F3F),
-- 900367: Shadow AoE helper (instant AoE Shadow damage around target)
-- Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), SchoolMask=32(Shadow), 8yd radius
(900367, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 150, 600, 15, 0, 0, 0, 0, 0, 15, 2770, 32, 0, 'Shadow Eruption', 0x003F3F);

-- 900368: Frost Breath (2s cast, cone 20yd, Frost damage + slow)
-- Effect1=SCHOOL_DAMAGE(2), Target=CONE_ENEMY(24), SchoolMask=16(Frost)
-- Effect2=APPLY_AURA(6) AURA_MOD_DECREASE_SPEED(11) -50% for 6s
-- CastingTimeIndex=16 (2000ms), DurationIndex=18 (6s), RangeIndex=4 (30yd)
DELETE FROM `spell_dbc` WHERE `ID` = 900368;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`,
    `CastingTimeIndex`, `DurationIndex`, `RangeIndex`,
    `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`,
    `EffectAura_1`, `EffectMiscValue_1`, `EffectRadiusIndex_1`,
    `Effect_2`, `EffectDieSides_2`, `EffectBasePoints_2`, `ImplicitTargetA_2`,
    `EffectAura_2`, `EffectMiscValue_2`, `EffectRadiusIndex_2`,
    `SpellClassSet`, `SpellIconID`, `SchoolMask`,
    `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900368, 0x10000000, 0, 0, 0,
 16, 18, 4,
 2, 1000, 5000, 24,
 0, 0, 15,
 6, 0, -50, 24,
 11, 0, 15,
 15, 3223, 16,
 'Frost Breath', 0x003F3F);

-- ============================================================
-- DK Frost: Frost Wyrm creature_template (NPC 900333)
-- Stats: 2× Gargoyle HP, Level 80, follows + casts Frost Breath
-- ============================================================
DELETE FROM `creature_template` WHERE `entry` = 900333;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`,
    `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`,
    `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`,
    `dynamicflags`, `family`,
    `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`,
    `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`,
    `HealthModifier`, `ManaModifier`, `ArmorModifier`,
    `ExperienceModifier`, `RacialLeader`, `movementId`,
    `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`,
    `ScriptName`, `VerifiedBuild`) VALUES
(900333, 0, 0, 0,
 0, 0, 'Frost Wyrm', '', '', 0,
 80, 80, 2, 14, 0, 1, 1.14286,
 20, 0.5, 1, 4, 1, 2000,
 2000, 1, 1, 1, 0, 2048,
 0, 0,
 6, 12288, 0, 0, 0,
 0, 0, 0, 0,
 '', 0, 1,
 2, 1, 1,
 0, 0, 0,
 1, 8388624, 0, 0,
 'npc_custom_frost_wyrm', 12340);

-- Frost Wyrm display model (Sindragosa-style, scaled down)
DELETE FROM `creature_template_model` WHERE `CreatureID` = 900333;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900333, 0, 26752, 1, 1, 12340);

-- ============================================================
-- Shaman Ele: spell_proc entries
-- ============================================================

-- Flame Shock ticks → reset LvB CD (900405)
-- ProcFlags=0x400000 (DONE_PERIODIC), 15% chance, 2s ICD
DELETE FROM `spell_proc` WHERE `SpellId` = 900405;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900405, 0, 11, 0x10000000, 0, 0, 0x400000, 1, 0, 0, 0, 0, 0, 15, 2000, 0);

-- ============================================================
-- Shaman Ele: spell_dbc entries (900400-900408)
-- SpellFamilyName=11 (Shaman)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900400, 900401, 900402, 900403, 900404, 900405, 900406, 900407, 900408);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900400: CL +6 targets (marker, C++ on -49271)
(900400, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 62, 0, 0, 'Ele: CL AoE', 0x003F3F),
-- 900401: Totems follow (DUMMY marker, PlayerScript handles logic via OnUpdate)
(900401, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Ele: Totem Follow', 0x003F3F),
-- 900402: Ragnaros (marker, C++ on 2894)
(900402, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 547, 0, 0, 'Ele: Ragnaros', 0x003F3F),
-- 900403: Lightning Overload + LvB (marker, C++ on -51505)
(900403, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 2018, 0, 0, 'Ele: LO + LvB', 0x003F3F),
-- 900404: LvB spreads FS (marker, C++ on -51505)
(900404, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 0, 'Ele: LvB Spread FS', 0x003F3F),
-- 900405: FS ticks → reset LvB CD (DUMMY, proc via spell_proc + C++)
(900405, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 0, 'Ele: FS Reset LvB', 0x003F3F),
-- 900406: LvB two charges (DUMMY marker + stacking, CumulativeAura=2)
(900406, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 64, 0, 2, 'Ele: LvB Charges', 0x003F3F),
-- 900407: Clearcasting → LvB instant (ADD_PCT_MODIFIER + SPELLMOD_CASTING_TIME -100%)
-- Lava Burst SpellFamilyFlags need verification! Using 0x1000 as estimate
(900407, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -100, 1, 108, 14, 0, 0x1000, 0, 11, 16164, 0, 0, 'Ele: Instant LvB', 0x003F3F),
-- 900408: CL AoE damage helper (instant Nature damage)
(900408, 0x10000000, 0, 0, 0, 1, 0, 4, 2, 0, 0, 6, 0, 0, 0, 0, 0, 11, 62, 8, 0, 'Chain Lightning Arc', 0x003F3F);

-- ============================================================
-- Shaman Enhance: spell_proc entries
-- ============================================================

-- Wolf summon proc (900436): 10% on melee auto attack, 5s ICD
DELETE FROM `spell_proc` WHERE `SpellId` = 900436;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900436, 0, 0, 0, 0, 0, 0x4, 1, 2, 0, 0, 0, 0, 10, 5000, 0);

-- ============================================================
-- Shaman Enhance + Resto: spell_dbc entries
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900433, 900434, 900435, 900436, 900437, 900438, 900439, 900440, 900466, 900467);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900433: Totem follow Enhance (DUMMY marker)
(900433, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Enh: Totem Follow', 0x003F3F),
-- 900434: Maelstrom AoE (DUMMY marker, C++ on 53817)
(900434, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Maelstrom AoE', 0x003F3F),
-- 900435: Summons +50% damage (ADD_PCT_MODIFIER not directly applicable to pets via DBC)
-- Use DUMMY marker; C++ or pet scaling handles the actual +50%
(900435, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Enh: Summons +50%', 0x003F3F),
-- 900436: Auto attack → summon wolf (DUMMY proc aura)
(900436, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Wolf Summon', 0x003F3F),
-- 900437: Spirit Wolves inherit haste (DUMMY marker, C++ on 51533)
(900437, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Enh: Wolf Haste', 0x003F3F),
-- 900438: Spirit Wolves CL proc (DUMMY marker, UnitScript)
(900438, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 62, 0, 0, 'Enh: Wolf CL', 0x003F3F),
-- 900439: Maelstrom empower buff (5s duration, visible buff)
(900439, 0x10000000, 0, 0, 0, 1, 18, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 3786, 0, 0, 'Maelstrom Fury', 0x003F3F),
-- 900440: Wolf AoE helper (instant Physical AoE around caster)
(900440, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 22, 0, 0, 0, 0, 0, 11, 3786, 1, 0, 'Spirit Howl', 0x003F3F),
-- 900466: Totem follow Resto (DUMMY marker)
(900466, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Resto: Totem Follow', 0x003F3F),
-- 900467: Mana regen per missing mana% (DUMMY marker, PlayerScript)
(900467, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 11, 136, 0, 0, 'Resto: Mana Regen', 0x003F3F);

-- ============================================================
-- Custom Wolf NPC for 900436 (auto attack wolf summon)
-- ============================================================
DELETE FROM `creature_template` WHERE `entry` = 900436;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`,
    `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`,
    `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`,
    `dynamicflags`, `family`,
    `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`,
    `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`,
    `HealthModifier`, `ManaModifier`, `ArmorModifier`,
    `ExperienceModifier`, `RacialLeader`, `movementId`,
    `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`,
    `ScriptName`, `VerifiedBuild`) VALUES
(900436, 0, 0, 0,
 0, 0, 'Spirit Wolf', '', '', 0,
 80, 80, 2, 14, 0, 1, 1.14286,
 20, 0.7, 0, 0, 0.5, 1500,
 2000, 1, 1, 1, 0, 2048,
 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 '', 0, 1,
 0.5, 0, 0.5,
 0, 0, 0,
 1, 0, 0, 0,
 '', 12340);

-- Spirit Wolf display model (same as Feral Spirit wolves)
DELETE FROM `creature_template_model` WHERE `CreatureID` = 900436;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(900436, 0, 27074, 1, 1, 12340);

-- ============================================================
-- Hunter: spell_proc entries
-- ============================================================

-- Explosive trap proc (900566): procs on ranged auto + ranged spell damage.
-- ProcFlags=0x44 (DONE_RANGED_AUTO_ATTACK 0x4 | DONE_SPELL_RANGED_DMG_CLASS 0x40).
-- 15% chance, 2s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900566;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900566, 0, 0, 0, 0, 0, 0x44, 1, 2, 0, 0, 0, 0, 15, 2000, 0);

-- ============================================================
-- Hunter: spell_dbc entries (900500-900567)
-- SpellFamilyName=9 (Hunter)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900500, 900501, 900502, 900503, 900504, 900505, 900533, 900534, 900535, 900536, 900566, 900567);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900500: Get back arrows (DUMMY marker, PlayerScript)
(900500, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Hunt: Get Back Arrows', 0x003F3F),
-- 900501: Multi-Shot unlimited targets (DUMMY marker, C++ on -49048)
(900501, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Hunt: Multishot AoE', 0x003F3F),
-- 900502: Pet damage +50% (DUMMY marker, UnitScript)
(900502, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet Damage +50%', 0x003F3F),
-- 900503: Pet attack speed +50% (DUMMY marker, PlayerScript)
(900503, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet Speed +50%', 0x003F3F),
-- 900504: Pet AoE proc (DUMMY marker, UnitScript)
(900504, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'BM: Pet AoE Proc', 0x003F3F),
-- 900505: Helper - Pet AoE damage (instant AoE physical, 8yd around target)
(900505, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 15, 0, 0, 0, 0, 0, 0, 9, 132, 1, 0, 'Beast Cleave', 0x003F3F),
-- 900533: Auto Shot bounces +9 (DUMMY marker, C++ on 75)
(900533, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'MM: Autoshot Bounce', 0x003F3F),
-- 900534: Multi-Shot Barrage (2s periodic, ticks every 100ms, PERIODIC_DUMMY)
-- DurationIndex=4 (2000ms=2s), EffectAuraPeriod_1=100 (100ms ticks)
(900534, 0x10000000, 0, 0, 0, 1, 4, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 100, 9, 132, 0, 0, 'MM: Barrage', 0x003F3F),
-- 900535: Helper - Auto Shot bounce damage (instant Physical single-target)
(900535, 0x10000000, 0, 0, 0, 1, 0, 4, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 9, 132, 1, 0, 'Ricochet Shot', 0x003F3F),
-- 900536: Helper - Barrage slow debuff (-50% movement, 2s duration)
-- Effect=APPLY_AURA(6), Aura=MOD_DECREASE_SPEED(11), BasePoints=-50
(900536, 0x10000000, 0, 0, 0, 1, 4, 1, 6, 0, -50, 1, 11, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Barrage Slow', 0x003F3F),
-- 900566: Explosive trap proc passive (DUMMY, proc via spell_proc + C++)
(900566, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 9, 132, 0, 0, 'Surv: Trap Proc', 0x003F3F),
-- 900567: Helper - Explosive burst (instant AoE Fire damage, 8yd around target)
-- SchoolMask=4 (Fire), Target=DEST_AREA_ENEMY(15)
(900567, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 1000, 15, 0, 0, 0, 0, 0, 0, 9, 132, 4, 0, 'Explosive Burst', 0x003F3F);

-- ============================================================
-- Druid Balance: spell_proc entries
-- ============================================================

-- Spell damage reduces Starfall CD (901004): procs on spell damage dealt.
-- ProcFlags=0x10000 (DONE_SPELL_MAGIC_DMG_CLASS_POS|NEG combined).
-- Actually: 0x10000 = DONE_SPELL_MAGIC_DMG_CLASS_NEG. Let's use 0x10010.
-- 100% chance, 1s ICD to prevent huge CD reduction from AoE.
DELETE FROM `spell_proc` WHERE `SpellId` = 901004;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901004, 0, 7, 0, 0, 0, 0x10010, 1, 2, 0, 0, 0, 0, 100, 1000, 0);

-- ============================================================
-- Druid Resto: spell_proc entries
-- ============================================================

-- HoTs summon treant (901066): procs on periodic healing done.
-- ProcFlags=0x40000 (DONE_PERIODIC). 5% chance, 5s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 901066;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(901066, 0, 7, 0, 0, 0, 0x40000, 2, 0, 0, 0, 0, 0, 5, 5000, 0);

-- ============================================================
-- Druid: spell_dbc entries (901000-901073)
-- SpellFamilyName=7 (Druid)
-- Moonfire SpellFamilyFlags[0] = 0x2 (verify!)
-- Starfall SpellFamilyFlags[0] = 0x100 (verify!)
-- Rejuvenation SpellFamilyFlags[0] = 0x10 (verify!)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (901000, 901001, 901002, 901003, 901004, 901005, 901033, 901034, 901049, 901050, 901051, 901066, 901067, 901068, 901069, 901070, 901071, 901072, 901073);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 901000: Moonfire +9 targets (DUMMY marker, C++ on -48463)
(901000, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Bal: MF +9 Targets', 0x003F3F),
-- 901001: Moonfire +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA=0x2 targets Moonfire family
(901001, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x2, 0, 0, 7, 132, 0, 0, 'Bal: MF +50%', 0x003F3F),
-- 901002: Starfall +9 targets (ADD_FLAT_MODIFIER + SPELLMOD_JUMP_TARGETS=17)
-- EffectSpellClassMaskA=0x100 targets Starfall family (verify!)
(901002, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0x100, 0, 0, 7, 132, 0, 0, 'Bal: SF +9 Targets', 0x003F3F),
-- 901003: Starfall +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
(901003, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x100, 0, 0, 7, 132, 0, 0, 'Bal: SF +50%', 0x003F3F),
-- 901004: Spell dmg reduces Starfall CD (DUMMY, proc via spell_proc + C++)
(901004, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Bal: SF CD Reduce', 0x003F3F),
-- 901005: Starfall stacks up to 10 (ADD_FLAT_MODIFIER + SPELLMOD_CHARGES=4)
-- EffectSpellClassMaskA=0x100 targets Starfall. BasePoints=9 (adds 9 charges → total 10).
(901005, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 4, 0, 0x100, 0, 0, 7, 132, 0, 0, 'Bal: SF Stacks 10', 0x003F3F),
-- 901033: Swipe Bear applies bleed (DUMMY marker, C++ on -48562)
(901033, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Bear Bleed', 0x003F3F),
-- 901034: Helper - Bear Swipe bleed DoT (Physical periodic damage, 12s, ticks every 3s)
-- Effect=APPLY_AURA(6), Aura=PERIODIC_DAMAGE(3), BasePoints=300, Duration=12s(idx=21 is perm, use 32=12s)
(901034, 0x10000000, 0, 0, 0, 1, 32, 1, 6, 50, 300, 6, 3, 0, 0, 0, 0, 3000, 7, 132, 1, 0, 'Swipe Bleed', 0x003F3F),
-- 901049: Swipe Cat applies bleed (DUMMY marker, C++ on 62078)
(901049, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Cat Bleed', 0x003F3F),
-- 901050: Helper - Cat Swipe bleed DoT (Physical periodic damage, 12s, ticks every 3s)
(901050, 0x10000000, 0, 0, 0, 1, 32, 1, 6, 50, 300, 6, 3, 0, 0, 0, 0, 3000, 7, 132, 1, 0, 'Rake Bleed', 0x003F3F),
-- 901051: Energy regen +50% (SPELL_AURA_MOD_POWER_REGEN_PERCENT, MiscValue=3 for Energy)
(901051, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 110, 3, 0, 0, 0, 0, 7, 132, 0, 0, 'Feral: Energy +50%', 0x003F3F),
-- 901066: HoTs summon treant (DUMMY, proc via spell_proc + C++)
(901066, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: HoT Treant', 0x003F3F),
-- 901067: Summons scale with healing power (DUMMY marker, PlayerScript)
(901067, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Summon Scale', 0x003F3F),
-- 901068: Summons heal on death (DUMMY marker, UnitScript)
(901068, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Summon Heal', 0x003F3F),
-- 901069: Thorns → Rejuv (DUMMY marker, UnitScript)
(901069, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Thorns Rejuv', 0x003F3F),
-- 901070: HoTs +50% healing (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE on HoT family)
-- Rejuv flags[0]=0x10, Regrowth flags[0]=0x20, Lifebloom flags[1]=0x10 (verify!)
-- We target the common HoT mask. Using broad flags to cover Rejuv+Regrowth+WG.
(901070, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x30, 0, 0, 7, 132, 0, 0, 'Resto: HoTs +50%', 0x003F3F),
-- 901071: HoTs tick 2x fast + 2x duration
-- This is TWO effects: (1) -50% tick interval via ADD_PCT_MODIFIER SPELLMOD_HASTE=22 (not standard)
-- Simpler: just double duration via ADD_PCT_MODIFIER + SPELLMOD_DURATION=17 (+100%)
-- and let the extra ticks naturally happen. DBC approach = duration +100%.
-- EffectSpellClassMaskA targets Rejuv+Regrowth+WG (0x30)
(901071, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 100, 1, 108, 17, 0, 0x30, 0, 0, 7, 132, 0, 0, 'Resto: HoTs 2x', 0x003F3F),
-- 901072: Mana regen per missing mana% (DUMMY marker, PlayerScript)
(901072, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 132, 0, 0, 'Resto: Mana Regen', 0x003F3F),
-- 901073: Helper - Treant heal on despawn (instant AoE Nature heal, 15yd around caster)
-- Effect=HEAL(10), Target=DEST_AREA_ALLY(30), SchoolMask=8(Nature)
(901073, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 500, 2000, 30, 0, 0, 0, 0, 0, 0, 7, 132, 8, 0, 'Nature Bloom', 0x003F3F);

-- ============================================================
-- Druid Resto: Custom Treant NPC (901066)
-- ============================================================
DELETE FROM `creature_template` WHERE `entry` = 901066;
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`,
    `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`,
    `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`,
    `dynamicflags`, `family`,
    `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`,
    `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`,
    `HealthModifier`, `ManaModifier`, `ArmorModifier`,
    `ExperienceModifier`, `RacialLeader`, `movementId`,
    `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`,
    `ScriptName`, `VerifiedBuild`) VALUES
(901066, 0, 0, 0,
 0, 0, 'Healing Treant', '', '', 0,
 80, 80, 2, 14, 0, 1, 1.14286,
 20, 0.8, 0, 0, 0.5, 2000,
 2000, 1, 1, 1, 0, 2048,
 0, 0,
 4, 0, 0, 0, 0,
 0, 0, 0, 0,
 '', 0, 1,
 0.5, 0, 0.5,
 0, 0, 0,
 1, 0, 0, 0,
 '', 12340);

-- Treant display model (Force of Nature treant model)
DELETE FROM `creature_template_model` WHERE `CreatureID` = 901066;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(901066, 0, 3889, 1, 1, 12340);

-- ============================================================
-- Rogue: spell_proc entries
-- ============================================================

-- Poison Nova proc (900603): procs on Nature spell damage (poison).
-- ProcFlags=0x10000 (DONE_SPELL_MAGIC_DMG_CLASS_NEG). 15% chance, 3s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900603;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900603, 8, 8, 0, 0, 0, 0x10000, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- ============================================================
-- Rogue: spell_dbc entries (900600-900669)
-- SpellFamilyName=8 (Rogue)
-- Mutilate SpellFamilyFlags[1]=0x200000 (verify!)
-- Sinister Strike SpellFamilyFlags[0]=0x2 (verify!)
-- Hemorrhage SpellFamilyFlags[0]=0x2000000 (verify!)
-- Blade Flurry SpellFamilyFlags[1]=0x800 (verify!)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900600, 900601, 900602, 900603, 900604, 900633, 900634, 900635, 900636, 900637, 900638, 900666, 900667, 900668, 900669);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- Assa: 900600 Energy regen +50% (MOD_POWER_REGEN_PERCENT, MiscValue=3 Energy)
(900600, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Assa: Energy +50%', 0x003F3F),
-- Assa: 900601 Mutilate +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- MutilateSpellFamilyFlags[1]=0x200000
(900601, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x200000, 0, 8, 132, 0, 0, 'Assa: Muti +50%', 0x003F3F),
-- Assa: 900602 Poison damage +50% (ADD_PCT_MODIFIER on poison school mask=8 Nature)
-- Poison spells share common flags. Using broad approach with SchoolMask filter in C++ if needed.
-- For DBC: target all Rogue Nature-school spells. SpellFamilyFlags for Instant Poison etc.
(900602, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x8000, 0x10000, 0, 8, 132, 0, 0, 'Assa: Poison +50%', 0x003F3F),
-- Assa: 900603 Poison Nova proc (DUMMY, proc via spell_proc + C++)
(900603, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 8, 132, 0, 0, 'Assa: Poison Nova', 0x003F3F),
-- Assa: 900604 Helper - Poison Nova AoE (instant AoE Nature damage, 8yd)
-- Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), SchoolMask=8(Nature)
(900604, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 15, 0, 0, 0, 0, 0, 0, 8, 132, 8, 0, 'Poison Nova', 0x003F3F),
-- Combat: 900633 SS +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- SS SpellFamilyFlags[0]=0x2
(900633, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x2, 0, 0, 8, 132, 0, 0, 'Combat: SS +50%', 0x003F3F),
-- Combat: 900634 SS +9 targets (DUMMY marker, C++ on -48638)
(900634, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 8, 132, 0, 0, 'Combat: SS AoE', 0x003F3F),
-- Combat: 900635 Blade Flurry 2min duration (ADD_FLAT_MODIFIER + SPELLMOD_DURATION=17)
-- BF SpellFamilyFlags[1]=0x800, BasePoints=+105000ms (15s base + 105s = 120s)
(900635, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 105000, 1, 107, 17, 0, 0, 0x800, 0, 8, 132, 0, 0, 'Combat: BF 2min', 0x003F3F),
-- Combat: 900636 Blade Flurry +9 targets (ADD_FLAT_MODIFIER + SPELLMOD_JUMP_TARGETS=17)
-- BF SpellFamilyFlags[1]=0x800, BasePoints=9
(900636, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 9, 1, 107, 17, 0, 0, 0x800, 0, 8, 132, 0, 0, 'Combat: BF +9 Targets', 0x003F3F),
-- Combat: 900637 Energy regen +50% (MOD_POWER_REGEN_PERCENT, MiscValue=3 Energy)
(900637, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Combat: Energy +50%', 0x003F3F),
-- Combat: 900638 Helper - SS bounce damage (instant Physical single-target)
(900638, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 8, 132, 1, 0, 'Sinister Slash', 0x003F3F),
-- Sub: 900666 Energy regen +50% (MOD_POWER_REGEN_PERCENT, MiscValue=3 Energy)
(900666, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 110, 3, 0, 0, 0, 0, 8, 132, 0, 0, 'Sub: Energy +50%', 0x003F3F),
-- Sub: 900667 Hemorrhage +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- Hemorrhage SpellFamilyFlags[0]=0x2000000
(900667, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x2000000, 0, 0, 8, 132, 0, 0, 'Sub: Hemo +50%', 0x003F3F),
-- Sub: 900668 Hemorrhage +9 targets (DUMMY marker, C++ on -48660)
(900668, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 8, 132, 0, 0, 'Sub: Hemo AoE', 0x003F3F),
-- Sub: 900669 Helper - Hemorrhage bounce damage (instant Physical single-target)
(900669, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 8, 132, 1, 0, 'Deep Cut', 0x003F3F);

-- ============================================================
-- Mage Arcane: spell_proc entries
-- ============================================================

-- Emergency Mana Shield (900708): procs on TAKEN_DAMAGE (0x100000).
-- C++ HandleProc checks health < 30%. 100% chance, 60s ICD.
DELETE FROM `spell_proc` WHERE `SpellId` = 900708;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(900708, 0, 0, 0, 0, 0, 0x100000, 0, 0, 0, 0, 0, 0, 100, 60000, 0);

-- ============================================================
-- Mage Arcane: spell_dbc entries (900700-900713)
-- SpellFamilyName=3 (Mage)
-- Arcane Barrage SpellFamilyFlags[1]=0x1000000 (verify!)
-- Arcane Blast SpellFamilyFlags[0]=0x20000000 (verify!)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900700, 900701, 900702, 900703, 900704, 900705, 900706, 900707, 900708, 900709, 900710, 900711, 900712, 900713);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900700: Mana regen per missing mana% (DUMMY marker, PlayerScript)
(900700, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Mana Regen', 0x003F3F),
-- 900701: Arcane Barrage +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskB=0x1000000 targets Arcane Barrage (verify!)
(900701, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x1000000, 0, 3, 33, 0, 0, 'Arcane: Barrage +50%', 0x003F3F),
-- 900702: Arcane Barrage +9 targets (DUMMY marker, C++ on -44781)
(900702, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Barrage AoE', 0x003F3F),
-- 900703: Arcane Blast cast time -50% (ADD_PCT_MODIFIER + SPELLMOD_CASTING_TIME=14)
-- EffectSpellClassMaskA=0x20000000 targets Arcane Blast (verify!)
(900703, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -50, 1, 108, 14, 0, 0x20000000, 0, 0, 3, 33, 0, 0, 'Arcane: Blast -50% Cast', 0x003F3F),
-- 900704: Arcane Blast +9 targets (DUMMY marker, C++ on -42897)
(900704, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Blast AoE', 0x003F3F),
-- 900705: Arcane Charges stack to 8 (DUMMY marker, C++ on -42897)
(900705, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Charges x8', 0x003F3F),
-- 900706: Arcane Explosion generates charge (DUMMY marker, C++ on -42921)
(900706, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: AE Charges', 0x003F3F),
-- 900707: Evocation increases spell damage (DUMMY marker, AuraScript on 12051)
(900707, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Evoc Power', 0x003F3F),
-- 900708: Emergency Mana Shield (DUMMY, proc via spell_proc + C++)
(900708, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 33, 0, 0, 'Arcane: Emergency Shield', 0x003F3F),
-- 900709: Blink target location (DUMMY marker)
(900709, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 258, 0, 0, 'Arcane: Blink Target', 0x003F3F),
-- 900710: Helper - Arcane Barrage bounce (instant Arcane single-target damage)
-- BasePoints via CastCustomSpell. SchoolMask=64 (Arcane)
(900710, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 33, 64, 0, 'Arcane Barrage Bounce', 0x003F3F),
-- 900711: Helper - Arcane Blast bounce (instant Arcane single-target damage)
-- BasePoints via CastCustomSpell. SchoolMask=64 (Arcane)
(900711, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 33, 64, 0, 'Arcane Blast Bounce', 0x003F3F),
-- 900712: Evocation Power buff (+20% spell damage per stack, max 4, 60s duration)
-- Effect=APPLY_AURA(6), Aura=MOD_DAMAGE_PERCENT_DONE(79), MiscValue=127(all schools)
-- DurationIndex=3 (60s), CumulativeAura=4
(900712, 0x10000000, 0, 0, 0, 1, 3, 1, 6, 0, 20, 1, 79, 127, 0, 0, 0, 0, 3, 33, 0, 4, 'Evocation Power', 0x003F3F),
-- 900713: Targeted Blink (active spell, ground targeting, 40yd range)
-- Effect=DUMMY(3), ImplicitTargetA=16 (ground targeting), RangeIndex=5 (40yd)
-- SpellIconID=258 (Blink icon). Needs client DBC patch for ground cursor.
(900713, 0x10000000, 0, 0, 0, 1, 0, 5, 3, 0, 0, 16, 0, 0, 0, 0, 0, 0, 3, 258, 0, 0, 'Targeted Blink', 0x003F3F);

-- ============================================================
-- Mage Fire: spell_dbc entries (900733-900740)
-- SpellFamilyName=3 (Mage)
-- Fireball SpellFamilyFlags[0]=0x1 (verify!)
-- Pyroblast SpellFamilyFlags[0]=0x400000 (verify!)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900733, 900734, 900735, 900736, 900737, 900738, 900739, 900740);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900733: Fireball +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA=0x1 targets Fireball (verify!)
(900733, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 0, 3, 185, 4, 0, 'Fire: Fireball +50%', 0x003F3F),
-- 900734: Fireball +9 targets (DUMMY marker, C++ on -42833)
(900734, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 185, 0, 0, 'Fire: Fireball AoE', 0x003F3F),
-- 900735: Pyroblast +9 targets (DUMMY marker, C++ on -42891)
(900735, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 1726, 0, 0, 'Fire: Pyro AoE', 0x003F3F),
-- 900736: Pyroblast +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA=0x400000 targets Pyroblast (verify!)
(900736, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x400000, 0, 0, 3, 1726, 4, 0, 'Fire: Pyro +50%', 0x003F3F),
-- 900737: Fire Blast off GCD + cast while casting (DUMMY marker)
-- NOTE: Actual behavior requires DBC patch on Fire Blast (42873):
--   StartRecoveryCategory=0, AttributesEx4 |= SPELL_ATTR4_CAN_CAST_WHILE_CASTING
(900737, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 12, 0, 0, 'Fire: Blast Off GCD', 0x003F3F),
-- 900738: Pyroblast triggers Hot Streak (DUMMY marker, C++ on -42891)
(900738, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 1726, 0, 0, 'Fire: Pyro Hot Streak', 0x003F3F),
-- 900739: Helper - Fireball bounce (instant Fire single-target damage)
-- BasePoints via CastCustomSpell. SchoolMask=4 (Fire)
(900739, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 185, 4, 0, 'Fireball Bounce', 0x003F3F),
-- 900740: Helper - Pyroblast bounce (instant Fire single-target damage)
-- BasePoints via CastCustomSpell. SchoolMask=4 (Fire)
(900740, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 1726, 4, 0, 'Pyroblast Bounce', 0x003F3F);

-- ============================================================
-- Mage Frost: spell_dbc entries (900766-900774)
-- SpellFamilyName=3 (Mage)
-- Frostbolt SpellFamilyFlags[0]=0x20 (verify!)
-- Ice Lance SpellFamilyFlags[0]=0x20000 (verify!)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900766, 900767, 900768, 900769, 900770, 900771, 900772, 900773, 900774);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900766: Frostbolt +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA=0x20 targets Frostbolt (verify!)
(900766, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x20, 0, 0, 3, 188, 16, 0, 'Frost: Frostbolt +50%', 0x003F3F),
-- 900767: Frostbolt +9 targets (DUMMY marker, C++ on -42842)
(900767, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 188, 0, 0, 'Frost: Frostbolt AoE', 0x003F3F),
-- 900768: Ice Lance +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA=0x20000 targets Ice Lance (verify!)
(900768, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x20000, 0, 0, 3, 2723, 16, 0, 'Frost: Ice Lance +50%', 0x003F3F),
-- 900769: Ice Lance +9 targets (DUMMY marker, C++ on -42914)
(900769, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 2723, 0, 0, 'Frost: Ice Lance AoE', 0x003F3F),
-- 900770: Water Elemental permanent (DUMMY marker, C++ on 31687)
(900770, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 3, 2735, 0, 0, 'Frost: Perm Elemental', 0x003F3F),
-- 900771: Comet Shower (active spell, ground targeting, 40yd range, instant)
-- Effect=DUMMY(3), ImplicitTargetA=16 (ground targeting), RangeIndex=5 (40yd)
-- Needs client DBC patch for ground cursor.
(900771, 0x10000000, 0, 0, 0, 1, 0, 5, 3, 0, 0, 16, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frost Comet Shower', 0x003F3F),
-- 900772: Helper - Frostbolt bounce (instant Frost single-target damage)
-- BasePoints via CastCustomSpell. SchoolMask=16 (Frost)
(900772, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frostbolt Bounce', 0x003F3F),
-- 900773: Helper - Ice Lance bounce (instant Frost single-target damage)
-- BasePoints via CastCustomSpell. SchoolMask=16 (Frost)
(900773, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 3, 2723, 16, 0, 'Ice Lance Bounce', 0x003F3F),
-- 900774: Helper - Comet Impact (instant Frost single-target damage, high base)
-- SchoolMask=16 (Frost). Base damage for the comet impact.
(900774, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 500, 3000, 6, 0, 0, 0, 0, 0, 0, 3, 188, 16, 0, 'Frost Comet', 0x003F3F);

-- ============================================================
-- Warlock Affliction: spell_proc entries
-- ============================================================

-- DoT AoE proc (900800): procs on DONE_PERIODIC (0x40000).
-- 20% chance, 2s ICD. C++ casts Shadow AoE helper on target.
DELETE FROM `spell_proc` WHERE `SpellId` IN (900800, 900802);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
-- 900800: DoT tick → Shadow AoE. ProcFlags=0x40000 (DONE_PERIODIC), 20% chance, 2s ICD.
(900800, 0, 5, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 20, 2000, 0),
-- 900802: DoT tick → Spread to 2. ProcFlags=0x40000 (DONE_PERIODIC), 15% chance, 3s ICD.
(900802, 0, 5, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- ============================================================
-- Warlock Affliction: spell_dbc entries (900800-900803)
-- SpellFamilyName=5 (Warlock)
-- Corruption SpellFamilyFlags[0]=0x2 (verify!)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900800, 900801, 900802, 900803);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900800: DoT ticks → Shadow AoE (DUMMY, proc via spell_proc + C++)
(900800, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Affl: DoT AoE', 0x003F3F),
-- 900801: Corruption +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA=0x2 targets Corruption (verify!)
(900801, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 0, 108, 0, 0, 0x2, 0, 0, 5, 313, 0, 0, 'Affl: Corruption +50%', 0x003F3F),
-- 900802: DoT ticks → Spread to 2 targets (DUMMY, proc via spell_proc + C++)
(900802, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Affl: DoT Spread', 0x003F3F),
-- 900803: Helper - Shadow AoE (instant Shadow AoE damage, 8yd around target)
-- Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), RadiusIndex=13(8yd), SchoolMask=32 (Shadow)
(900803, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 15, 13, 0, 0, 0, 0, 0, 0, 5, 313, 32, 0, 'Shadow Eruption', 0x003F3F);

-- ============================================================
-- Warlock Demonology: spell_dbc entries (900833-900843)
-- SpellFamilyName=5 (Warlock)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900833, 900834, 900835, 900836, 900837, 900838, 900839, 900840, 900841, 900842, 900843);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900833: Kill extends Meta duration (DUMMY marker, C++ via PlayerScript)
(900833, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Meta Kill Extend', 0x003F3F),
-- 900834: Demon Form periodic AoE+Heal (PERIODIC_DUMMY, 3s tick, permanent)
(900834, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 226, 0, 0, 0, 0, 3000, 5, 313, 0, 0, 'Demo: Meta AoE+Heal', 0x003F3F),
-- 900835: Demons spawn lesser versions (DUMMY marker, C++ via UnitScript)
(900835, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Lesser Spawn', 0x003F3F),
-- 900836: Imp Firebolt +50% damage (DUMMY marker, C++ via UnitScript pet damage boost)
-- NOTE: DBC ADD_PCT_MODIFIER cannot target pet spells (different SpellFamilyName)
(900836, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Imp FB +50%', 0x003F3F),
-- 900837: Imp Firebolt +9 targets (DUMMY marker, C++ on Imp Firebolt -47964)
(900837, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Imp FB AoE', 0x003F3F),
-- 900838: Felguard Cleave unlimited targets (DUMMY marker, C++ on 47994)
(900838, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: FG Unlim', 0x003F3F),
-- 900839: Felguard +50% damage (DUMMY marker, C++ via UnitScript pet damage boost)
-- NOTE: DBC MOD_DAMAGE_PERCENT_DONE on player would boost player damage, not pet
(900839, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: FG +50%', 0x003F3F),
-- 900840: Sacrifice grants ALL bonuses (DUMMY marker, C++ on 18788)
(900840, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Demo: Sacrifice All', 0x003F3F),
-- 900841: Helper - Imp Firebolt bounce (instant Fire single-target damage)
(900841, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 5, 313, 4, 0, 'Imp Firebolt Bounce', 0x003F3F),
-- 900842: Helper - Meta Shadow AoE (instant Shadow AoE, 8yd around caster)
-- RadiusIndex=13 (8yd) for SRC_AREA_ENEMY targeting
(900842, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 600, 22, 13, 0, 0, 0, 0, 0, 0, 5, 313, 32, 0, 'Meta Shadow Burst', 0x003F3F),
-- 900843: Helper - Meta Self Heal (instant heal on caster)
(900843, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 100, 400, 1, 0, 0, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Meta Demon Heal', 0x003F3F);

-- ============================================================
-- Warlock Demonology: creature_template entries (Lesser Demons)
-- ============================================================

-- Lesser Imp (NPC 900835): 30s temp summon, 50% stats
DELETE FROM `creature_template` WHERE `entry` IN (900835, 900836, 900837);
INSERT INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`,
    `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`,
    `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`,
    `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`,
    `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`,
    `dynamicflags`, `family`,
    `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`,
    `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`,
    `AIName`, `MovementType`, `HoverHeight`,
    `HealthModifier`, `ManaModifier`, `ArmorModifier`,
    `ExperienceModifier`, `RacialLeader`, `movementId`,
    `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`,
    `ScriptName`, `VerifiedBuild`) VALUES
-- Lesser Imp: small fire demon, 50% HP, 50% damage
(900835, 0, 0, 0,
 0, 0, 'Lesser Imp', '', '', 0,
 80, 80, 2, 14, 0, 1, 1.14286,
 20, 0.5, 0, 0, 0.5, 1500,
 2000, 1, 1, 1, 0, 2048,
 0, 0,
 3, 0, 0, 0, 0,
 0, 0, 0, 0,
 '', 0, 1,
 0.3, 0.3, 0.3,
 0, 0, 0,
 1, 0, 0, 0,
 '', 12340),
-- Lesser Felguard: melee demon, 50% HP, 50% damage
(900836, 0, 0, 0,
 0, 0, 'Lesser Felguard', '', '', 0,
 80, 80, 2, 14, 0, 1, 1.14286,
 20, 0.7, 0, 0, 0.5, 2000,
 2000, 1, 1, 1, 0, 2048,
 0, 0,
 3, 0, 0, 0, 0,
 0, 0, 0, 0,
 '', 0, 1,
 0.5, 0.3, 0.5,
 0, 0, 0,
 1, 0, 0, 0,
 '', 12340),
-- Lesser Voidwalker: tanky demon, 50% HP, low damage
(900837, 0, 0, 0,
 0, 0, 'Lesser Voidwalker', '', '', 0,
 80, 80, 2, 14, 0, 1, 1.14286,
 20, 0.6, 0, 0, 0.3, 2000,
 2000, 1, 1, 1, 0, 2048,
 0, 0,
 3, 0, 0, 0, 0,
 0, 0, 0, 0,
 '', 0, 1,
 0.6, 0.3, 0.6,
 0, 0, 0,
 1, 0, 0, 0,
 '', 12340);

-- Lesser Demon display models
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (900835, 900836, 900837);
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
-- Lesser Imp: Imp model (4449), small scale
(900835, 0, 4449, 0.6, 1, 12340),
-- Lesser Felguard: Felguard model (18399), smaller scale
(900836, 0, 18399, 0.7, 1, 12340),
-- Lesser Voidwalker: Voidwalker model (1132), smaller scale
(900837, 0, 1132, 0.6, 1, 12340);

-- ============================================================
-- Warlock Destruction: spell_dbc entries (900866-900872)
-- SpellFamilyName=5 (Warlock)
-- Shadow Bolt SpellFamilyFlags[0]=0x1 (verify!)
-- Chaos Bolt SpellFamilyFlags[1]=0x1000000 (verify!)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900866, 900867, 900868, 900869, 900870, 900871, 900872);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900866: Shadow Bolt +9 targets (DUMMY marker, C++ on -47809)
(900866, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Destro: SB AoE', 0x003F3F),
-- 900867: Shadow Bolt +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskA=0x1 targets Shadow Bolt (verify!)
(900867, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0x1, 0, 0, 5, 313, 0, 0, 'Destro: SB +50%', 0x003F3F),
-- 900868: Chaos Bolt +50% damage (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE)
-- EffectSpellClassMaskB=0x1000000 targets Chaos Bolt (verify!)
(900868, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 108, 0, 0, 0, 0x1000000, 0, 5, 313, 0, 0, 'Destro: CB +50%', 0x003F3F),
-- 900869: Chaos Bolt CD -2s (ADD_FLAT_MODIFIER + SPELLMOD_COOLDOWN)
-- EffectSpellClassMaskB=0x1000000 targets Chaos Bolt (verify!)
(900869, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, -2000, 1, 107, 11, 0, 0, 0x1000000, 0, 5, 313, 0, 0, 'Destro: CB CD -2s', 0x003F3F),
-- 900870: Chaos Bolt +9 targets (DUMMY marker, C++ on -59172)
(900870, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 5, 313, 0, 0, 'Destro: CB AoE', 0x003F3F),
-- 900871: Helper - Shadow Bolt bounce (instant Shadow single-target damage)
-- BasePoints via CastCustomSpell. SchoolMask=32 (Shadow)
(900871, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 5, 313, 32, 0, 'Shadow Bolt Bounce', 0x003F3F),
-- 900872: Helper - Chaos Bolt bounce (instant Fire single-target damage, ignores resists)
-- Attributes=0x10100000 (NOT_SHAPESHIFT + UNAFFECTED_BY_INVULNERABILITY)
-- SchoolMask=4 (Fire, same as Chaos Bolt)
(900872, 0x10100000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 5, 313, 4, 0, 'Chaos Bolt Bounce', 0x003F3F);

-- ============================================================
-- Priest Discipline: spell_dbc entries (900900-900903)
-- SpellFamilyName=6 (Priest)
-- PW:Shield SpellFamilyFlags[0]=0x1 (verify!)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900900, 900901, 900902, 900903);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900900: Shields explode on break/fade (DUMMY marker, C++ on -48066)
(900900, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Disc: Shield Explode', 0x003F3F),
-- 900901: Shields +50% absorb (ADD_PCT_MODIFIER + SPELLMOD_DAMAGE on PW:S)
-- EffectSpellClassMaskA=0x1 targets PW:Shield (verify!)
(900901, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 50, 1, 0, 108, 0, 0, 0x1, 0, 0, 6, 566, 0, 0, 'Disc: Shield +50%', 0x003F3F),
-- 900903: Helper - Shield Explosion AoE (instant Holy AoE, 8yd around target)
-- Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), RadiusIndex=13(8yd), SchoolMask=2 (Holy)
-- BasePoints via CastCustomSpell (scales with shield amount)
(900903, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 15, 13, 0, 0, 0, 0, 0, 0, 6, 566, 2, 0, 'Shield Explosion', 0x003F3F);

-- 900902: Weakened Soul duration override → 5 seconds
-- This overrides the existing Weakened Soul debuff (6788) duration to 5s
DELETE FROM `spell_dbc` WHERE `ID` = 6788;
INSERT INTO `spell_dbc` (`ID`, `DurationIndex`) VALUES
(6788, 1);
-- NOTE: DurationIndex=1 = 6s (closest standard duration to 5s).
-- For exact 5s, a custom SpellDuration.dbc entry or C++ override may be needed.
-- Alternative approach: use a field-by-field override that only sets DurationIndex.

-- ============================================================
-- Priest Holy: spell_dbc entries (900933)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` = 900933;
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900933: Direct heals → Holy Fire on enemies (DUMMY, proc via spell_proc + C++)
(900933, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Holy: Heal Fire', 0x003F3F);

-- ============================================================
-- Priest Shadow: spell_dbc entries (900966-900968)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (900966, 900967, 900968);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 900966: DoT ticks → Shadow AoE (DUMMY, proc via spell_proc + C++)
(900966, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Shadow: DoT AoE', 0x003F3F),
-- 900967: DoT ticks → Spread to 2 targets (DUMMY, proc via spell_proc + C++)
(900967, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 6, 566, 0, 0, 'Shadow: DoT Spread', 0x003F3F),
-- 900968: Helper - Shadow AoE (instant Shadow AoE damage, 8yd around target)
-- Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), RadiusIndex=13(8yd), SchoolMask=32 (Shadow)
(900968, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 200, 800, 15, 13, 0, 0, 0, 0, 0, 0, 6, 566, 32, 0, 'Shadow Eruption', 0x003F3F);

-- ============================================================
-- Priest: spell_proc entries
-- ============================================================
DELETE FROM `spell_proc` WHERE `SpellId` IN (900933, 900966, 900967);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
-- 900933: Direct heal → Holy Fire. ProcFlags=0x10000 (DONE_SPELL_MAGIC_DMG_CLASS_POS), 20%, 3s ICD.
(900933, 0, 6, 0, 0, 0, 0x10000, 2, 2, 0, 0, 0, 0, 20, 3000, 0),
-- 900966: DoT tick → Shadow AoE. ProcFlags=0x40000 (DONE_PERIODIC), 20%, 2s ICD.
(900966, 0, 6, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 20, 2000, 0),
-- 900967: DoT tick → Spread. ProcFlags=0x40000 (DONE_PERIODIC), 15%, 3s ICD.
(900967, 0, 6, 0, 0, 0, 0x40000, 1, 2, 0, 0, 0, 0, 15, 3000, 0);

-- ============================================================
-- Non-Class Global: spell_dbc entries (901100-901107)
-- SpellFamilyName=0 (Generic)
-- ============================================================
DELETE FROM `spell_dbc` WHERE `ID` IN (901100, 901101, 901102, 901103, 901104, 901105, 901106, 901107, 901108);
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectTriggerSpell_1`, `EffectSpellClassMaskA_1`, `EffectSpellClassMaskB_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `CumulativeAura`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
-- 901100: Cast while moving (DUMMY marker — SPELL_AURA_CAST_WHILE_WALKING=330 does NOT
-- exist in WotLK 3.3.5a. Requires AzerothCore core backport or per-spell ATTR5 flags.)
(901100, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Cast Moving', 0x003F3F),
-- 901101: Kill → heal 5% HP (DUMMY, proc via spell_proc + C++)
(901101, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Kill Heal', 0x003F3F),
-- 901102: Extra Attack 25% (DUMMY, proc via spell_proc + C++)
-- C++ grants extra attack via helper 901108 (SPELL_EFFECT_ADD_EXTRA_ATTACKS)
(901102, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Extra Attack', 0x003F3F),
-- 901103: 10% cleave to all enemies (DUMMY, proc via spell_proc + C++)
(901103, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Cleave Proc', 0x003F3F),
-- 901104: Avoid → counter attack (DUMMY, proc via spell_proc + C++)
(901104, 0x10000040, 0, 0, 0x10000000, 1, 21, 1, 6, 0, 0, 1, 4, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Global: Counter', 0x003F3F),
-- 901105: Helper - Kill Heal (instant heal on caster, BasePoints via CastCustomSpell)
(901105, 0x10000000, 0, 0, 0, 1, 0, 1, 10, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 136, 0, 0, 'Kill Heal', 0x003F3F),
-- 901106: Helper - Cleave Damage (instant single-target damage, BasePoints via CastCustomSpell)
-- SchoolMask=1 (Physical) — damage type mirrors the original hit via CastCustomSpell
(901106, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Cleave Hit', 0x003F3F),
-- 901107: Helper - Counter Attack (instant Physical single-target, BasePoints via CastCustomSpell)
(901107, 0x10000000, 0, 0, 0, 1, 0, 1, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Counter Strike', 0x003F3F),
-- 901108: Helper - Extra Attack (SPELL_EFFECT_ADD_EXTRA_ATTACKS=32, grants 1 extra melee swing)
-- Target=SELF(1), BasePoints=1
(901108, 0x10000000, 0, 0, 0, 1, 0, 1, 32, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 136, 1, 0, 'Extra Attack Hit', 0x003F3F);

-- ============================================================
-- Non-Class Global: spell_proc entries
-- ============================================================
DELETE FROM `spell_proc` WHERE `SpellId` IN (901101, 901102, 901103, 901104);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
-- 901101: Kill → heal. ProcFlags=0x1 (KILL), 100% chance, no ICD (kill-rate-limited).
(901101, 0, 0, 0, 0, 0, 0x1, 0, 0, 0, 0, 0, 0, 100, 0, 0),
-- 901102: Extra Attack. ProcFlags=0x14 (melee auto + melee spell), 25%, 1s ICD (prevent recursion).
(901102, 0, 0, 0, 0, 0, 0x14, 0, 2, 0, 0, 0, 0, 25, 1000, 0),
-- 901103: Cleave proc. ProcFlags=0x10014 (melee auto + melee spell + spell magic neg), 10%, 1s ICD.
(901103, 0, 0, 0, 0, 0, 0x10014, 1, 2, 0, 0, 0, 0, 10, 1000, 0),
-- 901104: Counter attack. ProcFlags=0x2 (taken melee auto), 100% (C++ filters by hit mask), 2s ICD.
(901104, 0, 0, 0, 0, 0, 0x2, 0, 0, 0, 0, 0, 0, 100, 2000, 0);
