#include "pico/platform/sections.h"

#include "disable_next_osdsys_hook.h"
#include <boot_rom/handler.h>
#include <boot_rom/data_out.h>
#include "idle.h"
#include <modchip/errno.h>

bool disable_next_osdsys_hook = false;

void __time_critical_func(handle_write_disable_next_osdsys_hook)(uint8_t w)
{
    static uint8_t counter = 0;
    counter++;

    if (counter == 1 && w == 0x5a) {}
    else if (counter == 2 && w == 0x61) {}
    else if (counter == 3 && w == 0x1c)
    {
        disable_next_osdsys_hook = true;
        boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_OK);
    }
    else
    {
        write_handler = &handle_write_idle;
        counter = 0;
    }
}
