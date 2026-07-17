-- =============================================================
-- mod-custom-spells — Client-side AIO UI
--
-- Spell picker: lists the player's custom class spells grouped
-- by spec; each row is a checkbox that learns/forgets the spell
-- (validated server-side). Open with /spells or /cs.
-- =============================================================

local AIO = AIO or require("AIO")
if AIO.AddAddon() then return end

if not CustomSpells_ClientHandlers then
	CustomSpells_ClientHandlers = {}
end
local CustomSpellsHandlers = CustomSpells_ClientHandlers

if not CustomSpells_ClientHandlersRegistered then
	AIO.AddHandlers("CustomSpells", CustomSpellsHandlers)
	CustomSpells_ClientHandlersRegistered = true
end

-- ============================================================
-- Main frame
-- ============================================================

local ROW_HEIGHT = 22
local HEADER_HEIGHT = 24

local mainFrame = CreateFrame("Frame", "CustomSpellsFrame", UIParent)
mainFrame:SetSize(380, 480)
mainFrame:SetPoint("CENTER")
mainFrame:SetBackdrop({
	bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
	edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
	tile = true, tileSize = 32, edgeSize = 32,
	insets = { left = 8, right = 8, top = 8, bottom = 8 },
})
mainFrame:SetMovable(true)
mainFrame:EnableMouse(true)
mainFrame:RegisterForDrag("LeftButton")
mainFrame:SetScript("OnDragStart", mainFrame.StartMoving)
mainFrame:SetScript("OnDragStop", mainFrame.StopMovingOrSizing)
mainFrame:SetFrameStrata("DIALOG")
mainFrame:Hide()

tinsert(UISpecialFrames, "CustomSpellsFrame")

local title = mainFrame:CreateFontString(nil, "OVERLAY", "GameFontNormalLarge")
title:SetPoint("TOP", 0, -14)
title:SetText("Forgotten Spells")

local closeBtn = CreateFrame("Button", nil, mainFrame, "UIPanelCloseButton")
closeBtn:SetPoint("TOPRIGHT", -6, -6)

-- ============================================================
-- Scrollable list
-- ============================================================

local scrollFrame = CreateFrame("ScrollFrame", "CustomSpellsScroll", mainFrame,
	"UIPanelScrollFrameTemplate")
scrollFrame:SetPoint("TOPLEFT", 16, -40)
scrollFrame:SetPoint("BOTTOMRIGHT", -36, 48)

local content = CreateFrame("Frame", nil, scrollFrame)
content:SetSize(310, 10)
scrollFrame:SetScrollChild(content)

local headerPool = {}
local rowPool = {}

local function GetHeader(index)
	local header = headerPool[index]
	if not header then
		header = content:CreateFontString(nil, "OVERLAY", "GameFontNormal")
		headerPool[index] = header
	end
	header:Show()
	return header
end

local function GetRow(index)
	local row = rowPool[index]
	if not row then
		row = CreateFrame("CheckButton", nil, content, "UICheckButtonTemplate")
		row:SetSize(24, 24)
		row.label = content:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
		row.label:SetPoint("LEFT", row, "RIGHT", 4, 0)
		row:SetScript("OnClick", function(self)
			-- Server decides the real state; the reply repaints the list
			AIO.Handle("CustomSpells", "Toggle", self.spellId)
		end)
		rowPool[index] = row
	end
	row:Show()
	row.label:Show()
	return row
end

local function Repaint(rows)
	for _, header in pairs(headerPool) do
		header:Hide()
	end
	for _, row in pairs(rowPool) do
		row:Hide()
		row.label:Hide()
	end

	local y = 0
	local headerIndex = 0
	local rowIndex = 0
	local lastSpec = nil

	for _, entry in ipairs(rows) do
		if entry.spec ~= lastSpec then
			lastSpec = entry.spec
			headerIndex = headerIndex + 1
			local header = GetHeader(headerIndex)
			header:ClearAllPoints()
			header:SetPoint("TOPLEFT", 4, -y - 6)
			header:SetText("|cffffd100" .. entry.spec .. "|r")
			y = y + HEADER_HEIGHT
		end

		rowIndex = rowIndex + 1
		local row = GetRow(rowIndex)
		row.spellId = entry.id
		row:ClearAllPoints()
		row:SetPoint("TOPLEFT", 10, -y)
		row:SetChecked(entry.learned == 1)
		row.label:SetText(entry.name)
		y = y + ROW_HEIGHT
	end

	content:SetHeight(y + 10)
end

-- ============================================================
-- Bottom buttons
-- ============================================================

local learnAllBtn = CreateFrame("Button", nil, mainFrame, "UIPanelButtonTemplate")
learnAllBtn:SetSize(120, 24)
learnAllBtn:SetPoint("BOTTOMLEFT", 16, 16)
learnAllBtn:SetText("Learn All")
learnAllBtn:SetScript("OnClick", function()
	AIO.Handle("CustomSpells", "SetAll", 1)
end)

local forgetAllBtn = CreateFrame("Button", nil, mainFrame, "UIPanelButtonTemplate")
forgetAllBtn:SetSize(120, 24)
forgetAllBtn:SetPoint("BOTTOMRIGHT", -30, 16)
forgetAllBtn:SetText("Forget All")
forgetAllBtn:SetScript("OnClick", function()
	AIO.Handle("CustomSpells", "SetAll", 0)
end)

-- ============================================================
-- Server -> client
-- ============================================================

function CustomSpellsHandlers.State(player, rows)
	if type(rows) ~= "table" then
		return
	end
	Repaint(rows)
	mainFrame:Show()
end

-- ============================================================
-- Slash commands
-- ============================================================

SLASH_CUSTOMSPELLS1 = "/spells"
SLASH_CUSTOMSPELLS2 = "/cs"
SlashCmdList["CUSTOMSPELLS"] = function()
	if mainFrame:IsShown() then
		mainFrame:Hide()
	else
		AIO.Handle("CustomSpells", "Show")
	end
end
