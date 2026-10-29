-- mod-custom-spells - a weaker recast of your own buff asks first: the popup, client side (AIO)
--
-- The server (src/custom_spells_buff_confirm.cpp) refuses a cast of the player's own buff that would come out weaker
-- than the one already on the target, and asks here by an addon whisper from the player to himself, prefix CSBC:
--   Q;spell id;spell name;target name;the active buff's amount;this cast's amount
-- Yes runs `.csbuff yes` (the server casts the spell again, allowed this time), No runs `.csbuff no`; the server drops
-- an unanswered question after 30 s.

local AIO = AIO or require("AIO")
if AIO.AddAddon() then
	return
end

local PREFIX = "CSBC"

StaticPopupDialogs["CUSTOMSPELLS_BUFF_REPLACE"] = {
	text = "%s",
	button1 = YES,
	button2 = NO,
	OnAccept = function()
		SendChatMessage(".csbuff yes", "SAY")
	end,
	OnCancel = function(self, data, reason)
		if reason == "clicked" then
			SendChatMessage(".csbuff no", "SAY")
		end
	end,
	timeout = 30,
	whileDead = false,
	hideOnEscape = true,
}

local listener = CreateFrame("Frame")
listener:RegisterEvent("CHAT_MSG_ADDON")
listener:SetScript("OnEvent", function(self, event, prefix, message, channel, sender)
	if prefix ~= PREFIX or channel ~= "WHISPER" or sender ~= UnitName("player") then
		return
	end
	local kind, _, spellName, target, active, new = strsplit(";", message)
	if kind ~= "Q" or not spellName then
		return
	end
	local text = string.format("Your active %s on %s is stronger (%s) than this cast would be (%s).\n\nReplace it anyway?",
		spellName, target or "?", active or "?", new or "?")
	StaticPopup_Show("CUSTOMSPELLS_BUFF_REPLACE", text)
end)
