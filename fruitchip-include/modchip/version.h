#pragma once

#include <tamtypes.h>

#include "modchip/io.h"
#include "modchip/cmd.h"
#include "crc32.h"

inline static bool modchip_fw_git_rev(char *dst)
{
    modchip_poke_u32(MODCHIP_CMD_FW_GIT_REV);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    modchip_peek_n(dst, 8);

    u32 crc_expected = modchip_peek_u32();
    u32 crc_actual = crc32((void *)dst, 8);
    if (crc_expected != crc_actual)
        return false;

    return true;
}

inline static bool modchip_fw_git_rev_with_retry(char *dst, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_fw_git_rev(dst);
        if (ret)
            break;
    }

    return ret;
}

inline static bool modchip_bootloader_git_rev(char *dst)
{
    modchip_poke_u32(MODCHIP_CMD_BOOTLOADER_GIT_REV);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    modchip_peek_n(dst, 8);

    u32 crc_expected = modchip_peek_u32();
    u32 crc_actual = crc32((void *)dst, 8);
    if (crc_expected != crc_actual)
        return false;

    return true;
}

inline static bool modchip_bootloader_git_rev_with_retry(char *dst, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_bootloader_git_rev(dst);
        if (ret)
            break;
    }

    return ret;
}