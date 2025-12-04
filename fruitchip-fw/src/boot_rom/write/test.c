#include "pico/platform/sections.h"

#include "boot_rom/handler.h"
#include "boot_rom/write/idle.h"
#include "boot_rom/write/version.h"
#include "boot_rom/data_out.h"
#include <modchip/cmd.h>

static uint8_t placeholder[15] = {
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

void __time_critical_func(handle_write_test_data_out_with_status_no_data_no_crc)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_NO_DATA_NO_CRC, 3)) { goto exit; }
            boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_OK);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_test_data_out_with_status_with_data_no_crc)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_WITH_DATA_NO_CRC, 3)) { goto exit; }
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, placeholder, sizeof(placeholder), false);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_test_data_out_with_status_with_data_with_crc)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_TEST_DATA_OUT_WITH_STATUS_WITH_DATA_WITH_CRC, 3)) { goto exit; }
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, placeholder, sizeof(placeholder), true);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_test_data_out_no_status_with_data_with_crc)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_TEST_DATA_OUT_NO_STATUS_WITH_DATA_WITH_CRC, 3)) { goto exit; }
            boot_rom_data_out_set_data(placeholder, sizeof(placeholder));
            boot_rom_data_out_start_data_without_status_code(true);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_test_data_out_no_status_with_data_no_crc)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_TEST_DATA_OUT_NO_STATUS_WITH_DATA_NO_CRC, 3)) { goto exit; }
            boot_rom_data_out_set_data(placeholder, sizeof(placeholder));
            boot_rom_data_out_start_data_without_status_code(false);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}

void __time_critical_func(handle_write_test_data_out_busy_code)(uint8_t w)
{
    cmd_byte_counter++;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_TEST_DATA_OUT_BUSY, 3)) { goto exit; }
            boot_rom_data_out_start_busy_code();
            boot_rom_data_out_stop_busy_code(MODCHIP_CMD_RESULT_OK);
exit:
        [[fallthrough]];
        default:
            write_handler = &handle_write_idle;
    }
}
