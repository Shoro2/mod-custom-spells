-- Third concept list (operator, 2026-10-31), wave 3, agent F: the large items of
-- the Warlock, Mage, Paladin and Hunter lines. Plan: share-public
-- custom-spells/records/2026-10-31-third-concept-list.md. Ids: Paladin Ret
-- 900266 / 900277, Hunter BM 900509 / 900510, Hunter Survival 900576, Mage
-- Arcane 900723 / 900724, Warlock Affliction 900811, Destruction 900883 /
-- 900884. The C++ is in custom_spells_{paladin,hunter,mage,warlock}.cpp, the
-- spell_script_names bindings are added by the integrator (agent F's report).
-- Written offline (no build, no database); the integrator builds and tests.
--
-- Conventions as in the earlier files: DieSides 0, so a value = its BasePoints;
-- DurationIndex 21 = infinite, 1 = 10 s, 8 = 15 s, 35 = 4 s; RangeIndex 1 = self,
-- 4 = 30 yd, 5 = 40 yd; EquippedItemClass -1 on every row. Radius index 14 = 8 yd.
-- Mask columns: EffectSpellClassMaskA_n = effect 1 word n-1. DefenseType is the
-- spell's DmgClass (0 none, 1 magic). Passive markers carry Attributes 0x10000040
-- and AttributesEx3 0x10000000 like the other script markers.
--
-- ACTIVES (player-castable, learned in the /spells picker, NOT cursed-item pool
-- passives, no 0x40 passive flag): 900266 Divine Barrage, 900509 Switch Places,
-- 900576 Trap Grab. Their client spell records (cast, cooldown, icon, tooltip)
-- come with wave 4 (tools/client_spell_sync.py); the three ids go into the
-- EXCLUDED table of mod-paragon-itemgen tools/gen_class_passives.py like 900741.
DELETE FROM `spell_dbc` WHERE `ID` IN (900266, 900277, 900509, 900510, 900576, 900723, 900724, 900811, 900883, 900884);

-- ----------------------------------------------------------------- Paladin
-- 900266 Retribution "Divine Barrage" (ACTIVE): "cast a divine storm every 0.5
-- sec for 4 seconds dealing bonus damage scaling with your paragon lvl. 60sec cd,
-- channels (like bladestorm just ds instead of ww)". A self aura, not a channel
-- (like Bladestorm): effect 1 aura 226 periodic dummy, period 500 ms, duration
-- 4 s (index 35), instant, 60 s cooldown (RecoveryTime), global cooldown 1.5 s
-- (StartRecoveryCategory 133), no cost. The C++ (spell_custom_ret_divine_barrage)
-- casts a triggered Divine Storm and the burst 900277 on every tick.
-- 900277 the bonus burst: Holy damage 666 (the script sets 666 + 5 x the Paragon
-- level) to every enemy within 8 yd of the paladin (target pair 22 / 15 as
-- Divine Storm's own weapon-damage effect); magic, cannot miss
-- (AttributesEx3 SPELL_ATTR3_ALWAYS_HIT 0x40000).
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `RecoveryTime`, `StartRecoveryCategory`, `StartRecoveryTime`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `ImplicitTargetB_1`, `EffectRadiusIndex_1`, `EffectAura_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `DefenseType`, `PreventionType`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900266, 0, 0, 1, 35, 1, -1, 60000, 133, 1500, 6, 0, 0, 1, 0, 0, 226, 500, 10, 3027, 2, 0, 0, 'Divine Barrage', 0x003F3F),
(900277, 0, 0x40000, 1, 0, 1, -1, 0, 0, 0, 2, 0, 666, 22, 15, 14, 0, 0, 10, 3027, 2, 1, 1, 'Barrage Strike', 0x003F3F);

-- ------------------------------------------------------------------ Hunter
-- 900509 Beast Mastery "Switch Places" (ACTIVE): "switch position with our pet
-- and enraging both of you for 10 seconds for +30% damage. 60 seconds cooldown."
-- Instant self cast with a dummy effect (the swap is C++, CheckCast needs the
-- pet alive within 40 yd in line of sight), 60 s cooldown, global cooldown 1.5 s.
-- 900510 the enrage on the hunter and the pet: aura 79 (+30 % damage done, all
-- schools = misc 127), 10 s (index 1).
-- 900576 Survival "Trap Grab" (ACTIVE): "trap grab: pulls together all traps within
-- 40y range and throws them at the target". Instant, an enemy within 40 yd (range
-- index 5), a dummy effect (the C++ takes the hunter's trap game objects within
-- 40 yd and sets their effects off at the target; CheckCast refuses without a
-- trap), 30 s cooldown (my choice: the traps' own), 5 % of the base mana,
-- global cooldown 1.5 s. DmgClass 0: no hit roll.
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `RecoveryTime`, `StartRecoveryCategory`, `StartRecoveryTime`, `ManaCostPct`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900509, 0, 1, 0, 1, -1, 60000, 133, 1500, 0, 3, 0, 0, 1, 0, 0, 9, 1680, 1, 'Switch Places', 0x003F3F),
(900510, 0, 1, 1, 1, -1, 0, 0, 0, 0, 6, 0, 30, 1, 79, 127, 9, 1680, 1, 'Switched Fury', 0x003F3F),
(900576, 0, 1, 0, 5, -1, 30000, 133, 1500, 5, 3, 0, 0, 6, 0, 0, 9, 37, 1, 'Trap Grab', 0x003F3F);

-- -------------------------------------------------------------------- Mage
-- 900723 Arcane "Blink: Through the Enemy": "when you blink through an enemy, blink
-- cd is reset but cost 5% more mana each cast (10sec duration, 10 stacks). cannot
-- be used if u have targeted blink from above" - marker read by
-- spell_custom_mage_blink_through (on Blink 1953). 900724 Blink Strain: aura 108
-- SPELLMOD_COST (14) +5 % per stack on Blink (family 3, flags0 0x10000), 10 stacks
-- (CumulativeAura), 10 s (index 1); every new stack refreshes the time.
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `EffectSpellClassMaskA_1`, `CumulativeAura`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900723, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 0, 0, 3, 1499, 0, 'Arcane: Blink Through the Enemy', 0x003F3F),
(900724, 0, 0, 1, 1, 1, -1, 6, 0, 5, 1, 108, 14, 0x10000, 10, 3, 1499, 0, 'Blink Strain', 0x003F3F);

-- ----------------------------------------------------------------- Warlock
-- 900811 Affliction "Drain Life: +9 Targets": "drain life +9 targets" - marker read
-- by spell_custom_wlk_drain_plus9 (an aura script on Drain Life 689, every rank).
-- 900883 Destruction "Shadow Bolt: Shadow Immolate": "targets affected by immolate
-- receive 15% more damage of your shadow bolt. when hit by shadow bolt the dot
-- transforms in shadow immolate (double damage, shadowfire damage)" - marker read
-- by spell_custom_wlk_shadow_bolt_immolate (Shadow Bolt 686, every rank).
-- 900884 Shadow Immolate: a copy of Immolate's DoT (rank 11, 47811 effect 1):
-- aura 3 periodic damage every 3 s, 15 s (index 8), base 314 = twice rank 11's
-- 157 a tick, school mask 0x24 = Shadow 0x20 + Fire 0x04, magic dispel type,
-- family 5 flags0 0x4 - the family flag is what gives the aura state
-- AURA_STATE_CONFLAGRATE (SpellInfo::LoadAuraState) and what Conflagrate,
-- Incinerate, Fire and Brimstone and the Immolate spell modifiers search for.
-- Attributes 0x10000 / Ex3 0x80 / Ex4 0x100000 as Immolate's. The C++ replaces
-- Immolate with it and overwrites the amount, duration and tick timer with the
-- doubled / remaining values of the old aura; the row's own numbers matter for a
-- fresh application only (a DoT-spread copy, 900802).
INSERT INTO `spell_dbc` (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`, `SpellClassSet`, `SpellIconID`, `SchoolMask`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900811, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 5, 546, 0, 'Affl: Drain Life +9 Targets', 0x003F3F),
(900883, 0x10000040, 0x10000000, 1, 21, 1, -1, 6, 0, 0, 1, 4, 0, 5, 213, 0, 'Destro: Shadow Immolate', 0x003F3F);
INSERT INTO `spell_dbc` (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `AttributesEx4`, `CastingTimeIndex`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`, `EffectAura_1`, `EffectAuraPeriod_1`, `SpellClassSet`, `SpellClassMask_1`, `SpellVisualID_1`, `SpellIconID`, `SchoolMask`, `DefenseType`, `PreventionType`, `Name_Lang_enUS`, `Name_Lang_Mask`) VALUES
(900884, 1, 0x10000, 0x80, 0x100000, 1, 8, 4, -1, 6, 0, 314, 6, 3, 3000, 5, 0x4, 46, 31, 36, 1, 1, 'Shadow Immolate', 0x003F3F);
