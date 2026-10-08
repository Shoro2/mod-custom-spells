-- =============================================================
-- mod-custom-spells - second pet control bar, client side (AIO)
--
-- The hunter's "Second Pet" (900525) and the warlock's "Second Demon"
-- (900858) put a second pet next to the real one. The 3.3.5 client
-- knows one pet only (one pet bar, pet frame, talent tab), so the
-- second one gets this bar: Attack, Follow, Stay, its spells (click:
-- cast at your target, right-click: autocast on/off), the react modes
-- Aggressive / Defensive / Passive, its name, health and power. It
-- commands the second pet only; the native pet bar keeps commanding
-- the real pet. The bar shows while a second pet exists and can be
-- dragged by its frame.
--
-- No unit token exists for the second pet, so the server sends its
-- state (C++, addon whispers with the prefix CSPB):
--   X                                         no second pet
--   F;name;level;power type;react;command;spells
--       spells = id:autocastable:autocast,...  react 0 passive, 1 defensive,
--       2 aggressive; command 0 stay, 1 follow
--   S;alive;health;max health;power;max power;attacking
--   C;spell id;cooldown left in ms
-- and the clicks go back as AIO requests (CustomSpells_PetBar_Server.lua).
-- =============================================================

local AIO = AIO or require("AIO")
if AIO.AddAddon() then return end

local HANDLER_NAME = "CustomSpellsPetBar"
local STATE_PREFIX = "CSPB"

local BUTTON_SIZE = 30
local BUTTON_GAP = 6
local SPELL_SLOTS = 4
local MARGIN = 12

local POWER_COLORS = {
	[0] = { 0.00, 0.00, 1.00 }, -- mana
	[1] = { 1.00, 0.00, 0.00 }, -- rage
	[2] = { 1.00, 0.50, 0.25 }, -- focus
	[3] = { 1.00, 1.00, 0.00 }, -- energy
}

-- ============================================================
-- Frame
-- ============================================================

local frame = CreateFrame("Frame", "CustomSpellsPetBarFrame", UIParent)
frame:SetSize(2 * MARGIN + 10 * BUTTON_SIZE + 9 * BUTTON_GAP, 82)
frame:SetPoint("BOTTOM", UIParent, "BOTTOM", 0, 240)
frame:SetBackdrop({
	bgFile = "Interface\\Tooltips\\UI-Tooltip-Background",
	edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
	tile = true, tileSize = 16, edgeSize = 16,
	insets = { left = 4, right = 4, top = 4, bottom = 4 },
})
frame:SetBackdropColor(0, 0, 0, 0.8)
frame:SetMovable(true)
frame:EnableMouse(true)
frame:SetClampedToScreen(true)
frame:RegisterForDrag("LeftButton")
frame:SetScript("OnDragStart", frame.StartMoving)
frame:SetScript("OnDragStop", frame.StopMovingOrSizing)
frame:Hide()
if AIO.SavePosition then
	AIO.SavePosition(frame, true)
end

local nameText = frame:CreateFontString(nil, "OVERLAY", "GameFontNormal")
nameText:SetPoint("TOPLEFT", MARGIN, -10)
nameText:SetWidth(120)
nameText:SetJustifyH("LEFT")

local function CreateBar(height)
	local bar = CreateFrame("StatusBar", nil, frame)
	bar:SetSize(frame:GetWidth() - 2 * MARGIN - 130, height)
	bar:SetStatusBarTexture("Interface\\TargetingFrame\\UI-StatusBar")
	bar:SetMinMaxValues(0, 1)
	bar:SetValue(0)
	bar.background = bar:CreateTexture(nil, "BACKGROUND")
	bar.background:SetAllPoints()
	bar.background:SetTexture(0, 0, 0, 0.6)
	bar.text = bar:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
	bar.text:SetPoint("CENTER")
	return bar
end

local healthBar = CreateBar(12)
healthBar:SetPoint("TOPRIGHT", -MARGIN, -9)
healthBar:SetStatusBarColor(0, 0.8, 0)

local powerBar = CreateBar(9)
powerBar:SetPoint("TOPRIGHT", healthBar, "BOTTOMRIGHT", 0, -2)

-- ============================================================
-- Buttons: Attack Follow Stay | 4 spells | Aggressive Defensive Passive
-- (the native pet bar's order)
-- ============================================================

local buttons = {}

local function HideTooltip(self)
	if GameTooltip:IsOwned(self) then
		GameTooltip:Hide()
	end
end

local function CreateButton(index)
	local button = CreateFrame("Button", "CustomSpellsPetBarButton" .. index, frame)
	button:SetSize(BUTTON_SIZE, BUTTON_SIZE)
	button:SetPoint("BOTTOMLEFT", MARGIN + (index - 1) * (BUTTON_SIZE + BUTTON_GAP), 12)
	button:RegisterForClicks("LeftButtonUp", "RightButtonUp")

	button.icon = button:CreateTexture(nil, "BACKGROUND")
	button.icon:SetAllPoints()
	button.icon:SetTexCoord(0.07, 0.93, 0.07, 0.93)

	button.border = button:CreateTexture(nil, "BORDER")
	button.border:SetTexture("Interface\\Buttons\\UI-Quickslot2")
	button.border:SetSize(BUTTON_SIZE * 1.8, BUTTON_SIZE * 1.8)
	button.border:SetPoint("CENTER")

	button:SetPushedTexture("Interface\\Buttons\\UI-Quickslot-Depress")
	button:SetHighlightTexture("Interface\\Buttons\\ButtonHilight-Square", "ADD")

	-- the active command / react mode
	button.active = button:CreateTexture(nil, "OVERLAY")
	button.active:SetTexture("Interface\\Buttons\\CheckButtonHilight")
	button.active:SetBlendMode("ADD")
	button.active:SetAllPoints()
	button.active:Hide()

	-- autocastable (corner brackets) and autocast on (a pulsing gold border)
	button.autocastable = button:CreateTexture(nil, "OVERLAY")
	button.autocastable:SetTexture("Interface\\Buttons\\UI-AutoCastableOverlay")
	button.autocastable:SetSize(BUTTON_SIZE * 1.93, BUTTON_SIZE * 1.93)
	button.autocastable:SetPoint("CENTER")
	button.autocastable:Hide()

	button.autocast = button:CreateTexture(nil, "OVERLAY")
	button.autocast:SetTexture("Interface\\Buttons\\UI-ActionButton-Border")
	button.autocast:SetBlendMode("ADD")
	button.autocast:SetVertexColor(1, 0.82, 0)
	button.autocast:SetSize(BUTTON_SIZE * 1.9, BUTTON_SIZE * 1.9)
	button.autocast:SetPoint("CENTER")
	button.autocast:Hide()

	button.cooldown = CreateFrame("Cooldown", nil, button)
	button.cooldown:SetAllPoints(button.icon)
	button.cooldown:Hide()

	button:SetScript("OnLeave", HideTooltip)
	button:SetScript("OnHide", HideTooltip)
	buttons[index] = button
	return button
end

local function Request(name, ...)
	AIO.Handle(HANDLER_NAME, name, ...)
end

local function SetCommandButton(button, icon, title, text, onClick)
	button.icon:SetTexture(icon)
	button:SetScript("OnClick", onClick)
	button:SetScript("OnEnter", function(self)
		GameTooltip:SetOwner(self, "ANCHOR_TOP")
		GameTooltip:SetText(title, 1, 1, 1)
		GameTooltip:AddLine(text, nil, nil, nil, true)
		GameTooltip:Show()
	end)
end

for index = 1, 10 do
	CreateButton(index)
end

local attackButton, followButton, stayButton = buttons[1], buttons[2], buttons[3]

SetCommandButton(attackButton, PET_ATTACK_TEXTURE or "Interface\\Icons\\Ability_GhoulFrenzy",
	PET_ACTION_ATTACK or "Attack", "Your second pet attacks your target.",
	function() Request("Attack") end)
SetCommandButton(followButton, PET_FOLLOW_TEXTURE or "Interface\\Icons\\Ability_Tracking",
	PET_ACTION_FOLLOW or "Follow", "Your second pet follows you.",
	function() Request("Follow") end)
SetCommandButton(stayButton, PET_WAIT_TEXTURE or "Interface\\Icons\\Spell_Nature_TimeStop",
	PET_ACTION_WAIT or "Stay", "Your second pet stays where it is.",
	function() Request("Stay") end)

local REACT_BUTTONS = {
	{ button = buttons[8], mode = 2, icon = PET_AGGRESSIVE_TEXTURE or "Interface\\Icons\\Ability_Racial_BloodRage",
		title = PET_MODE_AGGRESSIVE or "Aggressive", text = "Your second pet attacks every enemy near it." },
	{ button = buttons[9], mode = 1, icon = PET_DEFENSIVE_TEXTURE or "Interface\\Icons\\Ability_Defend",
		title = PET_MODE_DEFENSIVE or "Defensive", text = "Your second pet attacks what attacks you or it." },
	{ button = buttons[10], mode = 0, icon = PET_PASSIVE_TEXTURE or "Interface\\Icons\\Ability_Seal",
		title = PET_MODE_PASSIVE or "Passive", text = "Your second pet attacks only on your command." },
}

for _, react in ipairs(REACT_BUTTONS) do
	local mode = react.mode
	SetCommandButton(react.button, react.icon, react.title, react.text,
		function() Request("React", mode) end)
end

-- Spell buttons 4-7
local spellButtons = {}
for slot = 1, SPELL_SLOTS do
	local button = buttons[3 + slot]
	spellButtons[slot] = button
	button:SetScript("OnClick", function(self, mouseButton)
		if not self.spellId then
			return
		end
		if mouseButton == "RightButton" then
			if self.isAutocastable then
				Request("Autocast", self.spellId, self.autocastOn and 0 or 1)
			end
		else
			Request("Cast", self.spellId)
		end
	end)
	button:SetScript("OnEnter", function(self)
		if not self.spellId then
			return
		end
		GameTooltip:SetOwner(self, "ANCHOR_TOP")
		GameTooltip:SetHyperlink("spell:" .. self.spellId)
		if self.isAutocastable then
			GameTooltip:AddLine("Right-click: autocast " .. (self.autocastOn and "off" or "on"),
				0.5, 0.5, 1)
		end
		GameTooltip:Show()
	end)
end

-- ============================================================
-- State
-- ============================================================

local state = {
	alive = true,
	react = 1,
	command = 1,
	attacking = false,
}

local function SetActive(button, active)
	if active then
		button.active:Show()
	else
		button.active:Hide()
	end
end

local function RefreshHighlights()
	SetActive(attackButton, state.attacking)
	SetActive(followButton, state.command == 1 and not state.attacking)
	SetActive(stayButton, state.command == 0)
	for _, react in ipairs(REACT_BUTTONS) do
		SetActive(react.button, state.react == react.mode)
	end
	for _, button in ipairs(buttons) do
		button.icon:SetDesaturated(not state.alive)
	end
end

local function SetSpell(button, spellId, autocastable, autocastOn)
	button.spellId = spellId
	-- the flag, not button.autocastable (the corner-bracket texture)
	button.isAutocastable = autocastable
	button.autocastOn = autocastOn
	if not spellId then
		button.icon:SetTexture(nil)
		button.autocastable:Hide()
		button.autocast:Hide()
		button.cooldown:Hide()
		button:EnableMouse(false)
		return
	end
	local _, _, icon = GetSpellInfo(spellId)
	button.icon:SetTexture(icon or "Interface\\Icons\\INV_Misc_QuestionMark")
	button:EnableMouse(true)
	if autocastable then
		button.autocastable:Show()
	else
		button.autocastable:Hide()
	end
	if autocastOn then
		button.autocast:Show()
	else
		button.autocast:Hide()
	end
end

local function Split(text, separator)
	local parts = {}
	for part in string.gmatch(text .. separator, "(.-)" .. separator) do
		parts[#parts + 1] = part
	end
	return parts
end

local function OnFull(parts)
	nameText:SetText(parts[2] or "")
	state.powerType = tonumber(parts[4]) or 0
	state.react = tonumber(parts[5]) or 1
	state.command = tonumber(parts[6]) or 1

	local color = POWER_COLORS[state.powerType] or POWER_COLORS[0]
	powerBar:SetStatusBarColor(color[1], color[2], color[3])

	local slot = 0
	for entry in string.gmatch(parts[7] or "", "[^,]+") do
		local id, castable, auto = string.match(entry, "^(%d+):(%d):(%d)$")
		if id and slot < SPELL_SLOTS then
			slot = slot + 1
			SetSpell(spellButtons[slot], tonumber(id), castable == "1", auto == "1")
		end
	end
	for rest = slot + 1, SPELL_SLOTS do
		SetSpell(spellButtons[rest], nil)
	end

	RefreshHighlights()
	frame:Show()
end

local function SetBar(bar, value, maximum)
	if not value or not maximum or maximum <= 0 then
		bar:SetMinMaxValues(0, 1)
		bar:SetValue(0)
		bar.text:SetText("")
		return
	end
	bar:SetMinMaxValues(0, maximum)
	bar:SetValue(value)
	bar.text:SetText(value .. " / " .. maximum)
end

local function OnStatus(parts)
	state.alive = parts[2] == "1"
	state.attacking = parts[7] == "1"
	SetBar(healthBar, tonumber(parts[3]), tonumber(parts[4]))
	SetBar(powerBar, tonumber(parts[5]), tonumber(parts[6]))
	RefreshHighlights()
end

local function OnCooldown(parts)
	local spellId = tonumber(parts[2])
	local left = tonumber(parts[3]) or 0
	for _, button in ipairs(spellButtons) do
		if button.spellId and button.spellId == spellId then
			if left > 0 then
				button.cooldown:SetCooldown(GetTime(), left / 1000)
				button.cooldown:Show()
			else
				button.cooldown:Hide()
			end
		end
	end
end

-- Only the server's whispers to this player carry the bar's state; another
-- player can whisper the same prefix
local listener = CreateFrame("Frame")
listener:RegisterEvent("CHAT_MSG_ADDON")
listener:SetScript("OnEvent", function(self, event, prefix, message, channel, sender)
	if prefix ~= STATE_PREFIX or channel ~= "WHISPER" or sender ~= UnitName("player") then
		return
	end
	local parts = Split(message, ";")
	local kind = parts[1]
	if kind == "F" then
		OnFull(parts)
	elseif kind == "S" then
		OnStatus(parts)
	elseif kind == "C" then
		OnCooldown(parts)
	elseif kind == "X" then
		frame:Hide()
	end
end)

-- The whole state once the bar is loaded (login, /reload, /aio reset)
Request("Sync")
