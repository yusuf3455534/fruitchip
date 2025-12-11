#pragma once

#include "modchip/io.h"
#include "modchip/cmd.h"
#include "crc32.h"

inline static bool modchip_get_partition_size(u8 idx, u32 *flash_size)
{
    modchip_poke_u32(MODCHIP_CMD_GET_PARTITION_SIZE);
    modchip_poke_u8(idx);
    modchip_poke_u8(idx ^ 0xFF);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    *flash_size = modchip_peek_u32();

    u32 crc_expected = modchip_peek_u32();
    u32 crc_actual = crc32((uint8_t *)flash_size, sizeof(*flash_size));
    return crc_expected == crc_actual;
}

inline static bool modchip_get_partition_size_with_retry(u8 idx, u32 *flash_size, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_get_partition_size(idx, flash_size);
        if (ret)
            break;
    }

    return ret;
}

inline static bool modchip_get_flash_sector_size(u32 *sector_size)
{
    modchip_poke_u32(MODCHIP_CMD_GET_FLASH_SECTOR_SIZE);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    *sector_size = modchip_peek_u32();

    u32 crc_expected = modchip_peek_u32();
    u32 crc_actual = crc32((uint8_t *)sector_size, sizeof(*sector_size));
    return crc_expected == crc_actual;
}

inline static bool modchip_get_flash_sector_size_with_retry(u32 *sector_size, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_get_flash_sector_size(sector_size);
        if (ret)
            break;
    }

    return ret;
}

inline static bool modchip_set_write_lock(bool lock)
{
    modchip_poke_u32(MODCHIP_CMD_WRITE_LOCK);
    modchip_poke_u8(lock);
    modchip_poke_u8(lock ^ 0xFF);

    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    return true;
}

inline static bool modchip_set_write_lock_with_retry(bool lock, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_set_write_lock(lock);
        if (ret)
            break;
    }

    return ret;
}

inline static bool modchip_write_flash_sector(enum modchip_partition partition_idx, u32 sector_idx, u8 *sector, u32 sector_size)
{
    u32 sector_crc = crc32(sector, sector_size);

    modchip_poke_u32(MODCHIP_CMD_WRITE_FLASH_SECTOR);
    modchip_poke_u8(partition_idx);
    modchip_poke_u8(partition_idx ^ 0xFF);
    modchip_poke_u32(sector_idx);
    modchip_poke_u32(sector_idx ^ 0xFFFFFFFF);
    modchip_poke_u32(sector_crc);
    modchip_poke_u32(sector_crc ^ 0xFFFFFFFF);
    for (u32 i = 0; i < sector_size; i++)
    {
        modchip_poke_u8(sector[i]);
    }

    while (true)
    {
        int r = modchip_peek_u32();
        switch (r)
        {
            case MODCHIP_CMD_RESULT_BUSY:
                continue;
            case MODCHIP_CMD_RESULT_OK:
                return true;
            case MODCHIP_CMD_RESULT_ERR:
            default:
                return false;
        }
    }
}

inline static bool modchip_write_flash_sector_with_retry(enum modchip_partition partition_idx, u32 sector_idx, u8 *sector, u32 sector_size, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_write_flash_sector(partition_idx, sector_idx, sector, sector_size);
        if (ret)
            break;
    }

    return ret;
}

inline static bool modchip_erase_flash_sector(enum modchip_partition partition_idx, u32 sector_idx, u32 sector_count)
{
    modchip_poke_u32(MODCHIP_CMD_ERASE_FLASH_SECTOR);
    modchip_poke_u8(partition_idx);
    modchip_poke_u8(partition_idx ^ 0xFF);
    modchip_poke_u32(sector_idx);
    modchip_poke_u32(sector_idx ^ 0xFFFFFFFF);
    modchip_poke_u32(sector_count);
    modchip_poke_u32(sector_count ^ 0xFFFFFFFF);

    while (true)
    {
        int r = modchip_peek_u32();
        switch (r)
        {
            case MODCHIP_CMD_RESULT_BUSY:
                continue;
            case MODCHIP_CMD_RESULT_OK:
                return true;
            case MODCHIP_CMD_RESULT_ERR:
            default:
                return false;
        }
    }
}

inline static bool modchip_erase_flash_sector_with_retry(enum modchip_partition partition_idx, u32 sector_idx, u32 sector_count, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_erase_flash_sector(partition_idx, sector_idx, sector_count);
        if (ret)
            break;
    }

    return ret;
}

inline static bool modchip_reboot(enum modchip_reboot_mode mode)
{
    modchip_poke_u32(MODCHIP_CMD_REBOOT);
    modchip_poke_u8(mode);
    modchip_poke_u8(mode ^ 0xFF);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    return true;
}

inline static bool modchip_reboot_with_retry(enum modchip_reboot_mode mode, u8 attemps)
{
    bool ret;

    for (u8 attemp = 0; attemp < attemps; attemp++)
    {
        ret = modchip_reboot(mode);
        if (ret)
            break;
    }

    return ret;
}

inline static bool modchip_ping()
{
    modchip_poke_u32(MODCHIP_CMD_PING);
    if (modchip_peek_u32() != MODCHIP_CMD_RESULT_OK)
        return false;

    return true;
}