-- ============================================================
-- mod_custom_spells_g_aoe_hooks.sql
--
-- Reactivates C++ AfterHit hooks for "+9 targets" mechanics on
-- 14 single-target spells where the original DBC-only approach
-- (SPELLMOD_JUMP_TARGETS = 17) does NOT work.
--
-- Reason: SPELLMOD_JUMP_TARGETS only modifies ChainTarget on
-- spells that already have a chain/bounce mechanic in DBC.
-- Single-target spells (CS, HS, SS, Hemo, MF, FB, Pyro,
-- Frostbolt, IL, AB, ABarr, SB, CB) are unaffected by it.
--
-- The corresponding C++ classes (spell_custom_*_aoe) are already
-- registered via RegisterSpellScript() in their per-class .cpp
-- files. They check HasAura(<marker_aura>) before firing, so they
-- only activate for players who hold the relevant Paragon passive.
--
-- The marker auras (e.g. 900273 for CS +9 targets) remain in
-- spell_dbc as DUMMY/PCT_MODIFIER auras for visual reference and
-- the optional DBC modifier; the actual AoE effect is now driven
-- by C++.
-- ============================================================

-- Idempotent: clean up any existing entries for these script names first
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
    'spell_custom_ret_cs_aoe',
    'spell_custom_dkb_hs_aoe',
    'spell_custom_ele_cl_aoe',
    'spell_custom_bal_mf_aoe',
    'spell_custom_rog_ss_aoe',
    'spell_custom_rog_hemo_aoe',
    'spell_custom_mage_abarr_aoe',
    'spell_custom_mage_ab_aoe',
    'spell_custom_mage_fb_aoe',
    'spell_custom_mage_pyro_aoe',
    'spell_custom_mage_frostbolt_aoe',
    'spell_custom_mage_icelance_aoe',
    'spell_custom_wlk_sb_aoe',
    'spell_custom_wlk_cb_aoe'
);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
-- Paladin Ret: Crusader Strike +9 targets (all ranks via -35395)
-- Marker aura: 900273
(-35395, 'spell_custom_ret_cs_aoe'),
-- DK Blood: Heart Strike +9 targets (all ranks via -55050)
-- Marker aura: 900303
(-55050, 'spell_custom_dkb_hs_aoe'),
-- Shaman Ele: Chain Lightning +6 targets, full damage (all ranks via -49271)
-- Marker aura: 900400 (CL has native chain mechanic, but DBC version reduces damage per hop;
-- C++ version gives FULL damage on all targets)
(-49271, 'spell_custom_ele_cl_aoe'),
-- Druid Balance: Moonfire +9 targets (all ranks via -8921)
-- Marker aura: 901000
(-8921, 'spell_custom_bal_mf_aoe'),
-- Rogue Combat: Sinister Strike +9 targets (all ranks via -1752)
-- Marker aura: 900634
(-1752, 'spell_custom_rog_ss_aoe'),
-- Rogue Sub: Hemorrhage +9 targets (all ranks via -16511)
-- Marker aura: 900668
(-16511, 'spell_custom_rog_hemo_aoe'),
-- Mage Arcane: Arcane Barrage +9 targets (all ranks via -44425)
-- Marker aura: 900702
(-44425, 'spell_custom_mage_abarr_aoe'),
-- Mage Arcane: Arcane Blast +9 targets (all ranks via -30451)
-- Marker aura: 900704 (also keeps existing spell_custom_mage_ab_charges hook)
(-30451, 'spell_custom_mage_ab_aoe'),
-- Mage Fire: Fireball +9 targets (all ranks via -133)
-- Marker aura: 900734
(-133, 'spell_custom_mage_fb_aoe'),
-- Mage Fire: Pyroblast +9 targets (all ranks via -11366)
-- Marker aura: 900735 (also keeps existing spell_custom_mage_pyro_hotstreak hook)
(-11366, 'spell_custom_mage_pyro_aoe'),
-- Mage Frost: Frostbolt +9 targets (all ranks via -116)
-- Marker aura: 900767
(-116, 'spell_custom_mage_frostbolt_aoe'),
-- Mage Frost: Ice Lance +9 targets (all ranks via -30455)
-- Marker aura: 900769
(-30455, 'spell_custom_mage_icelance_aoe'),
-- Warlock Destruction: Shadow Bolt +9 targets (all ranks via -686)
-- Marker aura: 900866
(-686, 'spell_custom_wlk_sb_aoe'),
-- Warlock Destruction: Chaos Bolt +9 targets (all ranks via -50796)
-- Marker aura: 900870
(-50796, 'spell_custom_wlk_cb_aoe');
