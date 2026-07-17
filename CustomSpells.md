# Custom Spell Master Plan (All Classes)

> **Status legend**: `planned` → `implemented` → `tested`
>
> This file is the **ID catalog** for all custom spells in this repo. The detailed per-spec specifications (effects, status, source links, implementation notes for every individual spell ID) live in [`share-public/docs/custom-spells/specs/`](https://github.com/Shoro2/share-public/tree/main/docs/custom-spells/specs). All per-row data — `# / Spell ID / Effect / Approach / Status / Details` — is mirrored 1:1 into the linked spec file.
>
> Cross-cutting topics:
> [`00-overview`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/00-overview.md) ·
> [`01-architecture`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/01-architecture.md) ·
> [`02-id-blocks`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/02-id-blocks.md) ·
> [`03-procs-and-flags`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/03-procs-and-flags.md) ·
> [`04-adding-a-spell`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/04-adding-a-spell.md) ·
> [`05-complex-spells`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/05-complex-spells.md).

## ID block schema

> **Rule**: 100 IDs per class, 33 slots per spec (Spec1: +0..+32, Spec2: +33..+65, Spec3: +66..+99).

| Class | Block | Spec 1 | Spec 2 | Spec 3 |
|--------|-------|--------|--------|--------|
| **Warrior** | 900100-900199 | Arms: 900100-900132 | Fury: 900133-900165 | Prot: 900166-900199 |
| **Paladin** | 900200-900299 | Holy: 900200-900232 | Prot: 900233-900265 | Ret: 900266-900299 |
| **DK** | 900300-900399 | Blood: 900300-900332 | Frost: 900333-900365 | Unholy: 900366-900399 |
| **Shaman** | 900400-900499 | Ele: 900400-900432 | Enhance: 900433-900465 | Resto: 900466-900499 |
| **Hunter** | 900500-900599 | BM: 900500-900532 | MM: 900533-900565 | Surv: 900566-900599 |
| **Rogue** | 900600-900699 | Assa: 900600-900632 | Combat: 900633-900665 | Sub: 900666-900699 |
| **Mage** | 900700-900799 | Arcane: 900700-900732 | Fire: 900733-900765 | Frost: 900766-900799 |
| **Warlock** | 900800-900899 | Affli: 900800-900832 | Demo: 900833-900865 | Destro: 900866-900899 |
| **Priest** | 900900-900999 | Disc: 900900-900932 | Holy: 900933-900965 | Shadow: 900966-900999 |
| **Druid** | 901000-901099 | Balance: 901000-901032 | Feral: 901033-901065 | Resto: 901066-901099 |
| **Non-Class** | 901100-901199 | Global: 901100-901199 | — | — |

## Current allocation & per-spec links

> Status reflects the spec-level summary. Individual rows in the linked spec files carry their own per-spell status field.

| Class | Spec | Used | Free | Status | Spec doc |
|--------|------|--------|------|--------|----------|
| Warrior | Arms | 900100-900107 (8) | 900108-900132 (25) | tested | [warrior-arms](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warrior-arms.md) |
| Warrior | Fury | 900108-900121 (14) | 900122-900165 (44) | tested | [warrior-fury](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warrior-fury.md) |
| Warrior | Prot | 900168-900175 (8) | 900176-900199 (24) | implemented | [warrior-protection](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warrior-protection.md) |
| Paladin | Holy | 900200-900211 (12) | 900212-900232 (21) | implemented | [paladin-holy](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/paladin-holy.md) |
| Paladin | Prot | 900234-900241 (8) | 900242-900265 (24) | implemented | [paladin-protection](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/paladin-protection.md) |
| Paladin | Ret | 900268-900275 (8) | 900276-900299 (24) | implemented | [paladin-retribution](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/paladin-retribution.md) |
| DK | Blood | 900300-900304 (5) | 900305-900332 (28) | implemented | [death-knight-blood](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/death-knight-blood.md) |
| DK | Frost | 900333, 900368 (2) | 900334-900365 (32) | implemented | [death-knight-frost](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/death-knight-frost.md) |
| DK | Unholy | 900366-900367 (2) | 900369-900399 (32) | implemented | [death-knight-unholy](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/death-knight-unholy.md) |
| Shaman | Ele | 900400-900408 (9) | 900409-900432 (24) | implemented | [shaman-elemental](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/shaman-elemental.md) |
| Shaman | Enh | 900433-900440 (8) | 900441-900465 (25) | implemented | [shaman-enhancement](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/shaman-enhancement.md) |
| Shaman | Resto | 900466-900467 (2) | 900468-900499 (32) | implemented | [shaman-restoration](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/shaman-restoration.md) |
| Hunter | Shared | 900500-900501 (2) | — | implemented | [hunter-shared](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/hunter-shared.md) |
| Hunter | BM | 900502-900505 (4) | 900506-900532 (27) | implemented | [hunter-beast-mastery](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/hunter-beast-mastery.md) |
| Hunter | MM | 900533-900536 (4) | 900537-900565 (29) | implemented | [hunter-marksmanship](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/hunter-marksmanship.md) |
| Hunter | Surv | 900566-900567 (2) | 900568-900599 (32) | implemented | [hunter-survival](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/hunter-survival.md) |
| Rogue | Assa | 900600-900604 (5) | 900605-900632 (28) | implemented | [rogue-assassination](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/rogue-assassination.md) |
| Rogue | Combat | 900633-900638 (6) | 900639-900665 (27) | implemented | [rogue-combat](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/rogue-combat.md) |
| Rogue | Sub | 900666-900669 (4) | 900670-900699 (30) | implemented | [rogue-subtlety](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/rogue-subtlety.md) |
| Mage | Shared | 900700 (1) | — | implemented | [mage-shared](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/mage-shared.md) |
| Mage | Arcane | 900701-900713 (13) | 900714-900732 (19) | implemented | [mage-arcane](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/mage-arcane.md) |
| Mage | Fire | 900733-900740 (8) | 900741-900765 (25) | implemented | [mage-fire](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/mage-fire.md) |
| Mage | Frost | 900766-900774 (9) | 900775-900799 (25) | implemented | [mage-frost](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/mage-frost.md) |
| Warlock | Affli | 900800-900803 (4) | 900804-900832 (29) | implemented | [warlock-affliction](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warlock-affliction.md) |
| Warlock | Demo | 900833-900844 (12) | 900845-900865 (21) | implemented | [warlock-demonology](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warlock-demonology.md) |
| Warlock | Destro | 900866-900872 (7) | 900873-900899 (27) | implemented | [warlock-destruction](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warlock-destruction.md) |
| Priest | Disc | 900900-900903 (4) | 900904-900932 (29) | implemented | [priest-discipline](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/priest-discipline.md) |
| Priest | Holy | 900933 (1) | 900934-900965 (32) | implemented | [priest-holy](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/priest-holy.md) |
| Priest | Shadow | 900966-900968 (3) | 900969-900999 (31) | implemented | [priest-shadow](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/priest-shadow.md) |
| Druid | Balance | 901000-901005 (6) | 901006-901032 (27) | implemented | [druid-balance](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/druid-balance.md) |
| Druid | Feral Tank | 901033-901034 (2) | 901035-901048 (14) | implemented | [druid-feral-tank](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/druid-feral-tank.md) |
| Druid | Feral DPS | 901049-901051 (3) | 901052-901065 (14) | implemented | [druid-feral-dps](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/druid-feral-dps.md) |
| Druid | Resto | 901066-901073 (8) | 901074-901099 (26) | implemented | [druid-restoration](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/druid-restoration.md) |
| Non-Class | Global | 901100-901108 (9) | 901109-901199 (91) | implemented | [global](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/global.md) |

## Cross-spec notes (kept here because they apply across multiple specs)

- **Warrior Fury (900108-900121)** is intentionally placed inside the Arms numeric range. The IDs were created manually in `Spell.dbc` and are pure DBC — no C++ scripts. The old C++/SQL entries (900138-900145) were removed.
- **DK Frost** has two split IDs across the spec range: `900333` (Ghoul → Frost Wyrm marker) and `900368` (Frost Breath helper). The Frost Wyrm NPC (entry `900333`) has its own AI script `npc_custom_frost_wyrm` (DisplayID 26752 Sindragosa-style, DisplayScale 1.0, HealthModifier 2× via `creature_template`). Frost Breath: 2s cast, cone 20yd, 5000+1000rnd Frost damage + 50% slow 6s, scaled with owner AP (5000 + 50% AP).
- **Custom NPCs**: `900333` (Frost Wyrm — DK Frost), `900436` (Spirit Wolf — Shaman Enhance proc summon, DisplayID 27074), `901066` (Healing Treant — Druid Resto HoT proc).
- **Paladin "Consecration around you" (900205/900234/900268)** is shared across Holy / Prot / Ret. The Consecration DBC must be patched separately (TargetA → `TARGET_DEST_CASTER`).
- **Paladin "Judgement cd −2sec" (900241/900269)** is shared between Prot and Ret with separate IDs.
- **Shaman "Totems follow player" (900401/900433/900466)** is shared across Ele / Enhance / Resto via the `custom_totem_follow_playerscript` PlayerScript.
- **Shaman 900435 (Summons +50%)** is currently only a marker — the actual damage increase needs to be implemented via C++ pet scaling or owner→pet aura transfer.
- **Mage 900700 ("Channeling Evocation increases spell damage")** is the only shared Mage spell across all three specs.

## SpellFamilyName values

| Value | Class | Value | Class |
|------:|-------|------:|-------|
| 0 | Generic | 8 | Rogue |
| 3 | Mage | 9 | Hunter |
| 4 | Warrior | 10 | Paladin |
| 5 | Warlock | 11 | Shaman |
| 6 | Priest | 15 | Death Knight |
| 7 | Druid | | |

## Adding a new spell

See [`share-public/docs/custom-spells/04-adding-a-spell.md`](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/04-adding-a-spell.md) for the full step-by-step recipe (DBC → SQL → C++ → build → test).

After implementation, log the change in [`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md).
