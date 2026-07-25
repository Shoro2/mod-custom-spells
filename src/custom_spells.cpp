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
 *    custom_spells_warlock.cpp  - Warlock (Affli/Demo/Destro)
 *    custom_spells_priest.cpp   - Priest (Disc/Holy/Shadow)
 *    custom_spells_global.cpp   - Non-Class Global (all classes)
 *
 *  Shared constants and spell IDs are in custom_spells_common.h
 */

// ============================================================
//  Module master switch, cached instead of read per handler.
//  Declared in custom_spells_common.h; the note there explains why
//  sConfigMgr->GetOption must not be called from a spell handler.
// ============================================================
bool g_CustomSpellsEnabled = true;

class CustomSpells_World : public WorldScript
{
public:
    CustomSpells_World() : WorldScript("CustomSpells_World") { }

    // Also fires on `.reload config` (cs_reload.cpp -> World::LoadConfigSettings(true)
    // -> ScriptMgr::OnAfterConfigLoad), so the switch stays live-togglable exactly as
    // it was when every handler read sConfigMgr directly.
    void OnAfterConfigLoad(bool /*reload*/) override
    {
        g_CustomSpellsEnabled = sConfigMgr->GetOption<bool>("CustomSpells.Enable", true);
    }
};

// ============================================================
//  Shared helper: area bursts cast AT a hit target (Holy Shock
//  Burst 900208, DK Shadow Eruption 900367, Beast Cleave 900505,
//  Explosive Burst 900567, Poison Nova 900604) must not hit the
//  anchor target again - it already took the triggering spell.
// ============================================================
class spell_custom_exclude_anchor_target : public SpellScript
{
    PrepareSpellScript(spell_custom_exclude_anchor_target);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (Unit* anchor = GetExplTargetUnit())
            targets.remove(anchor);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(
            spell_custom_exclude_anchor_target::FilterTargets,
            EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

void AddCustomSpellsScripts()
{
    new CustomSpells_World();

    RegisterSpellScript(spell_custom_exclude_anchor_target);

    AddWarriorSpellsScripts();
    AddPaladinSpellsScripts();
    AddDKSpellsScripts();
    AddShamanSpellsScripts();
    AddHunterSpellsScripts();
    AddRogueSpellsScripts();
    AddDruidSpellsScripts();
    AddMageSpellsScripts();
    AddWarlockSpellsScripts();
    AddPriestSpellsScripts();
    AddGlobalSpellsScripts();
}
