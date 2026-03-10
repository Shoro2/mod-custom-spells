#!/usr/bin/env python3
"""
Extract custom spells (ID >= 900000) from a WotLK Spell.dbc file.

Usage:
    python3 extract_custom_spells.py <Spell.dbc> [output.dbc]

If output.dbc is given, writes a standalone DBC containing only the custom spells.
Otherwise prints spell data to stdout.

Handles corrupted DBC files by scanning for spell IDs at all 4-byte boundaries
within the data area and validating record structure before accepting matches.
"""

import struct
import sys
import os

HEADER_SIZE = 20
FIELD_COUNT = 234
RECORD_SIZE = 936  # 234 fields * 4 bytes

# Key field indices (0-based, uint32 positions)
FIELDS = {
    'ID': 0, 'Attributes': 4, 'AttributesEx': 5, 'AttributesEx2': 6, 'AttributesEx3': 7,
    'CastingTimeIndex': 28,
    'ProcTypeMask': 34, 'ProcChance': 35, 'ProcCharges': 36,
    'DurationIndex': 40, 'RangeIndex': 46, 'StackAmount': 49,
    'Effect_1': 71, 'Effect_2': 72, 'Effect_3': 73,
    'EffectBasePoints_1': 80, 'EffectBasePoints_2': 81, 'EffectBasePoints_3': 82,
    'ImplicitTargetA_1': 86, 'ImplicitTargetA_2': 87, 'ImplicitTargetA_3': 88,
    'EffectAura_1': 95, 'EffectAura_2': 96, 'EffectAura_3': 97,
    'EffectMiscValue_1': 110, 'EffectMiscValue_2': 111, 'EffectMiscValue_3': 112,
    'EffectTriggerSpell_1': 116, 'EffectTriggerSpell_2': 117, 'EffectTriggerSpell_3': 118,
    'EffectSpellClassMaskA_1': 122, 'EffectSpellClassMaskA_2': 123,
    'EffectSpellClassMaskB_1': 125, 'EffectSpellClassMaskB_2': 126,
    'SpellClassSet': 208,
    'SpellClassMask_1': 209, 'SpellClassMask_2': 210, 'SpellClassMask_3': 211,
    'DmgClass': 213, 'SchoolMask': 225,
    'Name_enUS': 136,
}

AURA_NAMES = {
    0: 'NONE', 3: 'DUMMY', 42: 'PROC_TRIGGER_SPELL',
    107: 'FLAT_MODIFIER', 108: 'ADD_PCT_MODIFIER', 262: 'MOD_MECHANIC_DMG_TAKEN_PCT',
}

EFFECT_NAMES = {
    0: 'NONE', 2: 'SCHOOL_DAMAGE', 3: 'DUMMY', 6: 'APPLY_AURA',
    10: 'HEAL', 64: 'TRIGGER_SPELL', 121: 'NORMALIZED_WEAPON_DMG',
}


def validate_spell_record(fields_u):
    """Heuristic: check if a 936-byte block looks like a valid spell record."""
    spell_id = fields_u[0]
    # ID must be in custom range
    if spell_id < 900000 or spell_id > 999999:
        return False
    # Effects must be valid (0-200 range)
    for i in [71, 72, 73]:
        if fields_u[i] > 200:
            return False
    # Auras must be valid (0-400 range)
    for i in [95, 96, 97]:
        if fields_u[i] > 400:
            return False
    # RangeIndex should be small
    if fields_u[46] > 200:
        return False
    # ProcChance should be 0-101
    if fields_u[35] > 101:
        return False
    return True


def scan_for_custom_spells(filepath, min_id=900000, max_id=999999):
    """Scan DBC file for custom spells, handling misaligned records."""
    file_size = os.path.getsize(filepath)

    with open(filepath, 'rb') as f:
        magic = f.read(4)
        if magic != b'WDBC':
            raise ValueError(f"Not a valid DBC file (magic: {magic})")

        record_count, field_count, record_size, string_block_size_hdr = struct.unpack('<IIII', f.read(16))
        print(f"DBC header: {record_count} records, {field_count} fields, {record_size} bytes/record", file=sys.stderr)

        # Calculate corrected string block
        data_end = HEADER_SIZE + record_count * record_size
        actual_sbs = file_size - data_end
        if actual_sbs != string_block_size_hdr:
            print(f"[WARN] String block mismatch: header={string_block_size_hdr} actual={actual_sbs}", file=sys.stderr)

        # Read string block (use the corrected offset)
        f.seek(data_end)
        string_block = f.read()

        # First try: read records at normal boundaries
        spells = {}
        f.seek(HEADER_SIZE)
        data = f.read(data_end - HEADER_SIZE) if data_end > HEADER_SIZE else b''

        # Try standard record-aligned reading
        for i in range(record_count):
            offset = i * record_size
            if offset + record_size > len(data):
                break
            record = data[offset:offset + record_size]
            spell_id = struct.unpack('<I', record[:4])[0]
            if min_id <= spell_id <= max_id:
                fields_u = struct.unpack('<' + 'I' * FIELD_COUNT, record)
                if validate_spell_record(fields_u):
                    fields_s = struct.unpack('<' + 'i' * FIELD_COUNT, record)
                    spells[spell_id] = (fields_u, fields_s, record)

        if not spells:
            # Fallback: scan at every 4-byte boundary in the data area
            print("[INFO] No spells found at record boundaries, scanning byte-by-byte...", file=sys.stderr)
            target_bytes = {}
            for sid in range(min_id, max_id + 1):
                target_bytes[struct.pack('<I', sid)] = sid

            for pos in range(0, len(data) - record_size, 4):
                id_bytes = data[pos:pos + 4]
                if id_bytes in target_bytes:
                    spell_id = target_bytes[id_bytes]
                    if spell_id in spells:
                        continue
                    record = data[pos:pos + record_size]
                    if len(record) < record_size:
                        break
                    fields_u = struct.unpack('<' + 'I' * FIELD_COUNT, record)
                    if validate_spell_record(fields_u):
                        fields_s = struct.unpack('<' + 'i' * FIELD_COUNT, record)
                        spells[spell_id] = (fields_u, fields_s, record)
                        print(f"  Found spell {spell_id} at data offset {pos} (misaligned by {pos % record_size})", file=sys.stderr)

        return spells, string_block


def get_string(string_block, offset):
    if offset == 0 or offset >= len(string_block):
        return ""
    try:
        end = string_block.index(b'\x00', offset)
        return string_block[offset:end].decode('utf-8', errors='replace')
    except ValueError:
        return ""


def print_spell(spell_id, u, s, string_block):
    """Pretty-print a spell's key fields."""
    name = get_string(string_block, u[136])
    print(f"\n{'='*60}")
    print(f"Spell {spell_id}: {name}")
    print(f"{'='*60}")
    print(f"  Attributes:    {u[4]:#010x}  Ex: {u[5]:#010x}  Ex2: {u[6]:#010x}  Ex3: {u[7]:#010x}")
    print(f"  ProcTypeMask:  {u[34]:#010x}  Chance: {u[35]}  Charges: {u[36]}")
    print(f"  Duration: {u[40]}  Stacks: {u[49]}  Range: {u[46]}")

    for eff_idx in range(3):
        eff = u[71 + eff_idx]
        aura = u[95 + eff_idx]
        bp = s[80 + eff_idx]
        tgt = u[86 + eff_idx]
        misc = s[110 + eff_idx]
        trigger = u[116 + eff_idx]

        if eff == 0 and aura == 0:
            continue

        eff_name = EFFECT_NAMES.get(eff, f'UNK_{eff}')
        aura_name = AURA_NAMES.get(aura, f'UNK_{aura}')

        print(f"\n  Effect[{eff_idx}]: {eff_name} ({eff})")
        if aura:
            print(f"    Aura: {aura_name} ({aura})")
        if bp:
            print(f"    BasePoints: {bp}")
        if tgt:
            print(f"    TargetA: {tgt}")
        if misc:
            print(f"    MiscValue: {misc}")
        if trigger:
            print(f"    TriggerSpell: {trigger}")

        mask_a = u[122 + eff_idx]
        mask_b = u[125 + eff_idx]
        mask_c = u[128 + eff_idx]
        if mask_a or mask_b or mask_c:
            print(f"    EffectClassMask: [{mask_a:#x}, {mask_b:#x}, {mask_c:#x}]")

    print(f"\n  SpellFamily: {u[208]}  FamilyMask: [{u[209]:#x}, {u[210]:#x}, {u[211]:#x}]")
    print(f"  DmgClass: {u[213]}  SchoolMask: {u[225]}")


def write_custom_dbc(output_path, spells, source_string_block):
    """Write a standalone DBC with only custom spells."""
    new_strings = bytearray(b'\x00')
    string_map = {0: 0}

    new_records = []
    for spell_id in sorted(spells.keys()):
        u, s, raw_record = spells[spell_id]
        record_ints = list(struct.unpack('<' + 'I' * FIELD_COUNT, raw_record))

        # Remap string offsets
        string_ranges = [(136, 152), (153, 169), (170, 186), (187, 203)]
        for start, mask_idx in string_ranges:
            for fi in range(start, mask_idx):
                old_offset = record_ints[fi]
                if old_offset == 0:
                    continue
                if old_offset not in string_map:
                    s_val = get_string(source_string_block, old_offset)
                    if s_val:
                        new_offset = len(new_strings)
                        string_map[old_offset] = new_offset
                        new_strings.extend(s_val.encode('utf-8') + b'\x00')
                    else:
                        string_map[old_offset] = 0
                record_ints[fi] = string_map.get(old_offset, 0)

        new_records.append(struct.pack('<' + 'I' * FIELD_COUNT, *record_ints))

    string_block = bytes(new_strings)

    with open(output_path, 'wb') as f:
        f.write(b'WDBC')
        f.write(struct.pack('<IIII', len(new_records), FIELD_COUNT, RECORD_SIZE, len(string_block)))
        for rec in new_records:
            f.write(rec)
        f.write(string_block)

    print(f"\nWrote {len(new_records)} spells to {output_path} ({os.path.getsize(output_path)} bytes)")


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <Spell.dbc> [output.dbc]")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else None

    spells, string_block = scan_for_custom_spells(input_path)

    if not spells:
        print("No custom spells (ID >= 900000) found.")
        sys.exit(0)

    print(f"\nFound {len(spells)} custom spell(s):")
    for spell_id in sorted(spells.keys()):
        u, s, _ = spells[spell_id]
        print_spell(spell_id, u, s, string_block)

    if output_path:
        write_custom_dbc(output_path, spells, string_block)


if __name__ == '__main__':
    main()
