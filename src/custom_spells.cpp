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

#include "custom_spells_common.h"

/*
 * ==========================================================================
 *  mod-custom-spells - Custom Spell Effects Module
 * ==========================================================================
 *
 *  This module uses SpellScripts to override spell effect behavior.
 *  Each class has its own source file:
 *    custom_spells_warrior.cpp  - Warrior (Arms/Fury/Prot)
 *    custom_spells_paladin.cpp  - Paladin (Holy/Prot/Ret)
 *    custom_spells_dk.cpp       - Death Knight (Blood/Frost/Unholy)
 *    custom_spells_shaman.cpp   - Shaman (Ele/Enhance/Resto)
 *    custom_spells_hunter.cpp   - Hunter (BM/MM/Surv)
 *    custom_spells_rogue.cpp    - Rogue (Assa/Combat/Sub)
 *    custom_spells_druid.cpp    - Druid (Balance/Feral/Resto)
 *    custom_spells_mage.cpp     - Mage (Arcane/Fire/Frost)
 *
 *  Shared constants and spell IDs are in custom_spells_common.h
 */

void AddCustomSpellsScripts()
{
    AddWarriorSpellsScripts();
    AddPaladinSpellsScripts();
    AddDKSpellsScripts();
    AddShamanSpellsScripts();
    AddHunterSpellsScripts();
    AddRogueSpellsScripts();
    AddDruidSpellsScripts();
    AddMageSpellsScripts();
}
