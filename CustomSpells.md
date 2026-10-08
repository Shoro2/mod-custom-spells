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
| Warrior | Arms | 900100-900107 (8) | 900122-900132 (11) | implemented, T1 bots 2026-10-08 | [warrior-arms](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warrior-arms.md) |
| Warrior | Fury | 900108-900121, 900133 (15) | 900134-900165 (gaps 900138/140/141/144/145 retired) | implemented, T1 bots 2026-10-08 | [warrior-fury](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warrior-fury.md) |
| Warrior | Prot | 900168-900176 (9) | 900177-900199 (23) | implemented, T1 bots 2026-10-08 | [warrior-protection](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warrior-protection.md) |
| Paladin | Holy | 900200-900212 (13) | 900213-900232 (20) | implemented, T1 bots 2026-10-08 | [paladin-holy](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/paladin-holy.md) |
| Paladin | Prot | 900234-900241 (8) | 900242-900265 (24) | implemented, T1 bots 2026-10-08 | [paladin-protection](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/paladin-protection.md) |
| Paladin | Ret | 900268-900276 (9) | 900277-900299 (23) | implemented, T1 bots 2026-10-08 | [paladin-retribution](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/paladin-retribution.md) |
| DK | Blood | 900300-900308 (9) | 900309-900332 (24) | implemented, T1 bots 2026-10-08 | [death-knight-blood](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/death-knight-blood.md) |
| DK | Frost | 900333-900342, 900368 (11) | 900343-900365 (23) | implemented, T1 bots 2026-10-08 | [death-knight-frost](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/death-knight-frost.md) |
| DK | Unholy | 900366-900367, 900369-900374 (8) | 900375-900399 (25) | implemented, T1 bots 2026-10-08 | [death-knight-unholy](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/death-knight-unholy.md) |
| Shaman | Ele | 900400-900418 (19) | 900419-900432 (14) | implemented, T1 bots 2026-10-08 | [shaman-elemental](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/shaman-elemental.md) |
| Shaman | Enh | 900433-900440 (8) | 900441-900465 (25) | implemented, T1 bots 2026-10-08 | [shaman-enhancement](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/shaman-enhancement.md) |
| Shaman | Resto | 900466-900467 (2) | 900468-900499 (32) | implemented, T1 bots 2026-10-08 | [shaman-restoration](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/shaman-restoration.md) |
| Hunter | Shared | 900500-900501 (2) | (0) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [hunter-shared](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/hunter-shared.md) |
| Hunter | BM | 900502-900508, 900525-900526 (9) | 900509-900524, 900527-900532 (22) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [hunter-beast-mastery](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/hunter-beast-mastery.md) |
| Hunter | MM | 900533-900541 (9) | 900542-900565 (24) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [hunter-marksmanship](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/hunter-marksmanship.md) |
| Hunter | Surv | 900566, 900568-900572 (6) | 900567, 900573-900599 (28); 900567 retired (client record left) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [hunter-survival](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/hunter-survival.md) |
| Rogue | Assa | 900600-900605 (6) | 900606-900632 (27) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [rogue-assassination](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/rogue-assassination.md) |
| Rogue | Combat | 900633-900642 (10) | 900643-900665 (23) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [rogue-combat](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/rogue-combat.md) |
| Rogue | Sub | 900666-900674 (9) | 900675-900699 (25) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [rogue-subtlety](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/rogue-subtlety.md) |
| Mage | Shared | 900700 (1) | — | implemented, T1 bots 2026-10-08 | [mage-shared](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/mage-shared.md) |
| Mage | Arcane | 900701-900719 (19) | 900720-900732 (13) | implemented, T1 bots 2026-10-08 | [mage-arcane](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/mage-arcane.md) |
| Mage | Fire | 900733-900743 (11) | 900744-900765 (22, 900755 taken in the client) | implemented, T1 bots 2026-10-08 | [mage-fire](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/mage-fire.md) |
| Mage | Frost | 900766-900774 (9) | 900775-900799 (25) | implemented, T1 bots 2026-10-08 | [mage-frost](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/mage-frost.md) |
| Warlock | Affli | 900800-900806 (7) | 900807-900832 (26) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [warlock-affliction](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warlock-affliction.md) |
| Warlock | Demo | 900833-900841, 900844-900852, 900858 (19) | 900842-900843, 900853-900857, 900859-900865 (14); 900842/900843 retired | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [warlock-demonology](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warlock-demonology.md) |
| Warlock | Destro | 900866-900877 (12) | 900878-900899 (22) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [warlock-destruction](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/warlock-destruction.md) |
| Priest | Disc | 900900-900906 (7) | 900907-900932 (26) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [priest-discipline](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/priest-discipline.md) |
| Priest | Holy | 900933-900943 (11) | 900944-900965 (22) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [priest-holy](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/priest-holy.md) |
| Priest | Shadow | 900966-900976 (11) | 900977-900999 (23) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [priest-shadow](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/priest-shadow.md) |
| Druid | Balance | 901000-901007 (8) | 901008-901032 (25) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [druid-balance](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/druid-balance.md) |
| Druid | Feral Tank | 901033-901041 (9) | 901042-901048 (7) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [druid-feral-tank](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/druid-feral-tank.md) |
| Druid | Feral DPS | 901049-901056 (8) | 901057-901065 (9) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [druid-feral-dps](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/druid-feral-dps.md) |
| Druid | Resto | 901066-901075 (10) | 901076-901099 (24) | implemented, T1 bots 2026-10-08 (revision: custom-spells/09) | [druid-restoration](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/druid-restoration.md) |
| Non-Class | Global | 901100-901111 (12) | 901112-901199 (88) | implemented, T1 bots 2026-10-08 | [global](https://github.com/Shoro2/share-public/blob/main/docs/custom-spells/specs/global.md) |

## Cross-spec notes (kept here because they apply across multiple specs)

- **Warrior Fury (900108-900121)** is intentionally placed inside the Arms numeric range. The IDs were created manually in `Spell.dbc` and are pure DBC — no C++ scripts. The old C++/SQL entries (900138-900145) were removed.
- **DK Frost** has two split IDs across the spec range: `900333` (Ghoul → Frost Wyrm marker) and `900368` (Frost Breath helper). The Frost Wyrm NPC (entry `900333`) has its own AI script `npc_custom_frost_wyrm` (DisplayID 26752 Sindragosa-style, DisplayScale 1.0, HealthModifier 2× via `creature_template`). Frost Breath: 2s cast, cone 20yd, 5000+1000rnd Frost damage + 50% slow 6s, scaled with owner AP (5000 + 50% AP).
- **Custom NPCs**: `900333` (Frost Wyrm — DK Frost), `900436` (Spirit Wolf — Shaman Enhance proc summon, DisplayID 27074), `900525` (Beast Companion — the hunter's second pet, takes the real pet's model), `900835`-`900837` (Lesser Imp / Felguard / Voidwalker), `900964` (Redemption Guardian — Priest Holy 900938), `900999` (Lesser Tentacle — Priest Shadow 900975), `901066` (Healing Treant — Druid Resto HoT proc).
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
