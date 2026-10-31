-- =============================================================
-- mod-custom-spells - icon badges (client side, AIO addon, WoW 3.3.5a, Lua 5.1)
--
-- A custom class spell shows the icon of the stock spell it modifies (Crusader Strike for
-- "Crusader Strike: +9 Targets"), with a short text in the bottom-right corner ("+9", "+50%",
-- "ALL", "-2s" ...) so that one base icon can be reused and only icon + badge is unique.
-- The texts live in the server manifest (data/spellbook_enUS.json "badge"), reach the client
-- once per login as one block of AIO's initial message ("CustomSpellsBadges" / "Badges", added
-- by CustomSpells_Server.lua), and are kept in the global FL_SpellBadges[spellId] = "text".
--
-- Shared helper (every window may use it; each call must work when this file is missing, so
-- a caller checks `if FLSpellBadge then ... end` at call time):
--   FLSpellBadge.Get(spellId)               the badge text, or nil
--   FLSpellBadge.Set(frame, spellId, anchor) draw the badge of spellId in the bottom-right corner of
--                                            anchor (a frame or texture; default frame); a spell
--                                            without a badge, or a nil id, hides it again.
--                                            The badge lives in a child frame of `frame`.
--   FLSpellBadge.Text(spellId)              " |cffffd100[+9]|r" for an inline text line, or ""
--   FLSpellBadge.OnChange(fn)               fn() runs when a new badge table arrived
-- It also hooks the Blizzard spellbook, so learned custom passives carry their badge there.
-- Look: NumberFontNormalSmall's face with an outline, gold (the guide's accent colour) on a
-- faint black plate, 8-11 px by icon size (fl-ui-design / 15-ui-style-guide.md).
-- =============================================================

local AIO = AIO or require("AIO")
if AIO.AddAddon() then return end

-- AIO can run this file twice in one UI session; hook and handler are set up once.
if FLSpellBadge_Loaded then return end
FLSpellBadge_Loaded = true

FL_SpellBadges = FL_SpellBadges or {}
FLSpellBadge = FLSpellBadge or {}
local Badge = FLSpellBadge

local MAX_TEXT = 8                       -- the manifest allows 5; the client takes a little more
local MIN_SIZE, MAX_SIZE = 7, 11         -- font height in px
local FALLBACK_W = 32
local listeners = {}

local function FontPath()
	local path
	if NumberFontNormalSmall and NumberFontNormalSmall.GetFont then
		path = NumberFontNormalSmall:GetFont()
	end
	return path or "Fonts\\ARIALN.TTF"
end

function Badge.Get(spellId)
	spellId = tonumber(spellId)
	if not spellId then return nil end
	local t = FL_SpellBadges and FL_SpellBadges[spellId]
	if type(t) == "string" and t ~= "" then return t end
	return nil
end

function Badge.Text(spellId)
	local t = Badge.Get(spellId)
	if t then return " |cffffd100[" .. t .. "]|r" end
	return ""
end

function Badge.OnChange(fn)
	if type(fn) == "function" then listeners[#listeners + 1] = fn end
end

local function Build(frame)
	local h = CreateFrame("Frame", nil, frame)
	h:SetFrameLevel(frame:GetFrameLevel() + 6)      -- above a cooldown sweep or the button's own art
	h.back = h:CreateTexture(nil, "ARTWORK")
	h.back:SetTexture(0, 0, 0, 0.6)
	h.text = h:CreateFontString(nil, "OVERLAY")
	h.text:SetFont(FontPath(), 10, "OUTLINE")
	h.text:SetTextColor(1, 0.82, 0)
	h.back:SetPoint("TOPLEFT", h.text, "TOPLEFT", -2, 1)
	h.back:SetPoint("BOTTOMRIGHT", h.text, "BOTTOMRIGHT", 2, -1)
	frame.flBadge = h
	return h
end

function Badge.Set(frame, spellId, anchor)
	if not frame then return end
	local text = Badge.Get(spellId)
	local h = frame.flBadge
	if not text then
		if h then h:Hide() end
		return
	end
	h = h or Build(frame)
	anchor = anchor or frame
	h:ClearAllPoints()
	h:SetPoint("TOPLEFT", anchor, "TOPLEFT", 0, 0)
	h:SetPoint("BOTTOMRIGHT", anchor, "BOTTOMRIGHT", 0, 0)

	local w = anchor:GetWidth()
	if not w or w < 4 then w = frame:GetWidth() end
	if not w or w < 4 then w = FALLBACK_W end
	local size = math.floor(w * 0.3 + 0.5)
	if size > MAX_SIZE then size = MAX_SIZE end
	if size < 8 then size = 8 end

	local path = FontPath()
	h.text:ClearAllPoints()
	h.text:SetPoint("BOTTOMRIGHT", h, "BOTTOMRIGHT", -2, 2)
	h.text:SetFont(path, size, "OUTLINE")
	h.text:SetText(text)
	-- a long text shrinks until it fits the icon
	while size > MIN_SIZE and h.text:GetStringWidth() > w - 2 do
		size = size - 1
		h.text:SetFont(path, size, "OUTLINE")
		h.text:SetText(text)
	end
	h:Show()
end

-- ============================================================
-- Server -> client: the badge table
-- ============================================================

local Handlers = {}

function Handlers.Badges(player, badges)
	if type(badges) ~= "table" then return end
	-- fill the global in place, so a reference held elsewhere stays valid
	for k in pairs(FL_SpellBadges) do FL_SpellBadges[k] = nil end
	for id, text in pairs(badges) do
		id = tonumber(id)
		if id and type(text) == "string" and text ~= "" and #text <= MAX_TEXT then
			FL_SpellBadges[id] = text
		end
	end
	for _, fn in ipairs(listeners) do pcall(fn) end
end

AIO.AddHandlers("CustomSpellsBadges", Handlers)

-- ============================================================
-- The Blizzard spellbook: badge on a learned custom spell's button
-- (FrameXML SpellBookFrame.lua: SpellButton_UpdateButton paints every
-- button; SpellBook_GetSpellID(button:GetID()) is its spell slot)
-- ============================================================

local function PaintSpellButton(button)
	local name = button:GetName()
	local icon = name and _G[name .. "IconTexture"]
	if not icon or not icon:IsShown() or not SpellBook_GetSpellID or not SpellBookFrame then
		Badge.Set(button, nil)
		return
	end
	local slot = SpellBook_GetSpellID(button:GetID())
	local link = slot and GetSpellLink(slot, SpellBookFrame.bookType)
	local spellId = link and tonumber(string.match(link, "spell:(%d+)"))
	Badge.Set(button, spellId, icon)
end

local function RepaintSpellbook()
	if not SPELLS_PER_PAGE then return end
	for i = 1, SPELLS_PER_PAGE do
		local button = _G["SpellButton" .. i]
		if button then pcall(PaintSpellButton, button) end
	end
end

if SpellButton_UpdateButton then
	hooksecurefunc("SpellButton_UpdateButton", function(self)
		pcall(PaintSpellButton, self)
	end)
end
Badge.OnChange(RepaintSpellbook)
