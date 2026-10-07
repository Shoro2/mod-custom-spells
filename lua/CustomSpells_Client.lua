-- =============================================================
-- mod-custom-spells — Client-side AIO UI
--
-- Spell picker: lists the player's custom class spells grouped
-- by spec; each row is a checkbox that learns/forgets the spell
-- (validated server-side). Open with /spells or /cs. A GM also
-- gets "Learn all Talents" for the Forgotten Talents tree.
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
local ROW_HIT_WIDTH = 296
local HEADER_HEIGHT = 24
-- Bottom edge of the list, above the Learn All / Forget All row. The GM
-- row lifts it by GM_ROW_HEIGHT while that row is shown.
local LIST_BOTTOM = 48
local GM_ROW_HEIGHT = 28

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
scrollFrame:SetPoint("BOTTOMRIGHT", -36, LIST_BOTTOM)

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
		-- Include the label in the existing checkbox's click and hover area.
		row:SetHitRectInsets(0, 24 - ROW_HIT_WIDTH, 0, 0)
		row.label = content:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
		row.label:SetPoint("LEFT", row, "RIGHT", 4, 0)
		row:SetScript("OnClick", function(self)
			-- Server decides the real state; the reply repaints the list
			AIO.Handle("CustomSpells", "Toggle", self.spellId)
		end)
		row:SetScript("OnEnter", function(self)
			GameTooltip:SetOwner(self, "ANCHOR_RIGHT")
			GameTooltip:SetHyperlink("spell:" .. self.spellId)
			GameTooltip:Show()
		end)
		local function HideTooltip(self)
			if GameTooltip:IsOwned(self) then
				GameTooltip:Hide()
			end
		end
		row:SetScript("OnLeave", HideTooltip)
		row:SetScript("OnHide", HideTooltip)
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

-- GM only: every Forgotten Talents node at its maximum rank, free of
-- charge (mod-forgotten-talents' .forgotten learnall). It has a row of its
-- own above Learn All / Forget All and is shown only while the server's
-- State says GM; the server checks the rank again on every click.
local learnTalentsBtn = CreateFrame("Button", nil, mainFrame,
	"UIPanelButtonTemplate")
learnTalentsBtn:SetSize(160, 24)
learnTalentsBtn:SetPoint("BOTTOM", 0, 16 + GM_ROW_HEIGHT)
learnTalentsBtn:SetText("Learn all Talents")
learnTalentsBtn:SetScript("OnClick", function()
	AIO.Handle("CustomSpells", "LearnAllTalents")
end)
learnTalentsBtn:SetScript("OnEnter", function(self)
	GameTooltip:SetOwner(self, "ANCHOR_TOP")
	GameTooltip:SetText("Learn all Talents", 1, 0.82, 0)
	GameTooltip:AddLine("GM only: every Forgotten Talents node at its "
		.. "maximum rank, free of charge.", 1, 1, 1, true)
	GameTooltip:Show()
end)
learnTalentsBtn:SetScript("OnLeave", function()
	GameTooltip:Hide()
end)
learnTalentsBtn:Hide()

local function ShowGameMasterRow(shown)
	if shown then
		scrollFrame:SetPoint("BOTTOMRIGHT", -36, LIST_BOTTOM + GM_ROW_HEIGHT)
		learnTalentsBtn:Show()
	else
		scrollFrame:SetPoint("BOTTOMRIGHT", -36, LIST_BOTTOM)
		learnTalentsBtn:Hide()
	end
end

-- ============================================================
-- Server -> client
-- ============================================================

-- gm is 1 for a GM account and 0 otherwise; it only decides whether the
-- Learn all Talents row is shown.
function CustomSpellsHandlers.State(player, rows, gm)
	if type(rows) ~= "table" then
		return
	end
	ShowGameMasterRow(gm == 1)
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
