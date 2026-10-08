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
		{ 900234, "Consecration: Follows You", "Protection" },
		{ 900235, "Avenger's Shield: +9 Targets", "Protection" },
		{ 900236, "Avenger's Shield: +50% Damage", "Protection" },
		{ 900237, "Holy Shield: +99 Charges", "Protection" },
		{ 900238, "Holy Shield: +50% Damage", "Protection" },
		{ 900239, "Avenger's Shield: Consecration", "Protection" },
		{ 900240, "Judgement: Avenger's Shield", "Protection" },
		{ 900241, "Judgement: -2 sec Cooldown", "Protection" },
		{ 900268, "Consecration: Follows You", "Retribution" },
		{ 900269, "Judgement: -2 sec Cooldown", "Retribution" },
		{ 900270, "Divine Storm: +6 Targets", "Retribution" },
		{ 900271, "Divine Storm: +50% Damage", "Retribution" },
		{ 900272, "Crusader Strike: +50% Damage", "Retribution" },
		{ 900273, "Crusader Strike: +9 Targets", "Retribution" },
		{ 900274, "Exorcism Power", "Retribution" },
	},
	[3] = { -- Hunter
		{ 900500, "Recovered Arrows", "Shared" },
		{ 900501, "Multi-Shot: Unlimited Targets", "Shared" },
		{ 900502, "Pet: +50% Damage", "Beast Mastery" },
		{ 900503, "Pet: +50% Attack Speed", "Beast Mastery" },
		{ 900504, "Pet: Beast Cleave", "Beast Mastery" },
		{ 900533, "Auto Shot: Ricochet", "Marksmanship" },
		{ 900534, "Barrage", "Marksmanship" },
		{ 900566, "Explosive Shots", "Survival" },
	},
	[4] = { -- Rogue
		{ 900600, "Energy: +50% Regeneration", "Assassination" },
		{ 900601, "Mutilate: +50% Damage", "Assassination" },
		{ 900602, "Poisons: +50% Damage", "Assassination" },
		{ 900603, "Poison Nova", "Assassination" },
		{ 900633, "Sinister Strike: +50% Damage", "Combat" },
		{ 900634, "Sinister Strike: +9 Targets", "Combat" },
		{ 900635, "Blade Flurry: 2 min Duration", "Combat" },
		{ 900636, "Blade Flurry: +9 Targets", "Combat" },
		{ 900637, "Energy: +50% Regeneration", "Combat" },
		{ 900666, "Energy: +50% Regeneration", "Subtlety" },
		{ 900667, "Hemorrhage: +50% Damage", "Subtlety" },
		{ 900668, "Hemorrhage: +9 Targets", "Subtlety" },
	},
	[5] = { -- Priest
		{ 900900, "Power Word: Shield: Explosion", "Discipline" },
		{ 900901, "Power Word: Shield: +50% Absorption", "Discipline" },
		{ 900902, "Weakened Soul: 5 sec", "Discipline" },
		{ 900933, "Holy Fire Heals", "Holy" },
		{ 900966, "Shadow Eruption", "Shadow" },
		{ 900967, "Spreading Shadows", "Shadow" },
	},
	[6] = { -- Death Knight
		{ 900300, "Dancing Rune Weapon: Three Blades", "Blood" },
		{ 900301, "Dancing Rune Weapon: Double Cast", "Blood" },
		{ 900302, "Heart Strike: +50% Damage", "Blood" },
		{ 900303, "Heart Strike: +9 Targets", "Blood" },
		{ 900304, "Death Coil Strikes", "Blood" },
		{ 900306, "Bloodworm Burst", "Blood" },
		{ 900333, "Raise Dead: Frost Wyrm", "Frost" },
		{ 900334, "Obliterate: +50% Damage", "Frost" },
		{ 900335, "Obliterate: +9 Targets", "Frost" },
		{ 900336, "Howling Blast: +50% Damage", "Frost" },
		{ 900337, "Howling Blast: Frost Fever", "Frost" },
		{ 900338, "Lichborne: Leech", "Frost" },
		{ 900340, "Icy Talons: Frozen Strikes", "Frost" },
		{ 900366, "Plague Eruption", "Unholy" },
		{ 900369, "Ghoul Cleave", "Unholy" },
		{ 900371, "Risen Army", "Unholy" },
		{ 900373, "Death and Decay: Around You", "Unholy" },
	},
	[7] = { -- Shaman
		{ 900400, "Chain Lightning: +6 Targets", "Elemental" },
		{ 900410, "Lightning Bolt: +50% Damage", "Elemental" },
		{ 900411, "Lightning Bolt: +9 Targets", "Elemental" },
		{ 900412, "Chain Lightning: +50% Damage", "Elemental" },
		{ 900401, "Totems Follow You", "Elemental" },
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
		{ 900433, "Totems Follow You", "Enhancement" },
		{ 900434, "Maelstrom Fury", "Enhancement" },
		{ 900435, "Empowered Summons", "Enhancement" },
		{ 900436, "Spirit Wolf Call", "Enhancement" },
		{ 900437, "Spirit Wolves: Haste", "Enhancement" },
		{ 900438, "Spirit Wolves: Chain Lightning", "Enhancement" },
		{ 900466, "Totems Follow You", "Restoration" },
		{ 900467, "Deep Reserves", "Restoration" },
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
		{ 900733, "Fireball: +50% Damage", "Fire" },
		{ 900734, "Fireball: +9 Targets", "Fire" },
		{ 900735, "Pyroblast: +9 Targets", "Fire" },
		{ 900736, "Pyroblast: +50% Damage", "Fire" },
		{ 900737, "Fire Blast: Swift and Sure", "Fire" },
		{ 900738, "Pyroblast: Hot Streak", "Fire" },
		{ 900741, "Meteor", "Fire" },
		{ 900766, "Frostbolt: +50% Damage", "Frost" },
		{ 900767, "Frostbolt: +9 Targets", "Frost" },
		{ 900768, "Ice Lance: +50% Damage", "Frost" },
		{ 900769, "Ice Lance: +9 Targets", "Frost" },
		{ 900770, "Water Elemental: Permanent", "Frost" },
		{ 900771, "Comet Shower", "Frost" },
	},
	[9] = { -- Warlock
		{ 900800, "Shadow Eruption", "Affliction" },
		{ 900801, "Corruption: +50% Damage", "Affliction" },
		{ 900802, "Spreading Affliction", "Affliction" },
		{ 900833, "Metamorphosis: Feast", "Demonology" },
		{ 900834, "Metamorphosis: Shadow Pulse", "Demonology" },
		{ 900835, "Lesser Demons", "Demonology" },
		{ 900836, "Imp: +50% Damage", "Demonology" },
		{ 900837, "Imp: Firebolt +9 Targets", "Demonology" },
		{ 900838, "Felguard: Unlimited Cleave", "Demonology" },
		{ 900839, "Felguard: +50% Damage", "Demonology" },
		{ 900866, "Shadow Bolt: +9 Targets", "Destruction" },
		{ 900867, "Shadow Bolt: +50% Damage", "Destruction" },
		{ 900868, "Chaos Bolt: +50% Damage", "Destruction" },
		{ 900869, "Chaos Bolt: -2 sec Cooldown", "Destruction" },
		{ 900870, "Chaos Bolt: +9 Targets", "Destruction" },
	},
	[11] = { -- Druid
		{ 901000, "Moonfire: +9 Targets", "Balance" },
		{ 901001, "Moonfire: +50% Damage", "Balance" },
		{ 901002, "Starfall: +9 Targets", "Balance" },
		{ 901003, "Starfall: +50% Damage", "Balance" },
		{ 901004, "Starfall: Starlight Reset", "Balance" },
		{ 901005, "Starfall: 10 Stacks", "Balance" },
		{ 901033, "Swipe (Bear): Bleed", "Feral" },
		{ 901035, "Maul: Bleeding Wounds", "Feral" },
		{ 901049, "Swipe (Cat): Bleed", "Feral" },
		{ 901051, "Energy: +50% Regeneration", "Feral" },
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
