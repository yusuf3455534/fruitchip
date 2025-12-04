#include <string.h>

#include "pico/platform/sections.h"
#include "hardware/watchdog.h"
#include <hardware/flash.h>
#include <hardware/sync.h>

#include <boot_rom/data_out.h>
#include <boot_rom/handler.h>
#include <boot_rom/write/idle.h>

#include <modchip/cmd.h>
#include <modchip/update.h>

static uint8_t partition_idx;
static uint8_t partition_idx_xor;
static uint32_t sector_idx;
static uint32_t sector_idx_xor;
static uint32_t sector_crc;
static uint32_t sector_crc_xor;
static uint32_t sector_count;
static uint32_t sector_count_xor;

static int interrupts;

bool flash_write_lock = true;

static uint32_t dma_crc32(const void *data, size_t len, uint32_t crc)
{
    uint8_t dummy;

    bool channel = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(channel);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);
    channel_config_set_sniff_enable(&cfg, true);

    dma_sniffer_enable(channel, 0, false);
    dma_hw->sniff_data = crc;

    dma_channel_configure(
        channel,
        &cfg,
        &dummy,
        data,
        len,
        true
    );

    dma_channel_wait_for_finish_blocking(channel);
    dma_channel_unclaim(channel);

    return dma_hw->sniff_data;
}

void __time_critical_func(handle_write_get_partition_size)(uint8_t w)
{
    static uint32_t response;

    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_GET_PARTITION_SIZE, 3)) { goto exit; } break;
        case 4: partition_idx = w; break;
        case 5: partition_idx_xor = w;
        {
            bool is_valid_value = (partition_idx ^ partition_idx_xor) == 0xFF && partition_idx >= 0 && partition_idx < MODCHIP_PARTITION_IDX_TOTAL;
            if (is_valid_value)
            {
                switch (partition_idx)
                {
                    case MODCHIP_PARTITION_IDX_FW: response = FW_PARTITION_SIZE_BYTES; break;
                    case MODCHIP_PARTITION_IDX_FW_UPDATE: response = UPDATE_PARTITION_SIZE_BYTES; break;
                    case MODCHIP_PARTITION_IDX_APPS: response = APPS_PARTITION_SIZE_BYTES; break;
                    default: response = 0; break;
                }

                boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &response, sizeof(response), true);
            }
            else
            {
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
            }
        }
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_get_flash_sector_size)(uint8_t w)
{
    static uint32_t response = FLASH_SECTOR_SIZE;

    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_GET_FLASH_SECTOR_SIZE, 3)) { goto exit; }
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &response, sizeof(response), true);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_write_lock)(uint8_t w)
{
    static uint8_t value = 0;
    static uint8_t value_xor = 0;

    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_WRITE_LOCK, 3)) { goto exit; } break;
        case 4: value = w; break;
        case 5: value_xor = w;
        {
            bool is_valid_value = (value ^ value_xor) == 0xFF;
            if (is_valid_value)
            {
                flash_write_lock = value;
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_OK);
            }
            else
            {
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
            }
        }
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

static void __time_critical_func(write_flash_sector)(uint8_t w)
{
    static uint8_t sector[FLASH_SECTOR_SIZE];

    cmd_byte_counter++;

    if (cmd_byte_counter >= 22 && cmd_byte_counter <= (FLASH_SECTOR_SIZE + 22 - 1))
    {
        sector[cmd_byte_counter - 22] = w;

        if (cmd_byte_counter == (FLASH_SECTOR_SIZE + 22 - 1))
        {
            boot_rom_data_out_start_busy_code();

            bool is_valid_partition_idx = (partition_idx ^ partition_idx_xor) == 0xFF;
            bool is_valid_sector_idx = (sector_idx ^ sector_idx_xor) == 0xFFFFFFFF;
            uint32_t flash_offset = 0;
            bool is_valid_addr = false;

            if (is_valid_partition_idx && is_valid_sector_idx)
            {
                uint32_t offset = sector_idx * FLASH_SECTOR_SIZE;
                switch (partition_idx)
                {
                    case MODCHIP_PARTITION_IDX_FW_UPDATE:
                        flash_offset = UPDATE_PARTITION_OFFSET + offset;
                        is_valid_addr = (flash_offset + FLASH_SECTOR_SIZE) < (UPDATE_PARTITION_OFFSET + UPDATE_PARTITION_SIZE_BYTES);
                        break;
                    case MODCHIP_PARTITION_IDX_APPS:
                        flash_offset = APPS_PARTITION_OFFSET + offset;
                        is_valid_addr = (flash_offset + FLASH_SECTOR_SIZE) < (APPS_PARTITION_OFFSET + APPS_PARTITION_SIZE_BYTES);
                        break;
                }
            }

            bool is_valid_block_crc = ((sector_crc ^ sector_crc_xor) == 0xFFFFFFFF) &&
                dma_crc32(sector, sizeof(sector), 0xFFFFFFFF) == sector_crc;

            bool is_valid = is_valid_addr && is_valid_block_crc;

            if (!flash_write_lock && is_valid)
            {
                bool is_eq = memcmp((void *)(XIP_BASE + flash_offset), sector, sizeof(sector)) == 0;

                printf("write: sector idx %i (0x%x), is eq %i\n",
                    sector_idx, flash_offset, is_eq
                );

                if (!is_eq)
                {
                    interrupts = save_and_disable_interrupts();
                    flash_range_erase(flash_offset, sizeof(sector));
                    flash_range_program(flash_offset, sector, sizeof(sector));
                    restore_interrupts(interrupts);
                }

                boot_rom_data_out_stop_busy_code(MODCHIP_CMD_RESULT_OK);
            }
            else
            {
                boot_rom_data_out_stop_busy_code(MODCHIP_CMD_RESULT_ERR);
            }

            goto exit;
        }
    }
    else
    {
exit:
        write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_write_flash_sector)(uint8_t w)
{

    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_WRITE_FLASH_SECTOR, 3)) { goto exit; } break;

        case 4: partition_idx = w; break;

        case 5: partition_idx_xor = w; break;

        case 6: sector_idx = w; break;
        case 7: sector_idx |= w << 8; break;
        case 8: sector_idx |= w << 16; break;
        case 9: sector_idx |= w << 24; break;

        case 10: sector_idx_xor = w; break;
        case 11: sector_idx_xor |= w << 8; break;
        case 12: sector_idx_xor |= w << 16; break;
        case 13: sector_idx_xor |= w << 24; break;

        case 14: sector_crc = w; break;
        case 15: sector_crc |= w << 8; break;
        case 16: sector_crc |= w << 16; break;
        case 17: sector_crc |= w << 24; break;

        case 18: sector_crc_xor = w; break;
        case 19: sector_crc_xor |= w << 8; break;
        case 20: sector_crc_xor |= w << 16; break;
        case 21: sector_crc_xor |= w << 24;
            write_handler = &write_flash_sector;
            break;

        default:
exit:
            write_handler = &handle_write_idle;
            break;
    }
}

static void __time_critical_func(erase_flash_sector)(uint8_t w)
{
    cmd_byte_counter++;

    if (cmd_byte_counter == 21)
    {
        sector_count_xor |= w << 24;

        boot_rom_data_out_start_busy_code();

        bool is_valid_partition_idx = (partition_idx ^ partition_idx_xor) == 0xFF;
        bool is_valid_sector_idx = (sector_idx ^ sector_idx_xor) == 0xFFFFFFFF;
        uint32_t flash_offset = 0;
        bool is_valid_addr = false;

        if (is_valid_partition_idx && is_valid_sector_idx)
        {
            uint32_t offset = sector_idx * FLASH_SECTOR_SIZE;
            switch (partition_idx)
            {
                case MODCHIP_PARTITION_IDX_FW_UPDATE:
                    flash_offset = UPDATE_PARTITION_OFFSET + offset;
                    is_valid_addr = (flash_offset + FLASH_SECTOR_SIZE) < (UPDATE_PARTITION_OFFSET + UPDATE_PARTITION_SIZE_BYTES);
                    break;
                case MODCHIP_PARTITION_IDX_APPS:
                    flash_offset = APPS_PARTITION_OFFSET + offset;
                    is_valid_addr = (flash_offset + FLASH_SECTOR_SIZE) < (APPS_PARTITION_OFFSET + APPS_PARTITION_SIZE_BYTES);
                    break;
            }
        }

        bool is_valid_sector_count = (sector_count ^ sector_count_xor) == 0xFFFFFFFF;
        bool is_valid = is_valid_addr && is_valid_sector_count;

        if (!flash_write_lock && is_valid)
        {
            size_t sectors_size = sector_count * FLASH_SECTOR_SIZE;

            bool is_sector_empty = true;
            for (unsigned int i = 0; i < FLASH_SECTOR_SIZE; i++)
            {
                uint8_t *flash_addr = (uint8_t*)(XIP_BASE + flash_offset + i);
                uint8_t byte = *flash_addr;
                if (byte != 0xFF)
                {
                    is_sector_empty = false;
                    break;
                }
            }

            printf("erase: %u bytes at sector idx %i (0x%x), is empty %i\n",
                sectors_size, sector_idx, flash_offset, is_sector_empty
            );

            if (!is_sector_empty)
            {
                interrupts = save_and_disable_interrupts();
                flash_range_erase(
                    flash_offset,
                    sectors_size
                );
                restore_interrupts(interrupts);
            }

            boot_rom_data_out_stop_busy_code(MODCHIP_CMD_RESULT_OK);
        }
        else
        {
            boot_rom_data_out_stop_busy_code(MODCHIP_CMD_RESULT_ERR);
        }
    }

    write_handler = &handle_write_idle;
}

void __time_critical_func(handle_write_erase_flash_sector)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_ERASE_FLASH_SECTOR, 3)) { goto exit; } break;

        case 4: partition_idx = w; break;

        case 5: partition_idx_xor = w; break;

        case 6: sector_idx = w; break;
        case 7: sector_idx |= w << 8; break;
        case 8: sector_idx |= w << 16; break;
        case 9: sector_idx |= w << 24; break;

        case 10: sector_idx_xor = w; break;
        case 11: sector_idx_xor |= w << 8; break;
        case 12: sector_idx_xor |= w << 16; break;
        case 13: sector_idx_xor |= w << 24; break;

        case 14: sector_count = w; break;
        case 15: sector_count |= w << 8; break;
        case 16: sector_count |= w << 16; break;
        case 17: sector_count |= w << 24; break;

        case 18: sector_count_xor = w; break;
        case 19: sector_count_xor |= w << 8; break;
        case 20: sector_count_xor |= w << 16;
            write_handler = &erase_flash_sector;
            break;

        default:
exit:
            write_handler = &handle_write_idle;
            break;
    }
}

void __time_critical_func(handle_write_reboot)(uint8_t w)
{
    static enum modchip_reboot_mode mode;
    static uint8_t mode_xor;

    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_REBOOT, 3)) { goto exit; } break;
        case 4: mode = w; break;
        case 5: mode_xor = w;
        {
            bool is_valid_value = (mode ^ mode_xor) == 0xFF && mode >= 0 && mode < MODCHIP_REBOOT_MODE_TOTAL;
            if (is_valid_value)
            {
                if (mode == MODCHIP_REBOOT_MODE_UPDATE) watchdog_hw->scratch[0] = MODCHIP_UPDATE_MAGIC;
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_OK);

                printf("rebooting, good night\n");
                watchdog_reboot(0, 0, 100);
            }
            else
            {
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
            }
        }
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}
