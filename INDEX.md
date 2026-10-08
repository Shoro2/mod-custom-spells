# INDEX - mod-custom-spells

Entry point for AI tools.

| File | Purpose |
|---|---|
| `INDEX.md` | this file - navigation |
| `CLAUDE.md` | what the module is, its delivery chain (picker, spellbook, cursed items), cross-refs |
| `functions.md` | how it works: picker, spellbook texts, cursed pool, bot scenarios, editor conventions, pitfalls, APIs |
| `data_structure.md` | folder and file map, which SQL file owns what |
| `log.md` | change log, newest first |
| `todo.md` | open tasks |
| `CustomSpells.md` | ID catalog per class and spec |
| `PROCFLAGS_REFERENCE.md` | verified `ProcFlags` reference |
| `tools/README.md` | `client_spell_sync.py` (client `Spell.dbc` names, tooltips, icons) |

Cross-repo: share-public `docs/World of Warcraft/custom-spells/` (overview, ID blocks, procs, recipes,
the dated reviews `06`-`08`), `claude_log.md`, the global queue `12-server-todo.md`; the cursed-item pool
lives in mod-paragon-itemgen (`tools/gen_class_passives.py`); the bot harness is mod-fl-testbots.
