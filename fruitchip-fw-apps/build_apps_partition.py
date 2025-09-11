from pathlib import Path
import struct
import enum
import argparse


MAGIC1 = B'APPS'
MAGIC2 = 0x4FB80AB4
VERSION = 1


class Attribute(enum.Flag):
    DISABLE_NEXT_OSDSYS_HOOK = enum.auto()


apps = [
    ('PS2BBL',  Path('../ps2bbl/bin/COMPRESSED_PS2BBL.ELF'),            Attribute(0)),
]


crc32_lut = [
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
]


def crc32(data):
    crc = 0xFFFFFFFF

    for i in range(len(data)):
        crc ^= data[i] << 24
        crc = (crc << 4) ^ crc32_lut[crc >> 28]
        crc &= 0xFFFFFFFF
        crc = (crc << 4) ^ crc32_lut[crc >> 28]
        crc &= 0xFFFFFFFF

    return crc


def pack_entry(data: bytes, attrs: int):
    entry = bytearray()
    entry += struct.pack('<II', len(data), attrs)
    entry += data
    entry += struct.pack('<I', crc32(data))

    return entry


def main(out_path: Path):
    entries = []

    app_names = bytearray()
    for (name, _, _) in apps:
        name_encoded = name.encode('ascii')
        app_name = struct.pack(f'< B {len(name_encoded)}s', len(name_encoded), name_encoded)
        app_names += app_name
    app_names = struct.pack('<B', len(apps)) + app_names
    entries.append(pack_entry(app_names, 0))

    for (_, path, attrs) in apps:
        app_data = path.read_bytes()
        entries.append(pack_entry(app_data, attrs.value))

    hdr = struct.pack('< 4s I I I', MAGIC1, MAGIC2, VERSION, len(entries))

    lookup_table_len = 2 * 4 * len(entries)
    offset = len(hdr) + lookup_table_len
    lookup_table = bytearray()
    for entry in entries:
        lookup_table += struct.pack('<II', offset, len(entry))
        offset += len(entry)

    with open(out_path, 'wb') as out:
        out.write(hdr)
        out.write(lookup_table)
        for entry in entries:
            out.write(entry)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('out_path', type=Path)
    args = parser.parse_args()

    main(args.out_path)
