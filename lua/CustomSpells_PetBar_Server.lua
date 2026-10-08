-- =============================================================
-- mod-custom-spells - second pet control bar, server side (AIO)
--
-- The bar of CustomSpells_PetBar_Client.lua commands the second pet
-- of the hunter's "Second Pet" (900525, Beast Mastery) and the
-- warlock's "Second Demon" (900858, Demonology). The pet and its
-- commands live in C++ (src/custom_spells_second_pet.cpp): ALE cannot
-- reach the pet's charm data (its spells, autocast, command and react
-- state), so every request goes on as the player's own chat command
-- .cspet (player:RunCommand: the player's session, as if typed), which
-- checks it once more - the player's own live second pet, a valid
-- attack target, one of the pet's spells. The bar's state comes from
-- C++ directly, as addon whispers with the prefix CSPB.
--
-- This file checks what the client sent before anything runs: a known
-- request, whole numbers in range, at most MAX_REQUESTS_PER_SECOND
-- requests per player.
-- =============================================================

local AIO = AIO or require("AIO")

local HANDLER_NAME = "CustomSpellsPetBar"
local MAX_REQUESTS_PER_SECOND = 12
local MAX_SPELL_ID = 16777215 -- 24 bits, as a pet action button holds

-- ReactStates (0 passive, 1 defensive, 2 aggressive) -> .cspet subcommand
local REACT_COMMANDS = {
	[0] = "passive",
	[1] = "defensive",
	[2] = "aggressive",
}

local PLAYER_EVENT_ON_LOGOUT = 4

-- [guid low] = { start = ms, count = requests in that second }
local requestWindows = {}

local function Allowed(player)
	local key = player:GetGUIDLow()
	local window = requestWindows[key]
	if not window or GetTimeDiff(window.start) >= 1000 then
		requestWindows[key] = { start = GetCurrTime(), count = 1 }
		return true
	end
	if window.count >= MAX_REQUESTS_PER_SECOND then
		return false
	end
	window.count = window.count + 1
	return true
end

-- A whole number in [low, high], or nil
local function WholeNumber(value, low, high)
	value = tonumber(value)
	if not value or value ~= math.floor(value) or value < low or value > high then
		return nil
	end
	return value
end

local function Run(player, command)
	if Allowed(player) then
		player:RunCommand("cspet " .. command)
	end
end

local Handlers = {}

function Handlers.Attack(player)
	Run(player, "attack")
end

function Handlers.Follow(player)
	Run(player, "follow")
end

function Handlers.Stay(player)
	Run(player, "stay")
end

function Handlers.React(player, mode)
	mode = WholeNumber(mode, 0, 2)
	if mode then
		Run(player, REACT_COMMANDS[mode])
	end
end

function Handlers.Cast(player, spellId)
	spellId = WholeNumber(spellId, 1, MAX_SPELL_ID)
	if spellId then
		Run(player, string.format("cast %d", spellId))
	end
end

function Handlers.Autocast(player, spellId, on)
	spellId = WholeNumber(spellId, 1, MAX_SPELL_ID)
	on = WholeNumber(on, 0, 1)
	if spellId and on then
		Run(player, string.format("autocast %s %d", on == 1 and "on" or "off", spellId))
	end
end

-- The bar asks for its whole state when it loads (login, /reload, /aio reset)
function Handlers.Sync(player)
	Run(player, "sync")
end

AIO.AddHandlers(HANDLER_NAME, Handlers)

RegisterPlayerEvent(PLAYER_EVENT_ON_LOGOUT, function(_, player)
	requestWindows[player:GetGUIDLow()] = nil
end)
