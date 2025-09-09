#include "pico/platform/sections.h"

#include "get_payload.h"
#include <boot_rom/handler.h>
#include <boot_rom/data_out.h>
#include <boot_rom/loader.h>
#include <modchip/errno.h>
#include "idle.h"

static uint8_t counter = 0;

void __time_critical_func(handle_write_get_ee_stage1_size)(uint8_t w)
{
    counter++;

    if (counter == 1 && w == 0x5c) {}
    else if (counter == 2 && w == 0x08) {}
    else if (counter == 3 && w == 0xd0)
    {
        boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &LOADER_EE_STAGE_1_SIZE, sizeof(LOADER_EE_STAGE_1_SIZE), false);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}

void __time_critical_func(handle_write_get_ee_stage1)(uint8_t w)
{
    counter++;

    if (counter == 1 && w == 0x6f) {}
    else if (counter == 2 && w == 0x2a) {}
    else if (counter == 3 && w == 0xef)
    {
        boot_rom_data_out_start_data_without_status_code(LOADER_EE_STAGE_1, LOADER_EE_STAGE_1_SIZE, false);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}

void __time_critical_func(handle_write_get_ee_stage2_size)(uint8_t w)
{
    counter++;

    if (counter == 1 && w == 0x1e) {}
    else if (counter == 2 && w == 0x6e) {}
    else if (counter == 3 && w == 0xc4)
    {
        boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &LOADER_EE_STAGE_2_SIZE, sizeof(LOADER_EE_STAGE_2_SIZE), false);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}

void __time_critical_func(handle_write_get_ee_stage2)(uint8_t w)
{
    counter++;

    if (counter == 1 && w == 0x23) {}
    else if (counter == 2 && w == 0x2c) {}
    else if (counter == 3 && w == 0x88)
    {
        boot_rom_data_out_start_data_without_status_code(LOADER_EE_STAGE_2, LOADER_EE_STAGE_2_SIZE, false);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}
