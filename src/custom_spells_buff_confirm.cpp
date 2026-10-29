/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// ============================================================
//  A weaker recast of the player's own buff asks first
// ============================================================
// The core calls PlayerScript::OnPlayerCanReplaceStrongerOwnBuff when a player's cast of his own buff of an
// exclusive-highest spell group (Thorns, the druid marks, the paladin blessings, ...) would come out weaker than the one
// he already has on that target (Spell::CheckCast). Here that cast is refused and the player is asked in a popup
// (lua/CustomSpells_BuffConfirm_Client.lua), by an addon whisper from the player to himself, prefix CSBC:
//    Q;spell id;spell name;target name;the active buff's amount;this cast's amount (as the core compared them: the
//    same spell with every bonus, another rank by its base value)
// The popup's Yes runs `.csbuff yes`: the same spell is cast again on the same target as a normal cast (power, cooldown,
// range and the rest checked again), allowed this time, and the core replaces the old aura. `.csbuff no` or 30 s
// without an answer drop the question. A cast with an item (a scroll, a quest item) is never asked about - the recast
// could not use the item again - so it replaces as before.
//    CustomSpells.BuffReplaceConfirm.Enable = 0: never ask, always replace
#include "custom_spells_common.h"
#include "Chat.h"
#include "CommandScript.h"
#include "Timer.h"
#include "WorldPacket.h"
#include "WorldSession.h"

using namespace Acore::ChatCommands;

namespace
{
    constexpr char const* BUFF_CONFIRM_PREFIX = "CSBC";
    constexpr char const* BUFF_CONFIRM_KEY = "CustomSpellsBuffConfirm";
    constexpr uint32 BUFF_CONFIRM_TIMEOUT_MS = 30 * IN_MILLISECONDS;

    class BuffConfirmState : public DataMap::Base
    {
    public:
        uint32 SpellId = 0;         // the refused cast, asked about
        ObjectGuid Target;
        uint32 AskedAtMs = 0;
        bool Confirmed = false;     // `.csbuff yes`: the next cast of SpellId on Target goes through

        void Clear()
        {
            SpellId = 0;
            Target.Clear();
            AskedAtMs = 0;
            Confirmed = false;
        }

        bool Open() const
        {
            return SpellId && getMSTimeDiff(AskedAtMs, getMSTime()) <= BUFF_CONFIRM_TIMEOUT_MS;
        }
    };

    // the popup text's fields: no field separator inside a name
    std::string BuffConfirmSafe(std::string text)
    {
        for (char& c : text)
            if (c == ';' || c == '\t' || c == '\n')
                c = ' ';
        return text;
    }

    void SendBuffConfirmAsk(Player* player, std::string const& text)
    {
        WorldSession* session = player->GetSession();
        if (!session)
            return;

        WorldPacket data;
        ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, LANG_ADDON, player->GetGUID(),
            player->GetGUID(), std::string(BUFF_CONFIRM_PREFIX) + '\t' + text, 0);
        session->SendPacket(&data);
    }
}

class custom_buff_confirm_playerscript : public PlayerScript
{
public:
    custom_buff_confirm_playerscript() : PlayerScript("custom_buff_confirm_playerscript",
        { PLAYERHOOK_ON_CAN_REPLACE_STRONGER_OWN_BUFF }) { }

    bool OnPlayerCanReplaceStrongerOwnBuff(Player* player, Spell* spell, Unit* target, AuraEffect const* existing,
        int32 activeAmount, int32 newAmount) override
    {
        if (!sConfigMgr->GetOption<bool>("CustomSpells.BuffReplaceConfirm.Enable", true) || spell->m_CastItem)
            return true;

        uint32 const spellId = spell->GetSpellInfo()->Id;
        BuffConfirmState* state = player->CustomData.GetDefault<BuffConfirmState>(BUFF_CONFIRM_KEY);
        if (state->Confirmed && state->SpellId == spellId && state->Target == target->GetGUID() && state->Open())
        {
            state->Clear();
            return true;
        }

        state->SpellId = spellId;
        state->Target = target->GetGUID();
        state->AskedAtMs = getMSTime();
        state->Confirmed = false;

        SpellInfo const* activeInfo = existing->GetSpellInfo();
        std::string const name = activeInfo->SpellName[player->GetSession()->GetSessionDbcLocale()];
        SendBuffConfirmAsk(player, Acore::StringFormat("Q;{};{};{};{};{}", spellId, BuffConfirmSafe(name),
            BuffConfirmSafe(target == player ? std::string("you") : target->GetName()),
            std::abs(activeAmount), std::abs(newAmount)));
        return false;
    }
};

// ============================================================
//  .csbuff yes|no   the popup's answer
// ============================================================
class custom_buff_confirm_commandscript : public CommandScript
{
public:
    custom_buff_confirm_commandscript() : CommandScript("custom_buff_confirm_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable buffConfirmTable =
        {
            { "yes", HandleYes, SEC_PLAYER, Console::No },
            { "no",  HandleNo,  SEC_PLAYER, Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "csbuff", buffConfirmTable },
        };

        return commandTable;
    }

    static bool HandleYes(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        BuffConfirmState* state = player ? player->CustomData.Get<BuffConfirmState>(BUFF_CONFIRM_KEY) : nullptr;
        if (!state || !state->Open())
        {
            if (state)
                state->Clear();
            handler->SendSysMessage("There is no buff to replace.");
            return true;
        }

        Unit* target = ObjectAccessor::GetUnit(*player, state->Target);
        if (!target)
        {
            state->Clear();
            handler->SendSysMessage("The target of that buff is gone.");
            return true;
        }

        uint32 const spellId = state->SpellId;
        state->Confirmed = true;
        player->CastSpell(target, spellId, false);
        // let through, the hook cleared the question; refused for another reason (out of range, no mana), it ends here
        state->Clear();
        return true;
    }

    static bool HandleNo(ChatHandler* handler)
    {
        if (Player* player = handler->GetPlayer())
            if (BuffConfirmState* state = player->CustomData.Get<BuffConfirmState>(BUFF_CONFIRM_KEY))
                state->Clear();
        return true;
    }
};

void AddBuffConfirmScripts()
{
    new custom_buff_confirm_playerscript();
    new custom_buff_confirm_commandscript();
}
