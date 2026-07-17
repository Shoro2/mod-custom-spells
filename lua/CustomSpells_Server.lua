-- =============================================================
-- mod-custom-spells — Server-side AIO (Eluna)
--
-- Spell picker: players learn/forget the custom class spells
-- (900xxx markers/actives) via toggles in the client UI.
-- The whitelist below is the single source of truth for what a
-- class may learn; every Toggle request is validated against it.
-- Helper spells, orphaned markers and legacy rows are excluded.
--
-- Regeneration: the table mirrors acore_world.spell_dbc passives
-- (plus the player-castable actives and the warrior real-DBC
-- block 900100-900121); see share-public claude_log 2026-07-18.
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
		{ 900100, "Improved Mortal Strike Damage", "Arms" },
		{ 900101, "Improved Mortal Strike CDR", "Arms" },
		{ 900102, "Improved Overpower Damage", "Arms" },
		{ 900103, "Mortal Strike Cleave", "Arms" },
		{ 900104, "Overpower Cleave", "Arms" },
		{ 900105, "Critical Execution", "Arms" },
		{ 900106, "Critical Execution (Strike)", "Arms" },
		{ 900107, "Timed Attacks", "Arms" },
		{ 900108, "Whirlwind Unlimited Targets", "Fury" },
		{ 900109, "Improved Bloodthirst Damage", "Fury" },
		{ 900110, "Bloodthirst Cleave", "Fury" },
		{ 900111, "Improved Whirlwind Damage", "Fury" },
		{ 900112, "Cleave Unlimited Targets", "Fury" },
		{ 900114, "Whirly Attacks", "Fury" },
		{ 900116, "Bloody Whirlwind", "Fury" },
		{ 900117, "Speedy Bloodthirst", "Fury" },
		{ 900118, "Whirlwind: Overpower", "Fury" },
		{ 900119, "Whirlwind: Bloodthirst", "Fury" },
		{ 900168, "Revenge Damage", "Protection" },
		{ 900169, "Revenge AoE", "Protection" },
		{ 900170, "TC Rend + Sunder", "Protection" },
		{ 900171, "TC Damage", "Protection" },
		{ 900172, "Block AoE", "Protection" },
		{ 900173, "Block TC", "Protection" },
	},
	[2] = { -- Paladin
		{ 900200, "HS AoE Damage", "Holy" },
		{ 900201, "HS AoE Heal", "Holy" },
		{ 900202, "HS Both", "Holy" },
		{ 900203, "HS +50%", "Holy" },
		{ 900204, "Consec Heal", "Holy" },
		{ 900205, "Consec Around You", "Holy" },
		{ 900206, "Consec +50%", "Holy" },
		{ 900207, "Consec +5s", "Holy" },
		{ 900234, "Consec Around You", "Protection" },
		{ 900235, "AS +9 Targets", "Protection" },
		{ 900236, "AS +50%", "Protection" },
		{ 900237, "HS +99 Charges", "Protection" },
		{ 900238, "HS +50%", "Protection" },
		{ 900239, "AS Consec", "Protection" },
		{ 900240, "Judge AS", "Protection" },
		{ 900241, "Judge -2s CD", "Protection" },
		{ 900268, "Consec Around You", "Retribution" },
		{ 900269, "Judge -2s CD", "Retribution" },
		{ 900271, "DS +50%", "Retribution" },
		{ 900272, "CS +50%", "Retribution" },
		{ 900273, "CS +9 Targets", "Retribution" },
		{ 900274, "Exorcism Proc", "Retribution" },
	},
	[3] = { -- Hunter
		{ 900500, "Get Back Arrows", "Shared" },
		{ 900501, "Multishot AoE", "Shared" },
		{ 900502, "Pet Damage +50%", "Beast Mastery" },
		{ 900503, "Pet Speed +50%", "Beast Mastery" },
		{ 900504, "Pet AoE Proc", "Beast Mastery" },
		{ 900533, "Autoshot Bounce", "Marksmanship" },
		{ 900534, "Barrage", "Marksmanship" },
		{ 900566, "Trap Proc", "Survival" },
	},
	[4] = { -- Rogue
		{ 900600, "Energy +50%", "Assassination" },
		{ 900601, "Mutilate +50%", "Assassination" },
		{ 900602, "Poison +50%", "Assassination" },
		{ 900603, "Poison Nova", "Assassination" },
		{ 900633, "SS +50%", "Combat" },
		{ 900634, "SS +9 Targets", "Combat" },
		{ 900635, "BF 2min", "Combat" },
		{ 900636, "BF +9 Targets", "Combat" },
		{ 900637, "Energy +50%", "Combat" },
		{ 900666, "Energy +50%", "Subtlety" },
		{ 900667, "Hemo +50%", "Subtlety" },
		{ 900668, "Hemo +9 Targets", "Subtlety" },
	},
	[5] = { -- Priest
		{ 900900, "Shield Explode", "Discipline" },
		{ 900901, "Shield +50%", "Discipline" },
		{ 900902, "Weakened Soul CD", "Discipline" },
		{ 900933, "Heal Fire", "Holy" },
		{ 900966, "DoT AoE", "Shadow" },
		{ 900967, "DoT Spread", "Shadow" },
	},
	[6] = { -- Death Knight
		{ 900300, "3 Rune Weapons", "Blood" },
		{ 900301, "Double Cast", "Blood" },
		{ 900302, "HS +50%", "Blood" },
		{ 900303, "HS +9 Targets", "Blood" },
		{ 900304, "DC Proc", "Blood" },
		{ 900333, "Frost Wyrm", "Frost" },
		{ 900366, "DoT AoE Proc", "Unholy" },
	},
	[7] = { -- Shaman
		{ 900400, "CL +6 Targets", "Elemental" },
		{ 900401, "Totem Follow", "Elemental" },
		{ 900402, "Ragnaros", "Elemental" },
		{ 900403, "LO + LvB", "Elemental" },
		{ 900404, "LvB Spread FS", "Elemental" },
		{ 900405, "FS Reset LvB", "Elemental" },
		{ 900406, "LvB Charges", "Elemental" },
		{ 900407, "Instant LvB", "Elemental" },
		{ 900433, "Totem Follow", "Enhancement" },
		{ 900434, "Maelstrom AoE", "Enhancement" },
		{ 900436, "Wolf Summon", "Enhancement" },
		{ 900437, "Wolf Haste", "Enhancement" },
		{ 900438, "Wolf CL", "Enhancement" },
		{ 900466, "Totem Follow", "Restoration" },
		{ 900467, "Mana Regen", "Restoration" },
	},
	[8] = { -- Mage
		{ 900700, "Mana Regen", "Arcane" },
		{ 900701, "Barrage +50%", "Arcane" },
		{ 900702, "Barrage +9", "Arcane" },
		{ 900703, "Blast -50% Cast", "Arcane" },
		{ 900704, "Blast +9", "Arcane" },
		{ 900705, "Charges x8", "Arcane" },
		{ 900706, "AE Charges", "Arcane" },
		{ 900707, "Evoc Power", "Arcane" },
		{ 900708, "Emergency Shield", "Arcane" },
		{ 900713, "Targeted Blink", "Arcane" },
		{ 900733, "Fireball +50%", "Fire" },
		{ 900734, "Fireball +9", "Fire" },
		{ 900735, "Pyro +9", "Fire" },
		{ 900736, "Pyro +50%", "Fire" },
		{ 900738, "Pyro Hot Streak", "Fire" },
		{ 900766, "Frostbolt +50%", "Frost" },
		{ 900767, "Frostbolt +9", "Frost" },
		{ 900768, "Ice Lance +50%", "Frost" },
		{ 900769, "Ice Lance +9", "Frost" },
		{ 900770, "Perm Elemental", "Frost" },
		{ 900771, "Comet Shower", "Frost" },
	},
	[9] = { -- Warlock
		{ 900800, "DoT AoE", "Affliction" },
		{ 900801, "Corruption +50%", "Affliction" },
		{ 900802, "DoT Spread", "Affliction" },
		{ 900833, "Meta Kill Extend", "Demonology" },
		{ 900834, "Meta AoE+Heal", "Demonology" },
		{ 900835, "Lesser Spawn", "Demonology" },
		{ 900836, "Imp FB +50%", "Demonology" },
		{ 900837, "Imp FB AoE", "Demonology" },
		{ 900838, "FG Unlim", "Demonology" },
		{ 900839, "FG +50%", "Demonology" },
		{ 900840, "Sacrifice All", "Demonology" },
		{ 900866, "SB +9", "Destruction" },
		{ 900867, "SB +50%", "Destruction" },
		{ 900868, "CB +50%", "Destruction" },
		{ 900869, "CB CD -2s", "Destruction" },
		{ 900870, "CB +9", "Destruction" },
	},
	[11] = { -- Druid
		{ 901000, "MF +9 Targets", "Balance" },
		{ 901001, "MF +50%", "Balance" },
		{ 901002, "SF +9 Targets", "Balance" },
		{ 901003, "SF +50%", "Balance" },
		{ 901004, "SF CD Reduce", "Balance" },
		{ 901005, "SF Stacks 10", "Balance" },
		{ 901033, "Bear Bleed", "Feral" },
		{ 901049, "Cat Bleed", "Feral" },
		{ 901051, "Energy +50%", "Feral" },
		{ 901066, "HoT Treant", "Restoration" },
		{ 901067, "Summon Scale", "Restoration" },
		{ 901068, "Summon Heal", "Restoration" },
		{ 901069, "Thorns Rejuv", "Restoration" },
		{ 901070, "HoTs +50%", "Restoration" },
		{ 901071, "HoTs 2x", "Restoration" },
		{ 901072, "Mana Regen", "Restoration" },
	},
}

-- Available to every class (901100 Cast Moving stays excluded: no
-- server implementation yet)
local GLOBAL_SPELLS = {
	{ 901101, "Kill Heal 5%", "Global" },
	{ 901102, "Extra Attack 25%", "Global" },
	{ 901103, "Cleave Proc 10%", "Global" },
	{ 901104, "Counter Attack", "Global" },
}

-- classId -> { [spellId] = true } for O(1) Toggle validation
local ALLOWED = {}
for classId, list in pairs(CLASS_SPELLS) do
	local set = {}
	for _, entry in ipairs(list) do
		set[entry[1]] = true
	end
	for _, entry in ipairs(GLOBAL_SPELLS) do
		set[entry[1]] = true
	end
	ALLOWED[classId] = set
end

local function GetSpellList(player)
	return CLASS_SPELLS[player:GetClass()]
end

-- ============================================================
-- State push: full row list incl. learned flags as ONE table
-- arg (avoids the 15-arg limit per msg:Add)
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

	AIO.Msg():Add("CustomSpells", "State", rows):Send(player)
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

AIO.AddHandlers("CustomSpells", CustomSpells_ServerHandlers)
