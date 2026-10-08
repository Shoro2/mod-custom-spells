# data_structure.md - mod-custom-spells

```
mod-custom-spells/
├── src/
│   ├── mod_custom_spells_loader.cpp  # entry point: Addmod_custom_spellsScripts()
│   ├── custom_spells_common.h        # spell ids (enum CustomSpellIds), shared constants, helpers
│   ├── custom_spells.cpp             # master switch, CastAnchoredBurst + anchor exclusion, class loaders
│   ├── custom_spells_<class>.cpp     # warrior, paladin, dk, shaman, hunter, rogue, druid, mage, warlock, priest
│   └── custom_spells_global.cpp      # non-class spells, shared minion auras, mana regeneration
├── data/
│   ├── spellbook_enUS.json           # spellbook name/tooltip/icon per picker spell + helper buff tooltips
│   └── sql/db-world/                 # applied by the AC updater (hash-tracked, re-applied when changed)
│       ├── mod_custom_spells.sql     # spell_script_names (LIKE 'spell\_custom\_%' cleanup, then all rows)
│       ├── mod_custom_spells_b_warrior_paladin.sql
│       ├── mod_custom_spells_c_dk_shaman.sql
│       ├── mod_custom_spells_d_hunter_druid_rogue.sql
│       ├── mod_custom_spells_e_mage.sql
│       ├── mod_custom_spells_f_warlock_priest_global.sql
│       └── mod_custom_spells_z_fixups.sql   # EquippedItemClass -1 across the block, retired rows
├── lua/                              # AIO picker: CustomSpells_Server.lua (whitelist) + _Client.lua (UI)
├── tests/                            # mod-fl-testbots scenarios: cs_<class>.tbs, cs_concept_*.tbs
├── tools/                            # client_spell_sync.py (client Spell.dbc), README.md
└── conf/mod_custom_spells.conf.dist  # CustomSpells.Enable
```

Each class SQL file owns its `spell_dbc` rows (DELETE by explicit id list, then INSERT), its `spell_proc`
rows and its custom NPCs. Bindings stay in `mod_custom_spells.sql`: its name-scoped cleanup would delete a
binding another file inserted whenever it alone is re-applied.

The Lua pair deploys to `dcore/lua_scripts/CustomSpells/` (Eluna/ALE + AIO). Custom NPCs: 900333 Frost
Wyrm, 900436 Spirit Wolf, 901066 Healing Treant. ID blocks per class/spec: [`CustomSpells.md`](./CustomSpells.md)
and share-public `custom-spells/02-id-blocks.md`.
