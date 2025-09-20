#include "pico/platform/sections.h"

#include "boot_rom/handler.h"
#include "boot_rom/write/idle.h"
#include "boot_rom/write/version.h"
#include "boot_rom/data_out.h"
#include <modchip/cmd.h>
#include "../../version.h"

static uint8_t counter = 0;

void __time_critical_func(handle_write_git_rev)(uint8_t w)
{
    counter++;

    switch (counter) {
        case 1: if (w != GET_BYTE(MODCHIP_CMD_GIT_REV, 3)) { goto exit; }
            boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, GIT_REV, 8, false);
exit:
        [[fallthrough]];
        default:
            write_handler = handle_write_idle;
            counter = 0;
    }
}

void __time_critical_func(prepare_handle_write_git_rev)(uint8_t w)
{
    counter = 0;
    write_handler = handle_write_git_rev;
    write_handler(w);
}
