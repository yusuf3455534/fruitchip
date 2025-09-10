#pragma once

#include <stdint.h>

// taken from https://web.archive.org/web/20241212003540/https://www.eevblog.com/forum/programming/crc32-half-bytenibble-lookup-table-algorithm/
uint32_t crc32(uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;

    static const uint32_t lut[16] = {
        0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
        0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
        0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
        0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    };

    for (uint32_t i = 0; i < len; i++)
    {
        crc ^= (uint32_t)data[i] << 24;
        crc = (crc << 4) ^ lut[crc >> 28];
        crc = (crc << 4) ^ lut[crc >> 28];
    }

    return crc;
}
