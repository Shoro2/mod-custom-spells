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
    // Custom damage spell: Base 666 + 66% AP + 1% per Paragon level
    SPELL_CUSTOM_PARAGON_STRIKE = 900106,
};

// Paragon level aura (stack count = paragon level)
constexpr uint32 AURA_PARAGON_LEVEL   = 100000;

// Damage constants
constexpr int32  CUSTOM_BASE_DAMAGE   = 666;
constexpr float  CUSTOM_AP_COEFF      = 0.66f;   // 66% of Attack Power
constexpr float  CUSTOM_PARAGON_BONUS = 0.01f;    // +1% damage per Paragon level

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
            //  SPELL 900106: Paragon Strike
            //  - Base damage: 666
            //  - Bonus: 66% Attack Power
            //  - +1% total damage per Paragon level (aura 100000 stacks)
            // ============================================================
            case SPELL_CUSTOM_PARAGON_STRIKE:
            {
                Unit* target = spell->m_targets.GetUnitTarget();
                if (!target || !target->IsAlive())
                {
                    LOG_DEBUG("module", "mod-custom-spells: Player {} "
                        "has no valid target for Paragon Strike.",
                        player->GetName());
                    break;
                }

                // 66% of melee attack power
                float ap = player->GetTotalAttackPowerValue(BASE_ATTACK);
                float bonusDmg = ap * CUSTOM_AP_COEFF;

                // Paragon level from aura stacks on ID 100000
                uint32 paragonLevel = player->GetAuraCount(AURA_PARAGON_LEVEL);
                float paragonMult = 1.0f + (paragonLevel * CUSTOM_PARAGON_BONUS);

                // Final damage = (base + AP bonus) * paragon multiplier
                int32 totalDmg = static_cast<int32>(
                    (CUSTOM_BASE_DAMAGE + bonusDmg) * paragonMult);

                SpellNonMeleeDamage damageInfo(
                    player, target, spell->GetSpellInfo(),
                    spell->GetSpellInfo()->SchoolMask);
                damageInfo.damage = totalDmg;

                player->SendSpellNonMeleeDamageLog(&damageInfo);
                player->DealSpellDamage(&damageInfo, false);

                LOG_INFO("module", "mod-custom-spells: Player {} -> "
                    "Paragon Strike {} dmg on {} "
                    "(AP: {:.0f}, Paragon Lv: {}, Mult: {:.2f})",
                    player->GetName(), totalDmg, target->GetName(),
                    ap, paragonLevel, paragonMult);
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
