#pragma once

#include "modchip/io.h"
#include "modchip/cmd.h"
#include "crc32.h"

inline static u32 modchip_pin_config(enum modchip_pin pin)
{
    modchip_poke_u32(MODCHIP_CMD_PIN_CONFIG);
    modchip_poke_u8(pin);
    modchip_poke_u8(pin ^ 0xFF);

    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return 0;

    u32 mask = modchip_peek_u32();

    u32 crc_expected = modchip_peek_u32();
    u32 crc_actual = crc32((u8 *)&mask, sizeof(mask));
    if (crc_expected != crc_actual)
        return 0;

    return mask;
}
