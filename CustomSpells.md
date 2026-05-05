## Custom Spell Master Plan (All Classes)

> **Status legend**: `planned` → `implemented` → `tested`

### ID block schema

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

### Current allocation

| Class | Spec | Used | Free | Status |
|--------|------|--------|------|--------|
| Warrior | Arms | 900100-900107 (8) | 900108-900132 (25) | tested |
| Warrior | Fury | 900108-900121 (14) | 900122-900165 (44) | tested |
| Warrior | Prot | 900168-900175 (8) | 900176-900199 (24) | implemented |
| Paladin | Holy | 900200-900210 (11) | 900211-900232 (22) | implemented |
| Paladin | Prot | 900234-900241 (8) | 900242-900265 (24) | implemented |
| Paladin | Ret | 900268-900275 (8) | 900276-900299 (24) | implemented |
| DK | Blood | 900300-900304 (5) | 900305-900332 (28) | implemented |
| DK | Frost | 900333 (1) | 900334-900365 (32) | implemented |
| DK | Unholy | 900366-900368 (3) | 900369-900399 (31) | implemented |
| Shaman | Ele | 900400-900408 (9) | 900409-900432 (24) | implemented |
| Shaman | Enh | 900433-900440 (8) | 900441-900465 (25) | implemented |
| Shaman | Resto | 900466-900467 (2) | 900468-900499 (32) | implemented |
| Hunter | Shared | 900500-900501 (2) | — | implemented |
| Hunter | BM | 900502-900505 (4) | 900506-900532 (27) | implemented |
| Hunter | MM | 900533-900536 (4) | 900537-900565 (29) | implemented |
| Hunter | Surv | 900566-900567 (2) | 900568-900599 (32) | implemented |
| Druid | Balance | 901000-901005 (6) | 901006-901032 (27) | implemented |
| Druid | Feral Tank | 901033-901034 (2) | 901035-901048 (14) | implemented |
| Druid | Feral DPS | 901049-901051 (3) | 901052-901065 (14) | implemented |
| Druid | Resto | 901066-901073 (8) | 901074-901099 (26) | implemented |
| Rogue | Assa | 900600-900604 (5) | 900605-900632 (28) | implemented |
| Rogue | Combat | 900633-900638 (6) | 900639-900665 (27) | implemented |
| Rogue | Sub | 900666-900669 (4) | 900670-900699 (30) | implemented |
| Mage | Arcane | 900700-900713 (14) | 900714-900732 (19) | implemented |
| Mage | Fire | 900733-900740 (8) | 900741-900765 (25) | implemented |
| Mage | Frost | 900766-900774 (9) | 900775-900799 (25) | implemented |
| Warlock | Affli | 900800-900803 (4) | 900804-900832 (29) | implemented |
| Warlock | Demo | 900833-900844 (12) | 900845-900865 (21) | implemented |
| Warlock | Destro | 900866-900872 (7) | 900873-900899 (27) | implemented |
| Priest | Disc | 900900-900903 (4) | 900904-900932 (29) | implemented |
| Priest | Holy | 900933 (1) | 900934-900965 (32) | implemented |
| Priest | Shadow | 900966-900968 (3) | 900969-900999 (31) | implemented |
| Non-Class | Global | 901100-901108 (9) | 901109-901199 (91) | implemented |

---

### Warrior — Arms (900100-900107)

| # | Spell ID | Effect | Approach | Status |
|---|----------|--------|--------|--------|
| 1 | 900100 | Mortal Strike damage +50% | DBC | tested |
| 2 | 900101 | Mortal Strike cd -2sec | DBC | tested |
| 3 | 900102 | Overpower damage +50% | DBC | tested |
| 4 | 900103 | Overpower +9 targets (AoE) | DBC+C++ | tested |
| 5 | 900104 | Mortal Strike +9 targets (AoE) | DBC+C++ | tested |
| 6 | 900105 | 20% Crit-chance → Enhanced Execute (5 stacks) | C++ | tested |
| 7 | 900106 | Enhanced Execute (Damage Spell = Paragon Strike) | C++ | tested |
| 8 | 900107 | Phys. Damage → Bladestorm CD -0.5s | C++ | tested |

### Warrior — Fury (900108-900121)

> **Special note**: The Fury-Spells have been created manually directly in Spell.dbc (IDs 900108-900121),
> not via the ID block schema (900133-900165). They therefore reside in the Arms range of the ID table,
> but are functionally Fury spells. No C++ needed — all effects run via DBC passive auras,
> proc triggers and spell_proc. The old C++/SQL entries (900138-900145) were removed.

| # | Spell ID | Effect | Approach | Status |
|---|----------|--------|--------|--------|
| 1 | 900108 | Whirlwind unlimited targets | DBC | tested |
| 2 | 900109 | Improved Bloodthirst Damage | DBC | tested |
| 3 | 900110 | Bloodthirst Cleave | DBC | tested |
| 4 | 900111 | Improved Whirlwind Damage | DBC | tested |
| 5 | 900112 | Cleave unlimited targets | DBC | tested |
| 6 | 900113 | Whirly Attacks (Proc) | DBC | tested |
| 7 | 900114 | Whirly Attacks (Passive) | DBC | tested |
| 8 | 900115 | Bloody Whirlwind (Aura) | DBC | tested |
| 9 | 900116 | Bloody Whirlwind (Passive) | DBC | tested |
| 10 | 900117 | Speedy Bloodthirst (Passive) | DBC | tested |
| 11 | 900118 | Whirlwind: Overpower (Passive) | DBC | tested |
| 12 | 900119 | Whirlwind: Bloodthirst (Passive) | DBC | tested |
| 13 | 900120 | Whirlwind: Overpower (Proc) | DBC | tested |
| 14 | 900121 | Whirlwind: Bloodthirst (Proc) | DBC | tested |

### Warrior — Prot (900166-900199)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900168 | Revenge +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x400 (Revenge). spell_dbc entry. |
| 2 | 900169 | Revenge unlimited targets | C++ | implemented | Passive Marker aura (DUMMY). C++ SpellScript on Revenge (57823): `AfterHit` → `AnyUnfriendlyUnitInObjectRangeCheck(8yd)` → DealDamage on all enemies. Checks `HasAura(900169)`. |
| 3 | 900170 | Thunderclap → Rend + 5× Sunder Armor | C++ | implemented | Passive Marker aura (DUMMY). C++ SpellScript on TC (47502): `AfterHit` per target → CastSpell(Rend 47465) + 5× CastSpell(SunderArmor 58567). Checks `HasAura(900170)`. |
| 4 | 900171 | Thunderclap +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x80 (TC). spell_dbc entry. |
| 5 | 900172 | AoE damage on Block | C++ | implemented | Proc aura (DUMMY) with spell_proc: ProcFlags=0x2, 100% chance, 1s ICD. C++ HandleProc: Checks `PROC_HIT_BLOCK` → CastSpell(900174). |
| 6 | 900173 | 10% Block → Enhanced Thunderclap | C++ | implemented | Proc aura (DUMMY) with spell_proc: ProcFlags=0x2, 10% chance, 3s ICD. C++ HandleProc: Checks `PROC_HIT_BLOCK` → CastSpell(900175). |
| H1 | 900174 | Helper: Block AoE Damage | DBC | implemented | Instant AoE Physical Damage. Effect=SCHOOL_DAMAGE(2), Target=SRC_AREA_ENEMY(22), BasePoints=500+100rnd, Radius=8yd. |
| H2 | 900175 | Helper: Enhanced Thunderclap | DBC | implemented | Instant AoE Physical Damage. Effect=SCHOOL_DAMAGE(2), Target=SRC_AREA_ENEMY(22), BasePoints=1000+200rnd, Radius=10yd. |

> **Note Prot**: 900168/900171 are DBC-only (no C++ needed). 900169/900170 hook into existing spells (57823/47502) and check the marker aura via HasAura. 900172/900173 use the proc system with block detection. SpellFamilyFlags for Revenge (0x400) and TC (0x80) must be verified in-game.

---

### Paladin — Holy (900200-900232)

> Paladin SpellFamilyName = 10. Holy Shock (20473) is a dummy spell that routes to damage (25912+) or heal (25914+). scripts hook into the damage/heal variants via negative spell_script_names IDs (-25912, -25914 = all ranks).

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900200 | Holy Shock AoE damage | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on HS-Damage (-25912): `AfterHit` → CastSpell(900208) AoE Holy damage around target. Checks `HasAura(900200)`. |
| 2 | 900201 | Holy Shock AoE heal | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on HS-Heal (-25914): `AfterHit` → CastSpell(900209) AoE Holy heal around target. Checks `HasAura(900201)`. |
| 3 | 900202 | Holy Shock always both | C++ | implemented | Marker aura (DUMMY). Two scripts: (a) on HS-Damage: `AfterHit` → auto-heal nearest injured Ally (HS-Heal R7). (b) on HS-Heal: `AfterHit` → auto-damage nearest enemy (HS-Damage R7). Checks `HasAura(900202)`. |
| 4 | 900203 | Holy Shock +50% | DBC | implemented | Passive Aura: `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x200000 (Holy Shock). spell_dbc entry. |
| 5 | 900204 | Consecration also heals | C++ | implemented | Marker aura (DUMMY). C++ AuraScript on Consecration (48819): `OnEffectPeriodic` → CastSpell(900210) AoE Holy heal around caster. Checks `HasAura(900204)`. |
| 6 | 900205 | Consecration around you | DBC | implemented | Marker aura. Consecration-DBC must be patched separately (TargetA → `TARGET_DEST_CASTER`). **Shared with Prot and Ret**. |
| 7 | 900206 | Consecration +50% | DBC | implemented | Passive Aura: `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x20 (Consecration). |
| 8 | 900207 | Consecration +5sec | DBC | implemented | Passive Aura: `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_DURATION` (17). BasePoints=5000ms. EffectSpellClassMaskA=0x20. |
| H1 | 900208 | Helper: HS AoE Damage | DBC | implemented | Instant AoE Holy Damage. Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), BasePoints=800+100rnd, 10yd. |
| H2 | 900209 | Helper: HS AoE Heal | DBC | implemented | Instant AoE Holy Heal. Effect=HEAL(10), Target=DEST_AREA_ALLY(30), BasePoints=800+100rnd, 10yd. |
| H3 | 900210 | Helper: Consec Heal Tick | DBC | implemented | Instant AoE Holy Heal. Effect=HEAL(10), Target=SRC_AREA_ALLY(31), BasePoints=200+50rnd, 8yd. |

> **Note Holy**: SpellFamilyFlags for Holy Shock (0x200000) and Consecration (0x20) must be verified in-game. 900205 (Consec around you) needs additionally a DBC patch on the base spell (48819) to change TargetA — the marker aura alone is not enough.

### Paladin — Prot (900233-900265)

> **ID shift**: Originally 900208-900238 planned, but 900208-900210 are used as Holy helpers. Prot starts at 900234.

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900234 | Consecration around you | DBC | implemented | Marker aura (DUMMY). Consecration-DBC must be patched separately. **Shared with Holy (900205) and Ret**. |
| 2 | 900235 | Avenger's Shield +9 targets | DBC | implemented | `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_JUMP_TARGETS` (17). BasePoints=9. EffectSpellClassMaskA=0x4000. |
| 3 | 900236 | Avenger's Shield +50% damage | DBC | implemented | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x4000. |
| 4 | 900237 | Holy Shield charges +99 | DBC | implemented | `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_CHARGES` (4). BasePoints=99. EffectSpellClassMaskB=0x20 (flags[1]). |
| 5 | 900238 | Holy Shield +50% damage | DBC | implemented | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskB=0x20 (flags[1]). |
| 6 | 900239 | AS leaves Consecration | C++ | implemented | Marker + C++ on AS (-48827): `AfterHit` → CastSpell(Consec 48819, triggered). Checks `HasAura(900239)`. |
| 7 | 900240 | Judgement → free AS | C++ | implemented | Marker + C++ on Judgement Damage (54158): `AfterHit` → CastSpell(AS 48827, triggered). Checks `HasAura(900240)`. |
| 8 | 900241 | Judgement cd -2sec | DBC | implemented | `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_COOLDOWN` (11). BasePoints=-2000ms. EffectSpellClassMaskA=0x800000. |

> **Note Prot**: SpellFamilyFlags verify: AS=0x4000(flags[0]), HolyShield=0x20(flags[1]), Judgement=0x800000(flags[0]). 900237 (charges +99) must be tested whether SPELLMOD_CHARGES works.

### Paladin — Ret (900266-900299)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900268 | Consecration around you | DBC | implemented | Marker aura (DUMMY). **Shared concept with Holy (900205) and Prot (900234)**. Separate ID per spec. |
| 2 | 900269 | Judgement cd -2sec | DBC | implemented | `ADD_FLAT_MODIFIER` (107) + `SPELLMOD_COOLDOWN` (11). BasePoints=-2000ms. EffectSpellClassMaskA=0x800000. **Shared concept with Prot (900241)**. |
| 3 | 900270 | Divine Storm +6 targets | DBC | implemented | Marker aura (DUMMY). DS-Base-Spell (53385) needs DBC patch: `MaxAffectedTargets` on 10. No SpellMod available for AoE targets. |
| 4 | 900271 | Divine Storm +50% damage | DBC | implemented | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskB=0x20000 (DS flags[1], verify!). |
| 5 | 900272 | Crusader Strike +50% damage | DBC | implemented | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x1 (CS flags[0], verify!). |
| 6 | 900273 | Crusader Strike +9 targets | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on CS (-35395): `AfterHit` → DealDamage on 9 extra enemies in 8yd. Checks `HasAura(900273)`. |
| 7 | 900274 | CS/Judge/DS → Exorcism buff | C++ | implemented | Passive proc aura (DUMMY). spell_proc: ProcFlags=0x10, 100%. C++ CheckProc filters on CS(35395)/Judge(54158)/DS(53385). HandleProc → CastSpell(900275). |
| H1 | 900275 | Exorcism Power (buff) | DBC | implemented | Stacking Buff: `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). +50% Exorcism dmg per stack, max 10. 30s Duration. Consumed by Exorcism (48801) via C++ AfterCast. |

> **Note Ret**: SpellFamilyFlags verify: DS=0x20000(flags[1]), CS=0x1(flags[0]), Exorcism=0x200000(flags[0]). 900270 (DS +6 targets) needs DBC patch on the base spell. Exorcism-Buff (900275) EffectSpellClassMaskA=0x200000 must correctly match Exorcism.

---

### DK — Blood (900300-900332)

> DK SpellFamilyName = 15. Dancing Rune Weapon (49028) summons a Rune Weapon (NPC 27893) that copies the DK's spells.

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900300 | 3 Rune Weapons simultaneously | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on DRW (49028): `AfterCast` → 2× CastSpell(DRW, triggered=true). Checks `HasAura(900300)`. |
| 2 | 900301 | Rune Weapon double-casts | C++ | implemented | Marker aura (DUMMY). C++ AuraScript on DRW (49028): `OnEffectProc` → extra CastSpell/DealMeleeDamage. Checks `HasAura(900301)`. Rune Weapon casts each spell 2× instead of 1×. |
| 3 | 900302 | Heart Strike +50% damage | DBC | implemented | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_DAMAGE` (0). EffectSpellClassMaskA=0x2000000 (HS flags[0], verify!). |
| 4 | 900303 | Heart Strike +9 targets | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on HS (-55050): `AfterHit` → DealDamage on 9 extra enemies in 8yd. Checks `HasAura(900303)`. |
| 5 | 900304 | Dealing damage → chance Death Coil | C++ | implemented | Proc aura (DUMMY). spell_proc: ProcFlags=0x14, 15% chance, 3s ICD. C++ HandleProc → CastSpell(47632 Death Coil Damage, triggered). |

### DK — Frost (900333-900365)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900333 | Ghoul → Frost Wyrm | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on Raise Dead (46584): `AfterCast` → despawns Ghoul, SummonCreature(900333 Frost Wyrm). Checks `HasAura(900333)`. Frost Wyrm NPC has its own AI (`npc_custom_frost_wyrm`), 2× Gargoyle HP, casts Frost Breath. |
| H1 | 900368 | Frost Breath | DBC+C++ | implemented | 2s Cast, Cone 20yd, 5000+1000rnd Frost Damage + 50% Slow 6s. C++ `spell_custom_frost_breath` scales damage with owner AP (5000 + 50% AP). |

### DK — Unholy (900366-900399)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900366 | DoTs → Shadow AoE proc | C++ | implemented | Proc aura (DUMMY). spell_proc: ProcFlags=0x400000 (DONE_PERIODIC), 20% chance, 2s ICD. C++ HandleProc → CastSpell(900367, triggered) on DoT target. |
| H1 | 900367 | Shadow Eruption (helper) | DBC | implemented | Instant AoE Shadow Damage. Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), SchoolMask=32(Shadow), BasePoints=600+150rnd, 8yd. |

> **Note DK**: Heart Strike SpellFamilyFlags[0]=0x2000000 verify! 900300 (3 DRW) casts DRW again as triggered — can lead to aura stacking issues if not correctly handled. 900333 (Frost Wyrm) has its own creature_template (NPC 900333) + CreatureScript (`npc_custom_frost_wyrm`) with Frost Breath AI. DisplayID 26752 (Sindragosa-style), Scale 0.5, 2× Gargoyle HP.

---

### Shaman — Elemental (900400-900432)

> Shaman SpellFamilyName = 11. Chain Lightning flags[0]=0x2, Flame Shock flags[0]=0x10000000, Lightning Overload icon=2018.

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900400 | Chain Lightning +6 targets, no dmg reduction | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on CL (-49271): `AfterHit` → CastCustomSpell(900408) on 6 extra enemies within 12yd at full damage. Checks `HasAura(900400)`. |
| 2 | 900401 | Totems follow player | C++/PlayerScript | implemented | Marker aura (DUMMY). `custom_totem_follow_playerscript::OnPlayerUpdate` → every 2s checks whether totems are >5yd away → NearTeleportTo(Player). Checks `HasAura(900401)`. |
| 3 | 900402 | Fire Elemental → Ragnaros | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on Fire Ele Totem (2894): `AfterCast` → SetDisplayId(11121 Ragnaros), Scale 0.35, 2× HP. Checks `HasAura(900402)`. |
| 4 | 900403 | Lightning Overload + Lava Burst | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on LvB (-51505): `AfterHit` → checks LO Talent (icon 2018), double proc chance, CastCustomSpell(LvB, half damage, triggered). Checks `HasAura(900403)`. |
| 5 | 900404 | Lava Burst spreads Flame Shock | C++ | implemented | Marker aura (DUMMY). C++ SpellScript on LvB (-51505): `AfterHit` → checks whether Target FS has (flags[0]=0x10000000), CastSpell(FS) on 5 extra enemies in 10yd. Checks `HasAura(900404)`. |
| 6 | 900405 | Flame Shock ticks → reset LvB CD | C++ | implemented | Proc aura (DUMMY). spell_proc: ProcFlags=0x400000, SpellFamilyMask0=0x10000000, 15% chance, 2s ICD. C++ HandleProc → RemoveSpellCooldown(51505). |
| 7 | 900406 | Lava Burst two charges | C++ | implemented | Stacking DUMMY (CumulativeAura=2). C++ SpellScript on LvB (-51505): `AfterCast` → stack count as charge tracker (1=first charge used→reset CD, 2=second charge→normal CD). Checks `HasAura(900406)`. |
| 8 | 900407 | Clearcasting → Lava Burst instant | DBC | implemented | `ADD_PCT_MODIFIER` (108) + `SPELLMOD_CASTING_TIME` (14) = -100%. EffectSpellClassMaskB=0x1000 (LvB flags, verify!). Makes LvB permanently instant when the passive is active. |
| H1 | 900408 | Chain Lightning Arc (helper) | DBC | implemented | Instant Nature Damage. Effect=SCHOOL_DAMAGE(2), Target=ENEMY(6), SchoolMask=8(Nature). BasePoints overridden via CastCustomSpell. |

> **Note Ele**: Lava Burst SpellFamilyFlags in EffectSpellClassMaskB for 900407 must be verified (0x1000 is estimate). 900401 (Totem Follow) uses NearTeleportTo instead of MoveFollow because totems have no real movement — can cause visual hitching. 900402 (Ragnaros) is just a display swap + HP buff, no own AI. 900406 (LvB Charges) uses aura stacks as charge tracker — works but may have edge cases on rapid casting.

### Shaman — Enhancement (900433-900465)

> Maelstrom Weapon (53817) stacks to 5. Feral Spirit (51533) summons 2 Spirit Wolves (NPC 29264).

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900433 | Totems follow player | PlayerScript | implemented | DUMMY marker. Shared `custom_totem_follow_playerscript` also checks `HasAura(900433)`. teleports totems every 2s. |
| 2 | 900434 | 5 Maelstrom stacks → summons AoE 5s | C++ | implemented | DUMMY marker. C++ AuraScript on Maelstrom Weapon (53817): `OnEffectApply` on stack=5 → CastSpell(900439 Buff) + CastSpell(900440 AoE) on all controlled units. |
| 3 | 900435 | Summons +50% damage | DBC | implemented | DUMMY marker (BasePoints=50). Currently only a marker — +50% pet scaling must be implemented via C++ pet scaling or owner aura transfer. |
| 4 | 900436 | Auto attacks → summon wolf | C++ | implemented | Proc aura (DUMMY). spell_proc: ProcFlags=0x4 (melee auto), 10%, 5s ICD. C++ HandleProc → SummonCreature(900436 Spirit Wolf, 15s). NPC 900436 has DisplayID 27074 (Wolf). |
| 5 | 900437 | Spirit Wolves inherit haste | C++ | implemented | DUMMY marker. C++ SpellScript on Feral Spirit (51533): `AfterCast` → reads owner haste (UNIT_MOD_CAST_SPEED) → SetAttackTime on wolves. |
| 6 | 900438 | Spirit Wolves 5% CL on hit | C++/UnitScript | implemented | DUMMY marker. `custom_wolf_cl_unitscript::OnDamage` → if attacker=Spirit Wolf (29264) + owner HasAura(900438) → 5% CastSpell(CL 49271). |
| H1 | 900439 | Maelstrom Fury (buff) | DBC | implemented | 5s DUMMY buff (DurationIndex=18). visual marker for empowered summons. |
| H2 | 900440 | Spirit Howl (AoE helper) | DBC | implemented | Instant AoE Physical Damage. Effect=SCHOOL_DAMAGE(2), Target=SRC_AREA_ENEMY(22), 800+200rnd, 8yd. |

### Shaman — Restoration (900466-900499)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900466 | Totems follow player | PlayerScript | implemented | DUMMY marker. Shared `custom_totem_follow_playerscript` also checks `HasAura(900466)`. |
| 2 | 900467 | Mana regen +2% per missing mana% | C++/PlayerScript | implemented | DUMMY marker. `custom_mana_regen_playerscript::OnPlayerUpdate` → every 5s: missingPct = (1 - curMana/maxMana) × 100 → EnergizeBySpell(maxMana × missingPct × 0.02 / 100). At 50% missing → +1% maxMana/5s. At 90% missing → +1.8% maxMana/5s. |

> **Note Enhance**: 900435 (Summons +50%) is currently only a marker — the actual damage increase must be implemented via pet scaling or owner→pet aura transfer. 900434 (Maelstrom AoE) casts the AoE Helper directly on all controlled units at 5 stacks — the AoE fires once, not for 5s on every attack (simplified). 900438 (Wolf CL) uses UnitScript::OnDamage which fires for ALL damage events — monitor performance.

---

### Hunter — Shared (900500-900501)

> Hunter SpellFamilyName = 9. "Get back arrows" and "Multishot unlimited targets" apply to all 3 specs → shared spells.

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900500 | Get back arrows (no ammo consumption) | C++/PlayerScript | implemented | DUMMY marker. `custom_hunter_arrows_playerscript::OnSpellCast` → after every ranged spell (SPELL_ATTR0_USES_RANGED_SLOT) → StoreNewItemInBestSlots(ammoId, 1). Checks `HasAura(900500)`. |
| 2 | 900501 | Multi-Shot unlimited targets | C++ | implemented | DUMMY marker. C++ SpellScript on Multi-Shot (-49048): `AfterHit` → finds ALL enemies within 10yd radius around target → DealDamage with full Multi-Shot damage on each. Checks `HasAura(900501)`. |

### Hunter — Beast Mastery (900502-900532)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900502 | Pet damage +50% | C++/UnitScript | implemented | DUMMY marker. `custom_hunter_pet_unitscript::OnDamage` → if attacker=Pet + owner HasAura(900502) → damage *= 1.5f. |
| 2 | 900503 | Pet attack speed +50% | C++/PlayerScript | implemented | DUMMY marker. `custom_hunter_pet_speed_playerscript::OnPlayerUpdate` → every 3s: SetAttackTime(BASE_ATTACK, CreateAttackTime * 0.5f) on Pet. Checks `HasAura(900503)`. |
| 3 | 900504 | Pet AoE damage proc | C++/UnitScript | implemented | DUMMY marker. `custom_hunter_pet_aoe_unitscript::OnDamage` → if Pet hit + owner HasAura(900504) → 15% chance → CastSpell(900505 Beast Cleave). |
| H1 | 900505 | Helper: Beast Cleave AoE | DBC | implemented | Instant AoE Physical Damage. Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), 800+200rnd, 8yd. |

### Hunter — Marksmanship (900533-900565)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900533 | Auto Shot bounces +9 targets | C++ | implemented | DUMMY marker. C++ SpellScript on Auto Shot (75): `AfterHit` → finds up to 9 enemies in 10yd radius around target → CastCustomSpell(900535 Ricochet, damage) on each. Checks `HasAura(900533)`. |
| 2 | 900534 | Multi-Shot Barrage (0.1s ticks for 2s, 50% slow) | C++ | implemented | Active spell: 2s PERIODIC_DUMMY (Amplitude=100ms). AuraScript: `OnApply` → CastSpell(900536 Slow), `OnRemove` → RemoveAura(900536). `HandlePeriodic` → CastSpell(Multi-Shot 49048, triggered). 20 Multi-Shots in 2s. |
| H1 | 900535 | Helper: Ricochet Shot | DBC | implemented | Instant Physical single-target damage. BasePoints via CastCustomSpell. |
| H2 | 900536 | Helper: Barrage Slow | DBC | implemented | 2s APPLY_AURA MOD_DECREASE_SPEED -50% on caster. |

### Hunter — Survival (900566-900599)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900566 | chance to drop explosion on damage | C++ | implemented | Proc aura (DUMMY). spell_proc: ProcFlags=0x44 (ranged auto + ranged spell), 15% chance, 2s ICD. C++ HandleProc → CastSpell(900567 Explosive Burst, triggered) on target. |
| H1 | 900567 | Helper: Explosive Burst | DBC | implemented | Instant AoE Fire Damage. Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), SchoolMask=4(Fire), 1000+200rnd, 8yd. |

> **Note Hunter**: 900500 (Arrows) uses StoreNewItemInBestSlots which creates a new item stack — may fail on full bags. 900534 (Barrage) casts 20 Multi-Shots in 2s — monitor performance on many mobs. Pet UnitScripts (900502/900504) fire for ALL damage events — check whether the creature is a pet with an owner to protect performance.

---

### Druid — Balance (901000-901032)

> Druid SpellFamilyName = 7. Moonfire flags[0]=0x2, Starfall flags[0]=0x100 (verify!).

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 901000 | Moonfire +9 targets | C++ | implemented | DUMMY marker. C++ SpellScript on Moonfire (-48463): `AfterHit` → finds up to 9 enemies in 10yd radius → CastSpell(Moonfire R14, triggered) on each. Checks `HasAura(901000)`. |
| 2 | 901001 | Moonfire +50% damage | DBC | implemented | ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0). EffectSpellClassMaskA=0x2 targets Moonfire. |
| 3 | 901002 | Starfall +9 targets | DBC | implemented | ADD_FLAT_MODIFIER (107) + SPELLMOD_JUMP_TARGETS (17). EffectSpellClassMaskA=0x100 targets Starfall. BasePoints=9. |
| 4 | 901003 | Starfall +50% damage | DBC | implemented | ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0). EffectSpellClassMaskA=0x100. |
| 5 | 901004 | Spell dmg reduces Starfall CD | C++ | implemented | Proc aura (DUMMY). spell_proc: ProcFlags=0x10010 (spell magic dmg), 100% chance, 1s ICD. C++ HandleProc → ModifySpellCooldown(Starfall, -1000). CheckProc filters on Druid SpellFamily. |
| 6 | 901005 | Starfall stacks to 10 | DBC | implemented | ADD_FLAT_MODIFIER (107) + SPELLMOD_CHARGES (4). EffectSpellClassMaskA=0x100. BasePoints=9 (+9 charges). |

### Druid — Feral Tank (901033-901048)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 901033 | Swipe Bear applies bleed | C++ | implemented | DUMMY marker. C++ SpellScript on Swipe Bear (-48562): `AfterHit` → CastSpell(901034 Swipe Bleed DoT, triggered). Checks `HasAura(901033)`. |
| H1 | 901034 | Helper: Swipe Bleed DoT | DBC | implemented | APPLY_AURA PERIODIC_DAMAGE. 300+50rnd Physical per 3s tick, 12s duration. |

### Druid — Feral DPS (901049-901065)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 901049 | Swipe Cat applies bleed | C++ | implemented | DUMMY marker. C++ SpellScript on Swipe Cat (62078): `AfterHit` → CastSpell(901050 Rake Bleed DoT, triggered). Checks `HasAura(901049)`. |
| H1 | 901050 | Helper: Rake Bleed DoT | DBC | implemented | APPLY_AURA PERIODIC_DAMAGE. 300+50rnd Physical per 3s tick, 12s duration. |
| 2 | 901051 | Energy regen +50% | DBC | implemented | SPELL_AURA_MOD_POWER_REGEN_PERCENT (110), MiscValue=3 (Energy). BasePoints=50. |

### Druid — Resto (901066-901099)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 901066 | HoTs chance to summon Force of Nature | C++ | implemented | Proc aura (DUMMY). spell_proc: ProcFlags=0x40000 (periodic heal), 5% chance, 5s ICD. C++ HandleProc → SummonCreature(901066 Healing Treant, 30s). Treant attacks enemy or follows player. |
| 2 | 901067 | Summons scale with healing power | C++/PlayerScript | implemented | DUMMY marker. `custom_druid_summon_scale_playerscript::OnPlayerUpdate` → every 3s: for each Controlled Unit → SetMaxHealth(baseHP + spellPower*10). |
| 3 | 901068 | Summons heal on death/despawn | C++/UnitScript | implemented | DUMMY marker. `custom_druid_summon_heal_unitscript::OnUnitDeath` → if a summon dies + owner HasAura(901068) → CastSpell(901073 Nature Bloom). |
| 4 | 901069 | Thorns → chance to cast Rejuv | C++/UnitScript | implemented | DUMMY marker. `custom_druid_thorns_rejuv_unitscript::OnDamage` → if victim=player + HasAura(901069) + has Thorns → 20% chance → CastSpell(Rejuv R15, triggered). |
| 5 | 901070 | HoTs +50% healing | DBC | implemented | ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0). EffectSpellClassMaskA=0x30 targets Rejuv+Regrowth. |
| 6 | 901071 | HoTs tick 2x fast + 2x duration | DBC | implemented | ADD_PCT_MODIFIER (108) + SPELLMOD_DURATION (17), BasePoints=100 (+100% duration). EffectSpellClassMaskA=0x30. Double duration = double ticks at same interval. |
| 7 | 901072 | Mana regen per missing mana% (+2%) | C++/PlayerScript | implemented | DUMMY marker. `custom_druid_mana_regen_playerscript::OnPlayerUpdate` → every 5s: missingPct × 0.02 × maxMana / 100 → EnergizeBySpell. Same pattern as Shaman Resto (900467). |
| H1 | 901073 | Helper: Nature Bloom (treant death heal) | DBC | implemented | Instant AoE Nature Heal. Effect=HEAL(10), Target=DEST_AREA_ALLY(30), 2000+500rnd, SchoolMask=8. |

> **Note Druid**: NPC 901066 (Healing Treant) exists in creature_template. 901071 (HoTs 2x) uses a duration multiplier — doubled duration means double the ticks at the same interval. For truly "twice as fast ticking" you would need a C++ approach that halves EffectAmplitude. UnitScripts (901068/901069) fire for ALL unit events — monitor performance.

---

### Rogue — Assassination (900600-900632)

> Rogue SpellFamilyName = 8. Mutilate flags[1]=0x200000, Poison flags[0]=0x8000+flags[1]=0x10000 (verify!).

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900600 | Energy regen +50% | DBC | implemented | SPELL_AURA_MOD_POWER_REGEN_PERCENT (110), MiscValue=3 (Energy). BasePoints=50. |
| 2 | 900601 | Mutilate +50% damage | DBC | implemented | ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0). EffectSpellClassMaskB=0x200000 targets Mutilate. |
| 3 | 900602 | Poison damage +50% | DBC | implemented | ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0). EffectSpellClassMaskA=0x8000 + EffectSpellClassMaskB=0x10000 targets Poison spells. |
| 4 | 900603 | Poison Nova proc (15%, 3s ICD) | C++ | implemented | Proc aura (DUMMY). spell_proc: ProcFlags=0x10000 (spell magic dmg), SchoolMask=8(Nature), SpellFamily=8(Rogue), 15% chance, 3s ICD. C++ HandleProc → CastSpell(900604 Poison Nova). |
| H1 | 900604 | Helper: Poison Nova AoE | DBC | implemented | Instant AoE Nature Damage. Effect=SCHOOL_DAMAGE(2), Target=DEST_AREA_ENEMY(15), SchoolMask=8(Nature), 800+200rnd, 8yd. |

### Rogue — Combat (900633-900665)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900633 | SS +50% damage | DBC | implemented | ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0). EffectSpellClassMaskA=0x2 targets SS. |
| 2 | 900634 | SS +9 targets | C++ | implemented | DUMMY marker. C++ SpellScript on SS (-48638): `AfterHit` → finds up to 9 enemies in 8yd radius → CastCustomSpell(900638, damage) on each. Checks `HasAura(900634)`. |
| 3 | 900635 | Blade Flurry 2min duration | DBC | implemented | ADD_FLAT_MODIFIER (107) + SPELLMOD_DURATION (17). BasePoints=105000 (15s base +105s = 120s). EffectSpellClassMaskB=0x800. |
| 4 | 900636 | Blade Flurry +9 targets | DBC | implemented | ADD_FLAT_MODIFIER (107) + SPELLMOD_JUMP_TARGETS (17). BasePoints=9. EffectSpellClassMaskB=0x800. |
| 5 | 900637 | Energy regen +50% | DBC | implemented | SPELL_AURA_MOD_POWER_REGEN_PERCENT (110), MiscValue=3 (Energy). BasePoints=50. |
| H1 | 900638 | Helper: Sinister Slash | DBC | implemented | Instant Physical single-target damage. BasePoints via CastCustomSpell. |

### Rogue — Subtlety (900666-900699)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900666 | Energy regen +50% | DBC | implemented | SPELL_AURA_MOD_POWER_REGEN_PERCENT (110), MiscValue=3 (Energy). BasePoints=50. |
| 2 | 900667 | Hemorrhage +50% damage | DBC | implemented | ADD_PCT_MODIFIER (108) + SPELLMOD_DAMAGE (0). EffectSpellClassMaskA=0x2000000 targets Hemo. |
| 3 | 900668 | Hemorrhage +9 targets | C++ | implemented | DUMMY marker. C++ SpellScript on Hemo (-48660): `AfterHit` → finds up to 9 enemies in 8yd radius → CastCustomSpell(900669, damage) on each. Checks `HasAura(900668)`. |
| H1 | 900669 | Helper: Deep Cut | DBC | implemented | Instant Physical single-target damage. BasePoints via CastCustomSpell. |

> **Note Rogue**: SpellFamilyFlags verify: SS=0x2(flags[0]), Mutilate=0x200000(flags[1]), Hemorrhage=0x2000000(flags[0]), BF=0x800(flags[1]). 900602 (Poison +50%) uses wide mask — verify whether all Poison spells are mapped correctly. 900635/900636 (BF Duration/Targets) use SPELLMOD_DURATION and SPELLMOD_JUMP_TARGETS respectively on the same BF mask — cannot interfere with each other since they have different MiscValues.

---

### Mage — Shared (900700-900732)

> Mage SpellFamilyName = 3. "Channeling Evocation increases spell damage" applies to all 3 specs → shared Spell.

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900700 | Channeling Evocation increases spell damage | C++ | implemented | Evocation (12051) is a channel spell (8s). Approach: AuraScript on Evocation → `OnApply`: stack a buff with `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` (ALL_SCHOOLS) on caster, increasing per tick. `OnRemove`: buff remains X seconds or permanently. Alternative: during channel a stacking aura, e.g. +10% spell damage per second of channeling → up to +80% after a full channel. duration of the buff after channel is configurable. |

### Mage — Arcane (900700-900732)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900701 | Mana regen +2% per missing mana % | C++ | implemented | Dynamic mana regen scaling. `PlayerScript::OnUpdateManaRegen` or periodic aura tick: compute missing mana% → set regen bonus = missing% × 2%. At 50% mana missing → +100% Mana Regen. At 90% missing → +180%. Passive Aura with `SPELL_AURA_OBS_MOD_POWER` or C++ Hook on `Player::RegenerateMana()`. Very strong mana-sustain mechanic. |
| 2 | 900702 | Arcane Barrage +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Arcane Barrage (44781). SpellFamilyName=3, SpellFamilyFlags for Barrage. Simple damage multiplier. |
| 3 | 900703 | Arcane Barrage +9 targets | DBC/C++ | implemented | Arcane Barrage (44781) normally hits 3 targets. DBC: `MaxAffectedTargets` to 10+. Or C++: `OnObjectAreaTargetSelect` → remove target limit. |
| 4 | 900704 | Arcane Blast cast time -50% | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` (SPELLMOD_CASTING_TIME) -50% on Arcane Blast (42897). Base cast time 2.5s → 1.25s. Stacks with the Arcane Blast debuff (still faster with stacks). |
| 5 | 900705 | Arcane Blast +9 targets | C++ | implemented | Arcane Blast (42897) is Single-Target. SpellScript `AfterHit` → chains to 9 additional enemies within radius. CastSpell(AB damage helper, triggered=true). Needs helper spell (e.g. 900710). |
| 6 | 900706 | Arcane Charges stack up to 8 | DBC/C++ | implemented | Arcane Blast Debuff (36032) normally stacks up to 4. DBC: set `StackAmount` to 8. C++: If hardcoded → `AuraScript::OnStackChange` → allow stacks >4. Each stack increases AB damage +15% and mana cost +150% (base values). 8 stacks = +120% Damage, +1200% mana cost. Balance accordingly! |
| 7 | 900707 | Arcane Explosion generates 1 Arcane Charge (like Arcane Blast) | C++ | implemented | Arcane Explosion (42921) is AoE Instant. SpellScript `AfterCast` → ApplyAura(Arcane Blast Debuff 36032, 1 stack) on caster. Same mechanic as AB but without consume. AE becomes an AoE arcane charge generator. |
| 8 | 900708 | Below 30% health → activate Mana Shield + restore all mana | C++ | implemented | Passive proc aura: `PROC_FLAG_TAKEN_DAMAGE` (0x4000). `HandleProc`: When Health <30% → CastSpell(Mana Shield 43020, triggered=true) + SetPower(MANA, MaxMana). ICD recommended (e.g. 60s) to prevent abuse. Very strong survival mechanic: full mana + shield at low HP. |
| 9 | 900709 | Blink target location selection | C++ | implemented | Blink (1953) normally teleports 20yd forward. Approach: override Blink → click-to-Blink with target location. SpellScript `HandleDummy`: read SpellDestination → teleport caster there (max Range e.g. 40yd). DBC: change spell target type to `TARGET_DEST_DEST`. Needs client-side: spell shows ground target cursor. Comparable to Heroic Leap Targeting. |

### Mage — Fire (900733-900765)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900733 | Fireball +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Fireball (42833). SpellFamilyName=3, SpellFamilyFlags for Fireball. Simple damage multiplier. |
| 2 | 900734 | Fireball +9 targets | C++ | implemented | Fireball (42833) is Single-Target Projectile. SpellScript `AfterHit` → chains to 9 additional enemies within radius. CastSpell(Fireball damage helper, triggered=true). Needs helper spell (e.g. 900739). |
| 3 | 900735 | Pyroblast +9 targets | C++ | implemented | Pyroblast (42891) is Single-Target. SpellScript `AfterHit` → chains to 9 additional enemies. CastSpell(Pyro damage helper, triggered=true). Needs helper spell (e.g. 900740). |
| 4 | 900736 | Pyroblast +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Pyroblast (42891). Simple damage multiplier. |
| 5 | 900737 | Fire Blast off GCD and usable while casting | DBC/C++ | implemented | Fire Blast (42873): DBC → `StartRecoveryCategory` = 0 (off GCD). Attribute `SPELL_ATTR4_CAN_CAST_WHILE_CASTING` set. Enables Fire Blast as weave spell during a Fireball/Pyroblast cast. Comparable to retail Fire Mage design. |
| 6 | 900738 | Pyroblast now triggers Hot Streak | C++ | implemented | Hot Streak (48108) normally procs on 2 consecutive crits. Approach: Pyroblast Cast (even non-crit) → automatically apply Hot Streak buff (48108). SpellScript on Pyroblast `AfterCast` → AuraScript: ApplyAura(Hot Streak). Effect: every Pyroblast guarantees a follow-up instant Pyroblast. Extremely strong damage loop! |

### Mage — Frost (900766-900799)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900766 | Frostbolt +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Frostbolt (42842). Simple damage multiplier. |
| 2 | 900767 | Frostbolt +9 targets | C++ | implemented | Frostbolt (42842) is Single-Target. SpellScript `AfterHit` → chains to 9 additional enemies. CastSpell(Frostbolt damage helper, triggered=true). Needs helper spell (e.g. 900772). |
| 3 | 900768 | Ice Lance +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Ice Lance (42914). Simple damage multiplier. |
| 4 | 900769 | Ice Lance +9 targets | C++ | implemented | Ice Lance (42914) is Single-Target Instant. SpellScript `AfterHit` → chains to 9 additional enemies. CastSpell(Ice Lance damage helper, triggered=true). Needs helper spell (e.g. 900773). |
| 5 | 900770 | Water Elemental is permanently | DBC/C++ | implemented | Summon Water Elemental (31687) normally has 45s duration. DBC: Duration on -1 (permanently) or set to a very high value. C++: If duration is hardcoded → `OnSummon` Hook → SetDuration(0) (permanently). Similar to a hunter pet — Water Ele stays until death/dismiss. |
| 6 | 900771 | Frost Comet Shower spell | C++ | implemented | New active spell: Channel or Instant → spawns multiple Frost Comets at random positions within the target area for X seconds. Each comet = AoE frost damage + slow. Implementation: periodic trigger → SummonGameObject or CastSpell(comet impact helper) at random position within radius. Visual: Blizzard-like but with larger individual impacts. Needs helper spells + optionally a custom visual. Comparable to Meteor (Fire) but Frost-themed. |

> **Helper-Spells Mage**: 900702 (ABarr AoE) → Helper 900710. 900705 (AB +9) → Helper 900711. 900700 (Evoc Power) → Buff 900712. 900709 (Blink) → Helper 900713. 900734 (Fireball +9) → Helper 900739. 900735 (Pyro +9) → Helper 900740. 900767 (Frostbolt +9) → Helper 900772. 900769 (Ice Lance +9) → Helper 900773. 900771 (Comet Shower) → Helper 900774.

> **Particularly complex**: 900701 (Mana Regen Scaling) needs dynamic calculation per regen tick. 900709 (Blink Target Location) needs client-side ground target cursor — possibly a DBC spell target type patch needed. 900738 (Pyro → Hot Streak Loop) is a guaranteed Instant-Pyro-Chain — extremely strong burst, balancing critical. 900771 (Frost Comet Shower) is a completely new spell with custom visuals. Fire Meteor is comparably complex.

---

### Warlock — Affliction (900800-900832)

> Warlock SpellFamilyName = 5. Affliction focuses on DoT amplification and DoT spread.

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900800 | DoTs have a chance to deal shadow damage AoE | C++ | implemented | Passive proc aura: `PROC_FLAG_DONE_PERIODIC` (0x400000). `HandleProc`: When a periodic damage tick of a Warlock DoT → chance X% → CastSpell(shadow AoE helper, triggered=true) centered on the DoT target. Shadow AoE = area damage around the target. Needs an AoE helper spell (e.g. 900803). ICD recommended (e.g. 2s). |
| 2 | 900801 | Corruption +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Corruption (47813). SpellFamilyName=5, SpellFamilyFlags for Corruption. Simple damage multiplier for Periodic + Initial Damage. |
| 3 | 900802 | DoTs have a chance to spread to 2 additional targets on tick | C++ | implemented | Passive proc aura: `PROC_FLAG_DONE_PERIODIC`. `HandleProc`: When a DoT tick → chance X% → find the 2 nearest enemies within radius that do NOT have the DoT → ApplyAura(same DoT) on them. Must extract the DoT spell ID from ProcEventInfo and cast on new targets. Very strong spread mechanic — can grow exponentially! Needs possibly max target cap per cast. |

### Warlock — Demonology (900833-900865)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900833 | Killing an enemy increases Demon Form duration | C++ | implemented | Metamorphosis (47241) has 30s duration. `PlayerScript::OnKill` or `KillCredit`-Hook: When player is in Metamorphosis (HasAura 47241) → extend aura duration by X seconds (e.g. +5s per kill). `Aura::SetDuration(GetDuration() + 5000)`. No cap or with cap (e.g. max 120s). Farming sustain mechanic. |
| 2 | 900834 | Demon Form: periodic shadow AoE + self heal | C++ | implemented | Passive aura active only during Metamorphosis (47241). Periodic tick every X seconds → CastSpell(shadow AoE helper, triggered=true) around caster + CastSpell(heal helper, triggered=true) on caster. Heal = % of damage dealt or flat. Approach: AuraScript on Metamorphosis → `OnPeriodicTick` or separate periodic trigger spell that is only active when Meta aura is present. |
| 3 | 900835 | Demons have chance to spawn lesser version of themselves | C++ | implemented | Proc aura on Warlock pet: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4). `HandleProc`: chance X% → SummonCreature(lesser version NPC) at pet position. Lesser version: temporary NPC (e.g. 30s duration), reduced stats (50% HP/Damage), same NPC type but with suffix. Needs custom creature templates per pet type (Lesser Imp, Lesser Felguard, etc.). ICD recommended (e.g. 30s). |
| 4 | 900836 | Imp Firebolt +50% damage | DBC | implemented | Passive Aura on Warlock (transferred to Pet): `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Imp Firebolt (47964). Or: aura directly on pet via owner aura scaling. Simple damage multiplier. |
| 5 | 900837 | Imp Firebolt +9 targets | C++ | implemented | Imp Firebolt (47964) is Single-Target. SpellScript `AfterHit` → chains to 9 additional enemies within radius. CastSpell(Firebolt damage helper, triggered=true). Needs helper spell (e.g. 900841). Imp becomes the AoE caster. |
| 6 | 900838 | Felguard AoE unlimited targets | DBC/C++ | implemented | Felguard Cleave (47994) normally hits limited targets. DBC: remove `MaxAffectedTargets` or set very high. C++: `OnObjectAreaTargetSelect` → no target limit. |
| 7 | 900839 | Felguard +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_MOD_DAMAGE_PERCENT_DONE` +50% on Felguard (all schools). Applied via owner-to-pet aura scaling. Simple damage multiplier for all Felguard abilities. |
| 8 | 900840 | Sacrificing pet grants ALL pet bonuses | C++ | implemented | Demonic Sacrifice (18788) normally gives a buff depending on the sacrificed pet type (Imp→Fire Dmg, VW→HP Regen, etc.). Approach: SpellScript override → On sacrifice → ApplyAura for ALL pet type buffs simultaneously (Imp-Bonus + VW-Bonus + Succubus-Bonus + Felhunter-Bonus + Felguard-Bonus). Needs a list of all sacrifice buff IDs and applies all of them at once. |

### Warlock — Destruction (900866-900899)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900866 | Shadow Bolt +9 targets | C++ | implemented | Shadow Bolt (47809) is Single-Target. SpellScript `AfterHit` → chains to 9 additional enemies. CastSpell(SB damage helper, triggered=true). Needs helper spell (e.g. 900871). |
| 2 | 900867 | Shadow Bolt +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Shadow Bolt (47809). Simple damage multiplier. |
| 3 | 900868 | Chaos Bolt +50% damage | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on Chaos Bolt (59172). Simple damage multiplier. |
| 4 | 900869 | Chaos Bolt cooldown -2 sec | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_FLAT_MODIFIER` (SPELLMOD_COOLDOWN) -2000ms on Chaos Bolt (59172). Base CD 12s → 10s. Or directly via DBC: reduce `RecoveryTime`. |
| 5 | 900870 | Chaos Bolt +9 targets | C++ | implemented | Chaos Bolt (59172) is Single-Target. SpellScript `AfterHit` → chains to 9 additional enemies. CastSpell(CB damage helper, triggered=true). Chaos Bolt ignores resistances — helper spell should do the same (Attribute `SPELL_ATTR0_NO_IMMUNITIES` or similar). Needs helper spell (e.g. 900872). |

> **Helper-Spells Warlock**: 900800 (DoT AoE) → shadow AoE helper 900803. 900837 (Imp Firebolt +9) → Helper 900841. 900834 (Meta AoE) → Helper 900842. 900834 (Meta Heal) → Helper 900843. 900838 (FG Cleave) → Helper 900844. 900866 (SB +9) → Helper 900871. 900870 (CB +9) → Helper 900872. 900835 (Lesser Demons) needs custom creature templates.

> **Particularly complex**: 900802 (DoT Spread) can grow exponentially — needs target cap to protect server performance. 900835 (Lesser Demon spawn) needs custom creature templates per pet type with own AI. 900840 (Sacrifice All Bonuses) must correctly identify and stack all pet-type buffs. 900833 (Meta Duration Extension) + 900834 (Meta AoE+Heal) together turn Demo-Lock into a permanently transformed AoE healer-tank hybrid.

---

### Priest — Discipline (900900-900932)

> Priest SpellFamilyName = 6. Disc focuses on shield enhancement.

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900900 | Shields explode on breaking/fading | C++ | implemented | Power Word: Shield (48066) and other absorb shields. AuraScript `OnRemove`: When RemoveMode = EXPIRE (fade) or ENEMY_SPELL (broken via damage) → CastSpell(shield explosion helper, triggered=true) centered on the shield target. Explosion = Holy/Shadow AoE damage within radius, damage scales with remaining/absorbed shield amount. Needs an AoE helper spell (e.g. 900903). Very cool thematically — Disc becomes AoE DPS via shields. |
| 2 | 900901 | Shields +50% | DBC | implemented | Passive Aura: `SPELL_AURA_ADD_PCT_MODIFIER` +50% on PW:Shield (48066) absorb amount. SpellFamilyName=6, SpellFamilyFlags for PW:S. Increases absorb value by 50%. Stacks with existing talents (Improved PW:S, Twin Disciplines, etc.). |
| 3 | 900902 | Weakened Soul only 5 sec CD | DBC | implemented | Weakened Soul (6788) debuff normally has 15s duration. DBC: set duration to 5000ms. enables much more frequent re-shielding. Simple DBC duration change. Synergizes strongly with 900900 (Shield Explosion) — more shields = more explosions. |

### Priest — Holy (900933-900965)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900933 | Direct heals have chance to cast Holy Fire on enemies in 10y radius of target | C++ | implemented | Passive proc aura: `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS` (Positive Spell = Heal). `HandleProc`: When direct heal (Flash Heal 48071, Greater Heal 48063, etc. — no HoT) → chance X% → find all enemies within 10yd radius around the heal target → CastSpell(Holy Fire 48135, triggered=true) on each. Dual-purpose heal: healing + simultaneously AoE DPS. Needs SpellFamily filter so only direct heals trigger (not Renew/PoM). ICD recommended. |

### Priest — Shadow (900966-900999)

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 900966 | DoTs have a chance to deal shadow damage AoE | C++ | implemented | Same approach as Warlock 900800. Passive proc aura: `PROC_FLAG_DONE_PERIODIC`. `HandleProc`: When a Shadow-DoT tick (SW:Pain 48125, VT 48160, Devouring Plague 48300) → chance X% → CastSpell(shadow AoE helper, triggered=true) on the target. Needs an AoE helper spell (e.g. 900968). ICD recommended. |
| 2 | 900967 | DoTs have a chance to spread to 2 additional targets on tick | C++ | implemented | Same approach as Warlock 900802. Passive proc aura: `PROC_FLAG_DONE_PERIODIC`. `HandleProc`: DoT tick → chance X% → find the 2 nearest enemies without the DoT → ApplyAura(same DoT). Must extract the DoT spell ID from ProcEventInfo. Same warning: can grow exponentially, needs a target cap. |

> **Helper-Spells Priest**: 900900 (Shield Explosion) → Holy/shadow AoE helper 900903. 900966 (DoT AoE) → shadow AoE helper 900968.

> **Particularly complex**: 900900 (Shield Explosion) needs correct `OnRemove` detection (fade vs. dispel vs. break) and damage scaling based on shield amount. 900933 (Heal → Holy Fire AoE) is a novel dual-purpose concept — must cleanly distinguish between direct heals and HoTs. Shadow DoT mechanics (900966/900967) are identical to Warlock — code can be shared.

---

### Non-Class — Global (901100-901199)

> Global passive spells that apply to ALL classes. SpellFamilyName = 0 (Generic). Granted to all players automatically at Paragon level.

| # | Spell ID | Effect | Approach | Status | Details |
|---|----------|--------|--------|--------|---------|
| 1 | 901100 | Cast while moving | DBC/C++ | implemented | Passive Aura: Must affect all of the player's spells. Approach: `SPELL_ATTR5_CAN_CHANNEL_WHEN_MOVING` is sufficient only for channels. For all casts: C++ Hook on `Spell::CheckCast()` → Skip `SPELL_FAILED_MOVING` check when aura is active. Or: `Player::isMoving()` override. Alternatively DBC: Aura with `SPELL_AURA_CAST_WHILE_WALKING` (Aura 330, exists in WotLK DBC). Most powerful global buff — eliminates cast time weakness completely for all caster classes. |
| 2 | 901101 | Kill enemy → heal 5% total HP | C++ | implemented | Passive proc aura: `PROC_FLAG_KILL` (0x1). `HandleProc`: `GetCaster()->CastCustomSpell(heal helper, SPELLVALUE_BASE_POINT0, MaxHealth * 5 / 100, GetCaster(), triggered=true)`. Or: `GetCaster()->ModifyHealth(MaxHealth * 0.05)`. Simple on-kill heal. No ICD needed (kill events are naturally rate-limited). Needs optionally a heal helper spell (e.g. 901105) for combat log visibility. |
| 3 | 901102 | Attacks 25% chance to hit again (Extra Attack) | C++/DBC | implemented | Passive proc aura: `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` (0x4) + `PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS` (0x10). chance 25%. `HandleProc`: CastSpell(extra attack helper, triggered=true) on target — repeats the last attack. For melee: `SPELL_AURA_ADD_EXTRA_ATTACKS` (like Windfury/Sword Spec). For ranged/spell: SpellScript `AfterHit` → CastSpell(same spell, triggered=true) with 25% chance. Caution: must prevent recursive procs (extra attack does not trigger again). |
| 4 | 901103 | Spells/abilities 10% chance to hit all enemies within 10y | C++ | implemented | Passive proc aura: `PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG` + `PROC_FLAG_DONE_MELEE_AUTO_ATTACK` + `PROC_FLAG_DONE_RANGED_AUTO_ATTACK`. chance 10%. `HandleProc`: find all enemies within 10yd radius around target → CastSpell(damage helper, triggered=true) on each. Damage = same amount as the original hit. Needs ProcEventInfo → GetDamageInfo → GetDamage() for damage value. Needs damage helper spell (e.g. 901106). ICD recommended (e.g. 1s). |
| 5 | 901104 | Avoid attack → counter attack | C++ | implemented | Passive proc aura: `PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK` (0x2) with `PROC_HIT_DODGE|PROC_HIT_PARRY|PROC_HIT_BLOCK` (avoid events). `HandleProc`: When Dodge/Parry/Block → CastSpell(counter attack helper, triggered=true) on attacker. Counter attack = instant melee damage back. Comparable to Rogue Riposte or Warrior Overpower proc — but automatic and for all classes. Needs damage helper spell (e.g. 901107). |

> **Helper-Spells Non-Class**: 901101 (Kill Heal) → heal helper 901105. 901102 (Extra Attack) → Helper 901108. 901103 (AoE Proc) → damage helper 901106. 901104 (Counter Attack) → damage helper 901107.

> **Particularly complex**: 901100 (Cast While Moving) is the most powerful buff in the entire system — fundamentally changes gameplay for all casters. Must be implemented robustly (channel + cast + interruptible spells). 901102 (Extra Attack 25%) must cleanly prevent recursive procs. 901103 (10% AoE Proc) needs ICD to prevent spam on rapid DoT ticks.

---