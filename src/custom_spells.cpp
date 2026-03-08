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

#include "ScriptMgr.h"
#include "Player.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Config.h"
#include "Log.h"

/*
 * ==========================================================================
 *  mod-custom-spells - Custom Spell Effects Module
 * ==========================================================================
 *
 *  This module hooks into player spell casts and allows you to define
 *  custom behavior for specific spell IDs via C++ scripts.
 *
 *  HOW TO ADD A NEW CUSTOM SPELL:
 *  1. Define the spell ID as a constant in the enum below.
 *  2. Add a new case in OnPlayerSpellCast() for your spell ID.
 *  3. Implement your custom logic inside that case.
 *
 *  TEMPLATE EXAMPLE (copy & adapt):
 *  ---------------------------------------------------------------
 *      case SPELL_MY_CUSTOM_SPELL:
 *      {
 *          // Your custom logic here
 *          // e.g. player->CastSpell(target, SPELL_SOMETHING, true);
 *          break;
 *      }
 *  ---------------------------------------------------------------
 */

enum CustomSpellIds
{
    // Custom proc spell: Rage Burst + Execute
    SPELL_CUSTOM_RAGE_EXECUTE_PROC = 900106,

    // The Execute spell that gets triggered
    SPELL_CUSTOM_EXECUTE           = 97471,
};

enum CustomSpellValues
{
    RAGE_AMOUNT = 100,  // Rage to add (in display units, *10 internally)
};

class CustomSpellsPlayerScript : public PlayerScript
{
public:
    CustomSpellsPlayerScript() : PlayerScript("CustomSpellsPlayerScript") { }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!player || !spell || !spell->GetSpellInfo())
            return;

        if (!sConfigMgr->GetOption<bool>("CustomSpells.Enable", true))
            return;

        uint32 spellId = spell->GetSpellInfo()->Id;

        switch (spellId)
        {
            // ============================================================
            //  SPELL 900106: Rage Burst + Execute
            //  - Adds 100 rage to the caster
            //  - Casts Execute (97471) on the player's current target
            // ============================================================
            case SPELL_CUSTOM_RAGE_EXECUTE_PROC:
            {
                // Add 100 rage (rage is stored as *10 internally)
                player->EnergizeBySpell(
                    player,
                    SPELL_CUSTOM_RAGE_EXECUTE_PROC,
                    RAGE_AMOUNT * 10,
                    POWER_RAGE
                );

                LOG_INFO("module", "mod-custom-spells: Player {} cast spell {}. "
                    "Added {} rage.",
                    player->GetName(), spellId, RAGE_AMOUNT);

                // Cast Execute on the player's selected target
                Unit* target = player->GetSelectedUnit();
                if (target)
                {
                    player->CastSpell(target, SPELL_CUSTOM_EXECUTE, true);

                    LOG_INFO("module", "mod-custom-spells: Player {} -> "
                        "Execute (spell {}) on target {}.",
                        player->GetName(), SPELL_CUSTOM_EXECUTE,
                        target->GetName());
                }
                else
                {
                    LOG_DEBUG("module", "mod-custom-spells: Player {} "
                        "has no target for Execute.",
                        player->GetName());
                }
                break;
            }

            // ============================================================
            //  ADD MORE CUSTOM SPELLS HERE
            //  Copy the template below and adapt it:
            // ============================================================
            //
            // case SPELL_MY_NEW_SPELL:
            // {
            //     // Custom logic...
            //     break;
            // }

            default:
                break;
        }
    }
};

void AddCustomSpellsScripts()
{
    new CustomSpellsPlayerScript();
}
