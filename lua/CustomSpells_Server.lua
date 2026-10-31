-- =============================================================
-- mod-custom-spells — Server-side AIO (Eluna)
--
-- Spell picker: players learn/forget the custom class spells
-- (900xxx markers/actives) via toggles in the client UI.
-- The whitelist below is the single source of truth for what a
-- class may learn; every Toggle request is validated against it.
-- Helper spells, orphaned markers and legacy rows are excluded
-- (2026-10-08: 900106 is Critical Execution's damage strike, a
-- helper - learnable it was a free 666 + 66 % AP attack; 900840
-- Sacrifice All hooks Demonic Sacrifice, which no 3.3.5 warlock
-- can learn).
--
-- Regeneration: the table mirrors acore_world.spell_dbc passives
-- (plus the player-castable actives and the warrior real-DBC
-- block 900100-900121); see share-public claude_log 2026-07-18.
-- The labels are the spellbook names of data/spellbook_enUS.json:
-- tools/client_spell_sync.py --lua rewrites them, so edit the
-- manifest, not the strings here.
--
-- GMs also get "Learn all Talents": mod-forgotten-talents'
-- .forgotten learnall, run for the clicking GM (LearnAllTalents).
-- =============================================================

local AIO = AIO or require("AIO")

if not CustomSpells_ServerHandlers then
	CustomSpells_ServerHandlers = {}
end

-- ============================================================
-- Icon badges: spell id -> the short text the client draws on the
-- spell's icon (data/spellbook_enUS.json "badge"; the base icon is
-- the stock spell the passive modifies, so only icon + badge is
-- unique). The block is generated - edit the manifest and run
-- tools/client_spell_sync.py --lua; tools/check_badges.py checks it.
-- ============================================================

-- BEGIN GENERATED SPELL_BADGES (tools/client_spell_sync.py --lua; edit data/spellbook_enUS.json, not this block)
local SPELL_BADGES = {
	[900100] = "+50%", -- Mortal Strike: +50% Damage
	[900101] = "-2s", -- Mortal Strike: -2 sec Cooldown
	[900102] = "+50%", -- Overpower: +50% Damage
	[900103] = "+9", -- Mortal Strike: +9 Targets
	[900104] = "+9", -- Overpower: +9 Targets
	[900105] = "PROC", -- Critical Execution
	[900107] = "-0.5s", -- Timed Attacks
	[900108] = "ALL", -- Whirlwind: Unlimited Targets
	[900109] = "+50%", -- Bloodthirst: +50% Damage
	[900110] = "+9", -- Bloodthirst: +9 Targets
	[900111] = "+50%", -- Whirlwind: +50% Damage
	[900112] = "ALL", -- Cleave: Unlimited Targets
	[900114] = "PROC", -- Whirly Attacks
	[900116] = "x5", -- Bloody Whirlwind
	[900117] = "RESET", -- Speedy Bloodthirst
	[900118] = "+OP", -- Whirlwind: Overpower
	[900119] = "+BT", -- Whirlwind: Bloodthirst
	[900133] = "ANY", -- Whirlwind: Any Stance
	[900168] = "+50%", -- Revenge: +50% Damage
	[900169] = "AoE", -- Revenge: Sweeping
	[900170] = "REND", -- Thunder Clap: Rend and Sunder
	[900171] = "+50%", -- Thunder Clap: +50% Damage
	[900172] = "PROC", -- Devastate Lightning
	[900173] = "PROC", -- Block: Enhanced Thunder Clap
	[900200] = "BURST", -- Holy Shock: Burst
	[900201] = "RAYS", -- Holy Shock: Radiance
	[900202] = "BOTH", -- Holy Shock: Both Ways
	[900203] = "+50%S", -- Holy Shock: +50% Effect
	[900204] = "HEAL", -- Consecration: Healing
	[900205] = "MOVE", -- Consecration: Follows You
	[900206] = "+50%", -- Consecration: +50% Damage
	[900207] = "+5s", -- Consecration: +5 sec Duration
	[900213] = "AoE", -- Enlightened Heals
	[900214] = "ERUPT", -- Holy Shock: Light Erupts
	[900234] = "MOVEP", -- Consecration: Follows You
	[900235] = "+9", -- Avenger's Shield: +9 Targets
	[900236] = "+50%", -- Avenger's Shield: +50% Damage
	[900237] = "+99", -- Holy Shield: +99 Charges
	[900238] = "+50%", -- Holy Shield: +50% Damage
	[900239] = "CONS", -- Avenger's Shield: Consecration
	[900240] = "+AS", -- Judgement: Avenger's Shield
	[900241] = "-2s", -- Judgement: -2 sec Cooldown
	[900242] = "WAVE", -- Wave of Light
	[900268] = "MOVER", -- Consecration: Follows You
	[900269] = "-2s", -- Judgement: -2 sec Cooldown
	[900270] = "+6", -- Divine Storm: +6 Targets
	[900271] = "+50%", -- Divine Storm: +50% Damage
	[900272] = "+50%", -- Crusader Strike: +50% Damage
	[900273] = "+9", -- Crusader Strike: +9 Targets
	[900274] = "x10", -- Exorcism Power
	[900266] = "x8", -- Divine Barrage
	[900500] = "AMMO", -- Recovered Arrows
	[900501] = "ALL", -- Multi-Shot: Unlimited Targets
	[900502] = "+50%", -- Pet: +50% Damage
	[900503] = "HASTE", -- Pet: +50% Attack Speed
	[900504] = "AoE", -- Pet: Beast Cleave
	[900507] = "+100%", -- Pet: +100% Health
	[900525] = "x2", -- Second Pet
	[900509] = "SWAP", -- Switch Places
	[900533] = "+9", -- Auto Shot: Ricochet
	[900534] = "x20", -- Barrage
	[900537] = "STING", -- Multi-Shot: Serpent Sting
	[900538] = "+9", -- Chimera Shot: +9 Targets
	[900539] = "+50%", -- Chimera Shot: +50% Damage
	[900540] = "x5", -- Running Aim
	[900566] = "PROC", -- Explosive Traps
	[900568] = "+5%", -- Blade and Shot
	[900571] = "+9R", -- Raptor Strike: +9 Targets
	[900572] = "+50%R", -- Raptor Strike: +50% Damage
	[900573] = "+50%", -- Traps: +50% Damage
	[900574] = "RAND", -- Random Traps
	[900575] = "x2", -- Trap Echo
	[900576] = "GRAB", -- Trap Grab
	[900600] = "+50%", -- Energy: +50% Regeneration
	[900601] = "+50%", -- Mutilate: +50% Damage
	[900602] = "+50%", -- Poisons: +50% Damage
	[900603] = "NOVA", -- Poison Nova
	[900605] = "+9", -- Mutilate: +9 Targets
	[900606] = "JUMP", -- Hunger for Blood: Contagion
	[900633] = "+50%", -- Sinister Strike: +50% Damage
	[900634] = "+9", -- Sinister Strike: +9 Targets
	[900635] = "2min", -- Blade Flurry: 2 min Duration
	[900636] = "+9", -- Blade Flurry: +9 Targets
	[900637] = "+50%", -- Energy: +50% Regeneration
	[900639] = "RUSH", -- Adrenaline Rush: Unleashed
	[900642] = "FAN", -- Killing Spree: Fan of Knives
	[900643] = "BLEED", -- Fan of Knives: Serrated Blades
	[900666] = "+50%", -- Energy: +50% Regeneration
	[900667] = "+50%", -- Hemorrhage: +50% Damage
	[900668] = "+9", -- Hemorrhage: +9 Targets
	[900670] = "+9", -- Ambush: +9 Targets
	[900671] = "+50%", -- Ambush: +50% Damage
	[900672] = "FLOW", -- Shadow Dance: Flow
	[900674] = "ANY", -- Frontal Assault
	[900900] = "BOOM", -- Power Word: Shield: Explosion
	[900901] = "+50%", -- Power Word: Shield: +50% Absorption
	[900902] = "5s", -- Weakened Soul: 5 sec
	[900904] = "HEAL", -- Atonement
	[900905] = "+9", -- Smite: +9 Targets
	[900906] = "+50%", -- Smite: +50% Damage
	[900907] = "+50%", -- Penance: +50% Healing
	[900908] = "SHLD", -- Mana Burn: Burnt Reserve
	[900910] = "x50", -- Inner Fire: Fifty Stacks
	[900912] = "+4", -- Penance: Four More
	[900933] = "PROC", -- Holy Fire Heals
	[900935] = "+9", -- Holy Fire: +9 Targets
	[900936] = "+50%", -- Holy Fire: +50% Damage
	[900937] = "AUTO", -- Lightwell: Auto Renew
	[900938] = "GUARD", -- Spirit of Redemption: Guardian
	[900941] = "RAD", -- Holy Nova: +50% Radius
	[900942] = "x10", -- Holy Nova: Kindled Fire
	[900944] = "+50%", -- Holy Nova: +50% Damage and Healing
	[900945] = "+50%", -- Prayer of Mending: +50% Healing
	[900946] = "+1", -- Prayer of Mending: Second Jump
	[900947] = "WAVE", -- Divine Hymn: Waves of Light
	[900949] = "CAST", -- Spirit of Redemption: Spirit Casting
	[900966] = "ERUPT", -- Shadow Eruption
	[900967] = "SPRD", -- Spreading Shadows
	[900969] = "+50%", -- Shadow Word: Pain: +50% Damage
	[900970] = "+50%", -- Mind Blast: +50% Damage
	[900971] = "+9", -- Mind Blast: +9 Targets
	[900972] = "FEAST", -- Shadowform: Soul Feast
	[900975] = "TENT", -- Shadowform: Lesser Tentacles
	[900977] = "+50%", -- Mind Sear: +50% Damage
	[900978] = "+25%", -- Mind Flay: Quickened Flay
	[900980] = "+3", -- Mind Sear: Three More
	[900981] = "PULL", -- Shadowfiend: Dark Pull
	[900300] = "x2", -- Dancing Rune Weapon: Two Blades
	[900301] = "ECHO", -- Dancing Rune Weapon: Double Cast
	[900302] = "+50%", -- Heart Strike: +50% Damage
	[900303] = "+9", -- Heart Strike: +9 Targets
	[900304] = "PROC", -- Death Coil Strikes
	[900306] = "BOOM", -- Bloodworm Burst
	[900309] = "+50%", -- Death Strike: +50% Damage
	[900310] = "+9", -- Death Strike: +9 Targets
	[900311] = "PROC", -- Plague Retaliation
	[900312] = "+50%", -- Rune Tap: Shadow Power
	[900333] = "WYRM", -- Raise Dead: Frost Wyrm
	[900334] = "+50%", -- Obliterate: +50% Damage
	[900335] = "+9", -- Obliterate: +9 Targets
	[900336] = "+50%", -- Howling Blast: +50% Damage
	[900337] = "DoT", -- Howling Blast: Diseases
	[900338] = "LEECH", -- Lichborne: Leech
	[900340] = "PROC", -- Icy Talons: Frozen Strikes
	[900343] = "+50%", -- Frost Strike: +50% Damage
	[900344] = "+9", -- Frost Strike: +9 Targets
	[900345] = "FROST", -- Frost Wyrm: Frozen Breath
	[900366] = "ERUPT", -- Plague Eruption
	[900369] = "AoE", -- Ghoul Cleave
	[900371] = "PROC", -- Risen Army
	[900373] = "MOVE", -- Death and Decay: Around You
	[900375] = "+50%", -- Scourge Strike: +50% Damage
	[900376] = "+9", -- Scourge Strike: +9 Targets
	[900377] = "+50%", -- Death Coil: +50% Damage
	[900378] = "+9", -- Death Coil: +9 Targets
	[900379] = "x5", -- Summon Gargoyle: Army
	[900380] = "AUTO", -- Corpse Explosion: Corpse Blast
	[900381] = "WORM", -- Corpse Explosion: Corpse Worms
	[900400] = "+6", -- Chain Lightning: +6 Targets
	[900410] = "+50%", -- Lightning Bolt: +50% Damage
	[900411] = "+9", -- Lightning Bolt: +9 Targets
	[900412] = "+50%", -- Chain Lightning: +50% Damage
	[900401] = "SPRT", -- Totem Spirits
	[900402] = "RAG", -- Fire Elemental: Ragnaros
	[900403] = "OVER", -- Lava Overload
	[900404] = "SPRD", -- Lava Burst: Spreading Flame
	[900405] = "RESET", -- Flame Shock: Lava Reset
	[900406] = "x2", -- Lava Burst: Two Charges
	[900407] = "INST", -- Lava Burst: Clearcast Instant
	[900413] = "+50%", -- Lava Burst: +50% Damage
	[900414] = "+9", -- Lava Burst: +9 Targets
	[900415] = "+2%", -- Elemental Resonance
	[900418] = "+CL", -- Lightning Shield: Chain Lightning
	[900433] = "SPRT", -- Totem Spirits
	[900434] = "AoE", -- Maelstrom Fury
	[900435] = "+50%", -- Empowered Summons
	[900436] = "PROC", -- Spirit Wolf Call
	[900437] = "HASTE", -- Spirit Wolves: Haste
	[900438] = "+CL", -- Spirit Wolves: Chain Lightning
	[900441] = "+50%", -- Stormstrike: +50% Damage
	[900442] = "+9", -- Stormstrike: +9 Targets
	[900443] = "+50%", -- Lava Lash: +50% Damage
	[900444] = "+9", -- Lava Lash: +9 Targets
	[900445] = "RUSH", -- Ghost Wolf: Storm Rush
	[900466] = "SPRT", -- Totem Spirits
	[900467] = "MANA", -- Deep Reserves
	[900468] = "+LB", -- Healing Lightning
	[900469] = "SPIKE", -- Earth Shield: Spikes
	[900470] = "+1", -- Earth Shield: Charging Heals
	[900472] = "ALL", -- Earth Shield: Unlimited Targets
	[900473] = "SURGE", -- Healing Stream: Surge
	[900700] = "MANA", -- Deep Reserves
	[900701] = "+50%", -- Arcane Barrage: +50% Damage
	[900702] = "+9", -- Arcane Barrage: +9 Targets
	[900703] = "-50%", -- Arcane Blast: -50% Cast Time
	[900704] = "+9", -- Arcane Blast: +9 Targets
	[900705] = "x8", -- Arcane Blast: 8 Charges
	[900706] = "+AB", -- Arcane Explosion: Charges
	[900707] = "x4", -- Evocation Power
	[900708] = "SAVE", -- Emergency Mana Shield
	[900709] = "TRGT", -- Blink: To Your Target
	[900714] = "x2", -- Arcane Overflow
	[900715] = "PROC", -- Mirror Shield
	[900716] = "AoE", -- Mirror Images: Splash
	[900720] = "IMG", -- Blink: Mirror Image
	[900721] = "+5%", -- Mirror Images: Strength in Numbers
	[900713] = "GRND", -- Targeted Blink
	[900723] = "THRU", -- Blink: Through the Enemy
	[900733] = "+50%", -- Fireball: +50% Damage
	[900734] = "+9", -- Fireball: +9 Targets
	[900735] = "+9", -- Pyroblast: +9 Targets
	[900736] = "+50%", -- Pyroblast: +50% Damage
	[900737] = "CRIT", -- Fire Blast: Swift and Sure
	[900738] = "INST", -- Pyroblast: Hot Streak
	[900741] = "AoE", -- Meteor
	[900744] = "+LB", -- Dragon's Breath: Living Bomb
	[900745] = "+FS", -- Molten Armor: Flamestrike
	[900766] = "+50%", -- Frostbolt: +50% Damage
	[900767] = "+9", -- Frostbolt: +9 Targets
	[900768] = "+50%", -- Ice Lance: +50% Damage
	[900769] = "+9", -- Ice Lance: +9 Targets
	[900770] = "PERM", -- Water Elemental: Permanent
	[900771] = "AoE", -- Comet Shower
	[900775] = "+IL", -- Ice Block: Ice Lance Barrage
	[900776] = "BOOM", -- Ice Block: Detonation
	[900800] = "ERUPT", -- Shadow Eruption
	[900801] = "+50%", -- Corruption: +50% Damage
	[900802] = "SPRD", -- Spreading Affliction
	[900804] = "+1%", -- Withering Harvest
	[900807] = "+50%", -- Curse of Agony: +50% Damage
	[900808] = "+50%", -- Drain Life: +50% Damage
	[900809] = "+10%", -- Life Tap: Dark Tithe
	[900811] = "+9", -- Drain Life: +9 Targets
	[900833] = "+5s", -- Metamorphosis: Feast
	[900834] = "HEAL", -- Metamorphosis: Fel Vigor
	[900835] = "PROC", -- Lesser Demons
	[900836] = "+50%", -- Imp: +50% Damage
	[900837] = "+9", -- Imp: Firebolt +9 Targets
	[900846] = "FAST", -- Imp: Firebolt 50% Faster
	[900848] = "TANK", -- Voidwalker: Void Bulwark
	[900850] = "+9", -- Succubus: Mass Seduction
	[900852] = "AoE", -- Felhunter: Area Spell Lock and Devour Magic
	[900838] = "ALL", -- Felguard: Unlimited Cleave
	[900839] = "+50%", -- Felguard: +50% Damage
	[900858] = "x2", -- Second Demon
	[900853] = "ALL", -- Shadow Cleave: Unlimited Targets
	[900854] = "+50%", -- Shadow Cleave: +50% Damage
	[900855] = "+25%", -- Felhunter: Spell Shackles
	[900857] = "STUN", -- Demon Charge: Shockwave
	[900866] = "+9", -- Shadow Bolt: +9 Targets
	[900867] = "+50%", -- Shadow Bolt: +50% Damage
	[900868] = "+50%", -- Chaos Bolt: +50% Damage
	[900869] = "-2s", -- Chaos Bolt: -2 sec Cooldown
	[900870] = "+9", -- Chaos Bolt: +9 Targets
	[900873] = "FREE", -- Hellfire: Free Movement
	[900875] = "SELF", -- Rain of Fire: Around You
	[900878] = "+50%S", -- Soul Fire: +50% Damage
	[900879] = "+9S", -- Soul Fire: +9 Targets
	[900880] = "+9", -- Immolate: +9 Targets
	[900881] = "x10", -- Incinerate: Stoked Soul Fire
	[900883] = "+15%", -- Shadow Bolt: Shadow Immolate
	[901000] = "+9", -- Moonfire: +9 Targets
	[901001] = "+50%", -- Moonfire: +50% Damage
	[901002] = "AREA", -- Starfall: +50% Area
	[901003] = "+50%", -- Starfall: +50% Damage
	[901004] = "RESET", -- Starfall: Starlight Reset
	[901005] = "x10", -- Starfall: 10 Stacks
	[901006] = "+1%", -- Lunar Frenzy
	[901008] = "x10", -- Wrath: Starfire Surge
	[901010] = "OWLS", -- Force of Nature: Star Owls
	[901033] = "BLEED", -- Swipe (Bear): Bleed
	[901035] = "BLEED", -- Maul: Bleeding Wounds
	[901036] = "+50%", -- Swipe (Bear): +50% Damage
	[901037] = "OPEN", -- Thorns: Open Wounds
	[901039] = "+1%", -- Ursine Bulwark
	[901049] = "BLDC", -- Swipe (Cat): Bleed
	[901051] = "+50%", -- Energy: +50% Regeneration
	[901052] = "+50%C", -- Swipe (Cat): +50% Damage
	[901053] = "ANY", -- Frontal Assault
	[901054] = "RUSH", -- Berserk: Unleashed
	[901042] = "+50%", -- Maul: +50% Damage
	[901043] = "+9", -- Maul: +9 Targets
	[901057] = "+50%", -- Claw: +50% Damage
	[901058] = "+9", -- Claw: +9 Targets
	[901059] = "+50%", -- Shred: +50% Damage
	[901060] = "+9", -- Shred: +9 Targets
	[901066] = "PROC", -- Treant Call
	[901067] = "HP+", -- Summons: Healing Power
	[901068] = "BLOOM", -- Summons: Parting Bloom
	[901069] = "REJUV", -- Thorns: Rejuvenation
	[901070] = "+50%", -- Heal over Time: +50%
	[901071] = "x2", -- Heal over Time: Double Speed
	[901072] = "MANA", -- Deep Reserves
	[901101] = "HEAL", -- Killing Blow: Recovery
	[901102] = "+1", -- Extra Attack
	[901103] = "PROC", -- Cleaving Strikes
	[901104] = "+50%", -- Riposte
	[901111] = "+5%", -- Empowered
}
-- END GENERATED SPELL_BADGES

-- The table reaches the client inside AIO's initial message, as one
-- more block right after the addon code ran: "CustomSpellsBadges" /
-- "Badges" (lua/CustomSpells_Badge_Client.lua keeps it as
-- FL_SpellBadges for the /spells picker, the spellbook and the Cursed
-- Altar). A script reload makes AIO force-reload every client, so a
-- changed badge arrives with the next init. AddOnInit exists on the
-- main Lua state only.
if AIO.AddOnInit then
	AIO.AddOnInit(function(msg)
		return msg:Add("CustomSpellsBadges", "Badges", SPELL_BADGES)
	end)
end

-- ============================================================
-- Spell whitelist per class ({ id, name, spec } in UI order)
-- ============================================================

local CLASS_SPELLS = {
	[1] = { -- Warrior
		{ 900100, "Mortal Strike: +50% Damage", "Arms" },
		{ 900101, "Mortal Strike: -2 sec Cooldown", "Arms" },
		{ 900102, "Overpower: +50% Damage", "Arms" },
		{ 900103, "Mortal Strike: +9 Targets", "Arms" },
		{ 900104, "Overpower: +9 Targets", "Arms" },
		{ 900105, "Critical Execution", "Arms" },
		{ 900107, "Timed Attacks", "Arms" },
		{ 900108, "Whirlwind: Unlimited Targets", "Fury" },
		{ 900109, "Bloodthirst: +50% Damage", "Fury" },
		{ 900110, "Bloodthirst: +9 Targets", "Fury" },
		{ 900111, "Whirlwind: +50% Damage", "Fury" },
		{ 900112, "Cleave: Unlimited Targets", "Fury" },
		{ 900114, "Whirly Attacks", "Fury" },
		{ 900116, "Bloody Whirlwind", "Fury" },
		{ 900117, "Speedy Bloodthirst", "Fury" },
		{ 900118, "Whirlwind: Overpower", "Arms" },
		{ 900119, "Whirlwind: Bloodthirst", "Fury" },
		{ 900133, "Whirlwind: Any Stance", "Fury" },
		{ 900168, "Revenge: +50% Damage", "Protection" },
		{ 900169, "Revenge: Sweeping", "Protection" },
		{ 900170, "Thunder Clap: Rend and Sunder", "Protection" },
		{ 900171, "Thunder Clap: +50% Damage", "Protection" },
		{ 900172, "Devastate Lightning", "Protection" },
		{ 900173, "Block: Enhanced Thunder Clap", "Protection" },
	},
	[2] = { -- Paladin
		{ 900200, "Holy Shock: Burst", "Holy" },
		{ 900201, "Holy Shock: Radiance", "Holy" },
		{ 900202, "Holy Shock: Both Ways", "Holy" },
		{ 900203, "Holy Shock: +50% Effect", "Holy" },
		{ 900204, "Consecration: Healing", "Holy" },
		{ 900205, "Consecration: Follows You", "Holy" },
		{ 900206, "Consecration: +50% Damage", "Holy" },
		{ 900207, "Consecration: +5 sec Duration", "Holy" },
		{ 900213, "Enlightened Heals", "Holy" },
		{ 900214, "Holy Shock: Light Erupts", "Holy" },
		{ 900234, "Consecration: Follows You", "Protection" },
		{ 900235, "Avenger's Shield: +9 Targets", "Protection" },
		{ 900236, "Avenger's Shield: +50% Damage", "Protection" },
		{ 900237, "Holy Shield: +99 Charges", "Protection" },
		{ 900238, "Holy Shield: +50% Damage", "Protection" },
		{ 900239, "Avenger's Shield: Consecration", "Protection" },
		{ 900240, "Judgement: Avenger's Shield", "Protection" },
		{ 900241, "Judgement: -2 sec Cooldown", "Protection" },
		{ 900242, "Wave of Light", "Protection" },
		{ 900268, "Consecration: Follows You", "Retribution" },
		{ 900269, "Judgement: -2 sec Cooldown", "Retribution" },
		{ 900270, "Divine Storm: +6 Targets", "Retribution" },
		{ 900271, "Divine Storm: +50% Damage", "Retribution" },
		{ 900272, "Crusader Strike: +50% Damage", "Retribution" },
		{ 900273, "Crusader Strike: +9 Targets", "Retribution" },
		{ 900274, "Exorcism Power", "Retribution" },
		{ 900266, "Divine Barrage", "Retribution" },
	},
	[3] = { -- Hunter
		{ 900500, "Recovered Arrows", "Shared" },
		{ 900501, "Multi-Shot: Unlimited Targets", "Shared" },
		{ 900502, "Pet: +50% Damage", "Beast Mastery" },
		{ 900503, "Pet: +50% Attack Speed", "Beast Mastery" },
		{ 900504, "Pet: Beast Cleave", "Beast Mastery" },
		{ 900507, "Pet: +100% Health", "Beast Mastery" },
		{ 900525, "Second Pet", "Beast Mastery" },
		{ 900509, "Switch Places", "Beast Mastery" },
		{ 900533, "Auto Shot: Ricochet", "Marksmanship" },
		{ 900534, "Barrage", "Marksmanship" },
		{ 900537, "Multi-Shot: Serpent Sting", "Marksmanship" },
		{ 900538, "Chimera Shot: +9 Targets", "Marksmanship" },
		{ 900539, "Chimera Shot: +50% Damage", "Marksmanship" },
		{ 900540, "Running Aim", "Marksmanship" },
		{ 900566, "Explosive Traps", "Survival" },
		{ 900568, "Blade and Shot", "Survival" },
		{ 900571, "Raptor Strike: +9 Targets", "Survival" },
		{ 900572, "Raptor Strike: +50% Damage", "Survival" },
		{ 900573, "Traps: +50% Damage", "Survival" },
		{ 900574, "Random Traps", "Survival" },
		{ 900575, "Trap Echo", "Survival" },
		{ 900576, "Trap Grab", "Survival" },
	},
	[4] = { -- Rogue
		{ 900600, "Energy: +50% Regeneration", "Assassination" },
		{ 900601, "Mutilate: +50% Damage", "Assassination" },
		{ 900602, "Poisons: +50% Damage", "Assassination" },
		{ 900603, "Poison Nova", "Assassination" },
		{ 900605, "Mutilate: +9 Targets", "Assassination" },
		{ 900606, "Hunger for Blood: Contagion", "Assassination" },
		{ 900633, "Sinister Strike: +50% Damage", "Combat" },
		{ 900634, "Sinister Strike: +9 Targets", "Combat" },
		{ 900635, "Blade Flurry: 2 min Duration", "Combat" },
		{ 900636, "Blade Flurry: +9 Targets", "Combat" },
		{ 900637, "Energy: +50% Regeneration", "Combat" },
		{ 900639, "Adrenaline Rush: Unleashed", "Combat" },
		{ 900642, "Killing Spree: Fan of Knives", "Combat" },
		{ 900643, "Fan of Knives: Serrated Blades", "Combat" },
		{ 900666, "Energy: +50% Regeneration", "Subtlety" },
		{ 900667, "Hemorrhage: +50% Damage", "Subtlety" },
		{ 900668, "Hemorrhage: +9 Targets", "Subtlety" },
		{ 900670, "Ambush: +9 Targets", "Subtlety" },
		{ 900671, "Ambush: +50% Damage", "Subtlety" },
		{ 900672, "Shadow Dance: Flow", "Subtlety" },
		{ 900674, "Frontal Assault", "Subtlety" },
	},
	[5] = { -- Priest
		{ 900900, "Power Word: Shield: Explosion", "Discipline" },
		{ 900901, "Power Word: Shield: +50% Absorption", "Discipline" },
		{ 900902, "Weakened Soul: 5 sec", "Discipline" },
		{ 900904, "Atonement", "Discipline" },
		{ 900905, "Smite: +9 Targets", "Discipline" },
		{ 900906, "Smite: +50% Damage", "Discipline" },
		{ 900907, "Penance: +50% Healing", "Discipline" },
		{ 900908, "Mana Burn: Burnt Reserve", "Discipline" },
		{ 900910, "Inner Fire: Fifty Stacks", "Discipline" },
		{ 900912, "Penance: Four More", "Discipline" },
		{ 900933, "Holy Fire Heals", "Holy" },
		{ 900935, "Holy Fire: +9 Targets", "Holy" },
		{ 900936, "Holy Fire: +50% Damage", "Holy" },
		{ 900937, "Lightwell: Auto Renew", "Holy" },
		{ 900938, "Spirit of Redemption: Guardian", "Holy" },
		{ 900941, "Holy Nova: +50% Radius", "Holy" },
		{ 900942, "Holy Nova: Kindled Fire", "Holy" },
		{ 900944, "Holy Nova: +50% Damage and Healing", "Holy" },
		{ 900945, "Prayer of Mending: +50% Healing", "Holy" },
		{ 900946, "Prayer of Mending: Second Jump", "Holy" },
		{ 900947, "Divine Hymn: Waves of Light", "Holy" },
		{ 900949, "Spirit of Redemption: Spirit Casting", "Holy" },
		{ 900966, "Shadow Eruption", "Shadow" },
		{ 900967, "Spreading Shadows", "Shadow" },
		{ 900969, "Shadow Word: Pain: +50% Damage", "Shadow" },
		{ 900970, "Mind Blast: +50% Damage", "Shadow" },
		{ 900971, "Mind Blast: +9 Targets", "Shadow" },
		{ 900972, "Shadowform: Soul Feast", "Shadow" },
		{ 900975, "Shadowform: Lesser Tentacles", "Shadow" },
		{ 900977, "Mind Sear: +50% Damage", "Shadow" },
		{ 900978, "Mind Flay: Quickened Flay", "Shadow" },
		{ 900980, "Mind Sear: Three More", "Shadow" },
		{ 900981, "Shadowfiend: Dark Pull", "Shadow" },
	},
	[6] = { -- Death Knight
		{ 900300, "Dancing Rune Weapon: Two Blades", "Blood" },
		{ 900301, "Dancing Rune Weapon: Double Cast", "Blood" },
		{ 900302, "Heart Strike: +50% Damage", "Blood" },
		{ 900303, "Heart Strike: +9 Targets", "Blood" },
		{ 900304, "Death Coil Strikes", "Blood" },
		{ 900306, "Bloodworm Burst", "Blood" },
		{ 900309, "Death Strike: +50% Damage", "Blood" },
		{ 900310, "Death Strike: +9 Targets", "Blood" },
		{ 900311, "Plague Retaliation", "Blood" },
		{ 900312, "Rune Tap: Shadow Power", "Blood" },
		{ 900333, "Raise Dead: Frost Wyrm", "Frost" },
		{ 900334, "Obliterate: +50% Damage", "Frost" },
		{ 900335, "Obliterate: +9 Targets", "Frost" },
		{ 900336, "Howling Blast: +50% Damage", "Frost" },
		{ 900337, "Howling Blast: Diseases", "Frost" },
		{ 900338, "Lichborne: Leech", "Frost" },
		{ 900340, "Icy Talons: Frozen Strikes", "Frost" },
		{ 900343, "Frost Strike: +50% Damage", "Frost" },
		{ 900344, "Frost Strike: +9 Targets", "Frost" },
		{ 900345, "Frost Wyrm: Frozen Breath", "Frost" },
		{ 900366, "Plague Eruption", "Unholy" },
		{ 900369, "Ghoul Cleave", "Unholy" },
		{ 900371, "Risen Army", "Unholy" },
		{ 900373, "Death and Decay: Around You", "Unholy" },
		{ 900375, "Scourge Strike: +50% Damage", "Unholy" },
		{ 900376, "Scourge Strike: +9 Targets", "Unholy" },
		{ 900377, "Death Coil: +50% Damage", "Unholy" },
		{ 900378, "Death Coil: +9 Targets", "Unholy" },
		{ 900379, "Summon Gargoyle: Army", "Unholy" },
		{ 900380, "Corpse Explosion: Corpse Blast", "Unholy" },
		{ 900381, "Corpse Explosion: Corpse Worms", "Unholy" },
	},
	[7] = { -- Shaman
		{ 900400, "Chain Lightning: +6 Targets", "Elemental" },
		{ 900410, "Lightning Bolt: +50% Damage", "Elemental" },
		{ 900411, "Lightning Bolt: +9 Targets", "Elemental" },
		{ 900412, "Chain Lightning: +50% Damage", "Elemental" },
		{ 900401, "Totem Spirits", "Elemental" },
		{ 900402, "Fire Elemental: Ragnaros", "Elemental" },
		{ 900403, "Lava Overload", "Elemental" },
		{ 900404, "Lava Burst: Spreading Flame", "Elemental" },
		{ 900405, "Flame Shock: Lava Reset", "Elemental" },
		{ 900406, "Lava Burst: Two Charges", "Elemental" },
		{ 900407, "Lava Burst: Clearcast Instant", "Elemental" },
		{ 900413, "Lava Burst: +50% Damage", "Elemental" },
		{ 900414, "Lava Burst: +9 Targets", "Elemental" },
		{ 900415, "Elemental Resonance", "Elemental" },
		{ 900418, "Lightning Shield: Chain Lightning", "Elemental" },
		{ 900433, "Totem Spirits", "Enhancement" },
		{ 900434, "Maelstrom Fury", "Enhancement" },
		{ 900435, "Empowered Summons", "Enhancement" },
		{ 900436, "Spirit Wolf Call", "Enhancement" },
		{ 900437, "Spirit Wolves: Haste", "Enhancement" },
		{ 900438, "Spirit Wolves: Chain Lightning", "Enhancement" },
		{ 900441, "Stormstrike: +50% Damage", "Enhancement" },
		{ 900442, "Stormstrike: +9 Targets", "Enhancement" },
		{ 900443, "Lava Lash: +50% Damage", "Enhancement" },
		{ 900444, "Lava Lash: +9 Targets", "Enhancement" },
		{ 900445, "Ghost Wolf: Storm Rush", "Enhancement" },
		{ 900466, "Totem Spirits", "Restoration" },
		{ 900467, "Deep Reserves", "Restoration" },
		{ 900468, "Healing Lightning", "Restoration" },
		{ 900469, "Earth Shield: Spikes", "Restoration" },
		{ 900470, "Earth Shield: Charging Heals", "Restoration" },
		{ 900472, "Earth Shield: Unlimited Targets", "Restoration" },
		{ 900473, "Healing Stream: Surge", "Restoration" },
	},
	[8] = { -- Mage
		{ 900700, "Deep Reserves", "Arcane" },
		{ 900701, "Arcane Barrage: +50% Damage", "Arcane" },
		{ 900702, "Arcane Barrage: +9 Targets", "Arcane" },
		{ 900703, "Arcane Blast: -50% Cast Time", "Arcane" },
		{ 900704, "Arcane Blast: +9 Targets", "Arcane" },
		{ 900705, "Arcane Blast: 8 Charges", "Arcane" },
		{ 900706, "Arcane Explosion: Charges", "Arcane" },
		{ 900707, "Evocation Power", "Arcane" },
		{ 900708, "Emergency Mana Shield", "Arcane" },
		{ 900709, "Blink: To Your Target", "Arcane" },
		{ 900714, "Arcane Overflow", "Arcane" },
		{ 900715, "Mirror Shield", "Arcane" },
		{ 900716, "Mirror Images: Splash", "Arcane" },
		{ 900713, "Targeted Blink", "Arcane" },
		{ 900720, "Blink: Mirror Image", "Arcane" },
		{ 900721, "Mirror Images: Strength in Numbers", "Arcane" },
		{ 900723, "Blink: Through the Enemy", "Arcane" },
		{ 900733, "Fireball: +50% Damage", "Fire" },
		{ 900734, "Fireball: +9 Targets", "Fire" },
		{ 900735, "Pyroblast: +9 Targets", "Fire" },
		{ 900736, "Pyroblast: +50% Damage", "Fire" },
		{ 900737, "Fire Blast: Swift and Sure", "Fire" },
		{ 900738, "Pyroblast: Hot Streak", "Fire" },
		{ 900741, "Meteor", "Fire" },
		{ 900744, "Dragon's Breath: Living Bomb", "Fire" },
		{ 900745, "Molten Armor: Flamestrike", "Fire" },
		{ 900766, "Frostbolt: +50% Damage", "Frost" },
		{ 900767, "Frostbolt: +9 Targets", "Frost" },
		{ 900768, "Ice Lance: +50% Damage", "Frost" },
		{ 900769, "Ice Lance: +9 Targets", "Frost" },
		{ 900770, "Water Elemental: Permanent", "Frost" },
		{ 900771, "Comet Shower", "Frost" },
		{ 900775, "Ice Block: Ice Lance Barrage", "Frost" },
		{ 900776, "Ice Block: Detonation", "Frost" },
	},
	[9] = { -- Warlock
		{ 900800, "Shadow Eruption", "Affliction" },
		{ 900801, "Corruption: +50% Damage", "Affliction" },
		{ 900802, "Spreading Affliction", "Affliction" },
		{ 900804, "Withering Harvest", "Affliction" },
		{ 900807, "Curse of Agony: +50% Damage", "Affliction" },
		{ 900808, "Drain Life: +50% Damage", "Affliction" },
		{ 900809, "Life Tap: Dark Tithe", "Affliction" },
		{ 900811, "Drain Life: +9 Targets", "Affliction" },
		{ 900833, "Metamorphosis: Feast", "Demonology" },
		{ 900834, "Metamorphosis: Fel Vigor", "Demonology" },
		{ 900835, "Lesser Demons", "Demonology" },
		{ 900836, "Imp: +50% Damage", "Demonology" },
		{ 900837, "Imp: Firebolt +9 Targets", "Demonology" },
		{ 900846, "Imp: Firebolt 50% Faster", "Demonology" },
		{ 900848, "Voidwalker: Void Bulwark", "Demonology" },
		{ 900850, "Succubus: Mass Seduction", "Demonology" },
		{ 900852, "Felhunter: Area Spell Lock and Devour Magic", "Demonology" },
		{ 900838, "Felguard: Unlimited Cleave", "Demonology" },
		{ 900839, "Felguard: +50% Damage", "Demonology" },
		{ 900858, "Second Demon", "Demonology" },
		{ 900853, "Shadow Cleave: Unlimited Targets", "Demonology" },
		{ 900854, "Shadow Cleave: +50% Damage", "Demonology" },
		{ 900855, "Felhunter: Spell Shackles", "Demonology" },
		{ 900857, "Demon Charge: Shockwave", "Demonology" },
		{ 900866, "Shadow Bolt: +9 Targets", "Destruction" },
		{ 900867, "Shadow Bolt: +50% Damage", "Destruction" },
		{ 900868, "Chaos Bolt: +50% Damage", "Destruction" },
		{ 900869, "Chaos Bolt: -2 sec Cooldown", "Destruction" },
		{ 900870, "Chaos Bolt: +9 Targets", "Destruction" },
		{ 900873, "Hellfire: Free Movement", "Destruction" },
		{ 900875, "Rain of Fire: Around You", "Destruction" },
		{ 900878, "Soul Fire: +50% Damage", "Destruction" },
		{ 900879, "Soul Fire: +9 Targets", "Destruction" },
		{ 900880, "Immolate: +9 Targets", "Destruction" },
		{ 900881, "Incinerate: Stoked Soul Fire", "Destruction" },
		{ 900883, "Shadow Bolt: Shadow Immolate", "Destruction" },
	},
	[11] = { -- Druid
		{ 901000, "Moonfire: +9 Targets", "Balance" },
		{ 901001, "Moonfire: +50% Damage", "Balance" },
		{ 901002, "Starfall: +50% Area", "Balance" },
		{ 901003, "Starfall: +50% Damage", "Balance" },
		{ 901004, "Starfall: Starlight Reset", "Balance" },
		{ 901005, "Starfall: 10 Stacks", "Balance" },
		{ 901006, "Lunar Frenzy", "Balance" },
		{ 901008, "Wrath: Starfire Surge", "Balance" },
		{ 901010, "Force of Nature: Star Owls", "Balance" },
		{ 901033, "Swipe (Bear): Bleed", "Feral" },
		{ 901035, "Maul: Bleeding Wounds", "Feral" },
		{ 901036, "Swipe (Bear): +50% Damage", "Feral" },
		{ 901037, "Thorns: Open Wounds", "Feral" },
		{ 901039, "Ursine Bulwark", "Feral" },
		{ 901049, "Swipe (Cat): Bleed", "Feral" },
		{ 901051, "Energy: +50% Regeneration", "Feral" },
		{ 901052, "Swipe (Cat): +50% Damage", "Feral" },
		{ 901053, "Frontal Assault", "Feral" },
		{ 901054, "Berserk: Unleashed", "Feral" },
		{ 901042, "Maul: +50% Damage", "Feral" },
		{ 901043, "Maul: +9 Targets", "Feral" },
		{ 901057, "Claw: +50% Damage", "Feral" },
		{ 901058, "Claw: +9 Targets", "Feral" },
		{ 901059, "Shred: +50% Damage", "Feral" },
		{ 901060, "Shred: +9 Targets", "Feral" },
		{ 901066, "Treant Call", "Restoration" },
		{ 901067, "Summons: Healing Power", "Restoration" },
		{ 901068, "Summons: Parting Bloom", "Restoration" },
		{ 901069, "Thorns: Rejuvenation", "Restoration" },
		{ 901070, "Heal over Time: +50%", "Restoration" },
		{ 901071, "Heal over Time: Double Speed", "Restoration" },
		{ 901072, "Deep Reserves", "Restoration" },
	},
}

-- Available to every class (901100 Cast Moving stays excluded: no
-- server implementation yet)
local GLOBAL_SPELLS = {
	{ 901101, "Killing Blow: Recovery", "Global" },
	{ 901102, "Extra Attack", "Global" },
	{ 901103, "Cleaving Strikes", "Global" },
	{ 901104, "Riposte", "Global" },
	{ 901111, "Empowered", "Global" },
}

-- The 21 Chapters-of-Azeroth classes (ids 12-32) have no CLASS_SPELLS list:
-- every entry above is a modifier for a named stock ability (Moonfire, Shadow
-- Bolt, Pyroblast), and a CoA class casts none of them. They still get the
-- GLOBAL_SPELLS, which key off events rather than off a spell - and they used
-- to get nothing at all, because CLASS_SPELLS[class] was nil and SendState
-- returned before it reached the global rows. The legacy class's list is
-- deliberately NOT inherited: it would offer a Starcaller a "Moonfire +9
-- Targets" toggle for a spell the class cannot cast.
--
-- The pair below MIRRORS the core: SharedDefines.h IsAscensionClass()
-- (CLASS_BARBARIAN 12 .. CLASS_SPIRIT_MAGE 32). Eluna Lua cannot read a C++
-- header, so this is one of the two mirrors the core header names; the other
-- is mod-procedural-dungeon's PD_CLASS_CUSTOM_FIRST / _LAST. If the range ever
-- moves, all three move together.
local CUSTOM_CLASS_FIRST = 12
local CUSTOM_CLASS_LAST = 32
local NO_CLASS_SPELLS = {}

-- classId -> { [spellId] = true } for O(1) Toggle validation
local ALLOWED = {}
local function BuildAllowed(classId, list)
	local set = {}
	for _, entry in ipairs(list) do
		set[entry[1]] = true
	end
	for _, entry in ipairs(GLOBAL_SPELLS) do
		set[entry[1]] = true
	end
	ALLOWED[classId] = set
end

for classId, list in pairs(CLASS_SPELLS) do
	BuildAllowed(classId, list)
end
for classId = CUSTOM_CLASS_FIRST, CUSTOM_CLASS_LAST do
	BuildAllowed(classId, NO_CLASS_SPELLS)
end

local function GetSpellList(player)
	local classId = player:GetClass()
	if classId >= CUSTOM_CLASS_FIRST and classId <= CUSTOM_CLASS_LAST then
		return NO_CLASS_SPELLS
	end
	return CLASS_SPELLS[classId]
end

-- AccountTypes SEC_GAMEMASTER. The command behind the GM button is gated
-- on RBAC_PERM_COMMAND_MODIFY, which the default RBAC linkage grants from
-- this security level up. This check decides who sees the button and drops
-- a forged request early; the core still checks the permission itself when
-- the command runs.
local SEC_GAMEMASTER = 2

local function IsGameMaster(player)
	return player:GetGMRank() >= SEC_GAMEMASTER
end

-- ============================================================
-- State push: full row list incl. learned flags as ONE table
-- arg (avoids the 15-arg limit per msg:Add), plus the GM flag
-- (1/0) that shows the Learn all Talents button
-- ============================================================

local function SendState(player)
	local list = GetSpellList(player)
	if not list then
		return
	end

	local rows = {}
	for _, entry in ipairs(list) do
		rows[#rows + 1] = {
			id = entry[1],
			name = entry[2],
			spec = entry[3],
			learned = player:HasSpell(entry[1]) and 1 or 0,
		}
	end
	for _, entry in ipairs(GLOBAL_SPELLS) do
		rows[#rows + 1] = {
			id = entry[1],
			name = entry[2],
			spec = entry[3],
			learned = player:HasSpell(entry[1]) and 1 or 0,
		}
	end

	AIO.Msg():Add("CustomSpells", "State", rows,
		IsGameMaster(player) and 1 or 0):Send(player)
end

local function DenyInCombat(player)
	if player:IsInCombat() then
		player:SendBroadcastMessage("|cffff5555Custom Spells:|r cannot change spells while in combat.")
		return true
	end
	return false
end

-- ============================================================
-- Handlers
-- ============================================================

function CustomSpells_ServerHandlers.Show(player)
	SendState(player)
end

function CustomSpells_ServerHandlers.Toggle(player, spellId)
	spellId = tonumber(spellId)
	if not spellId then
		return
	end

	local allowed = ALLOWED[player:GetClass()]
	if not allowed or not allowed[spellId] then
		return
	end

	if DenyInCombat(player) then
		SendState(player)
		return
	end

	if player:HasSpell(spellId) then
		player:RemoveSpell(spellId)
	else
		player:LearnSpell(spellId)
	end

	SendState(player)
end

function CustomSpells_ServerHandlers.SetAll(player, learn)
	local list = GetSpellList(player)
	if not list then
		return
	end

	if DenyInCombat(player) then
		SendState(player)
		return
	end

	local all = {}
	for _, entry in ipairs(list) do
		all[#all + 1] = entry[1]
	end
	for _, entry in ipairs(GLOBAL_SPELLS) do
		all[#all + 1] = entry[1]
	end

	for _, spellId in ipairs(all) do
		if learn == 1 then
			if not player:HasSpell(spellId) then
				player:LearnSpell(spellId)
			end
		else
			if player:HasSpell(spellId) then
				player:RemoveSpell(spellId)
			end
		end
	end

	SendState(player)
end

-- GM only: every Forgotten Talents node at its maximum rank, free of charge.
-- The rank is checked again here, whatever the client showed, and a forged
-- request is dropped like an unknown Toggle. The learn-all itself is the C++
-- command, run as the player's own chat command: the core applies its RBAC
-- permission and GM log exactly as if typed. The command targets the
-- selected player when it gets no name, so the player's own GUID goes with
-- it - as %.0f, because Lua 5.2's %d goes through a C long, which is 32 bits
-- on Windows. Its result and any refusal (module disabled) reach the chat
-- frame.
function CustomSpells_ServerHandlers.LearnAllTalents(player)
	if not IsGameMaster(player) then
		return
	end

	player:RunCommand(string.format("forgotten learnall %.0f",
		player:GetGUIDLow()))
	SendState(player)
end

AIO.AddHandlers("CustomSpells", CustomSpells_ServerHandlers)
