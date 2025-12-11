#include "pico/platform/sections.h"

#include "boot_rom/handler.h"
#include "boot_rom/write/idle.h"
#include "boot_rom/write/version.h"
#include "boot_rom/data_out.h"
#include <modchip/cmd.h>
#include "git_version.h"

void __time_critical_func(handle_write_pin_config)(uint8_t w)
{
    cmd_byte_counter++;

    static uint8_t idx;
    static uint8_t idx_xor;
    static uint32_t mask;

    switch (cmd_byte_counter)
    {
        case 3: if (w != GET_BYTE(MODCHIP_CMD_PIN_CONFIG, 3)) { goto exit; } break;
        case 4: idx = w; break;
        case 5: idx_xor = w;
            if ((idx ^ idx_xor) == 0xFF)
            {
                switch (idx)
                {
                    case MODCHIP_PIN_BOOT_ROM_DATA:
                        mask =  1 << (BOOT_ROM_Q0_PIN + 0) |
                                1 << (BOOT_ROM_Q0_PIN + 1) |
                                1 << (BOOT_ROM_Q0_PIN + 2) |
                                1 << (BOOT_ROM_Q0_PIN + 3) |
                                1 << (BOOT_ROM_Q0_PIN + 4) |
                                1 << (BOOT_ROM_Q0_PIN + 5) |
                                1 << (BOOT_ROM_Q0_PIN + 6) |
                                1 << (BOOT_ROM_Q0_PIN + 7);
                        break;
                    case MODCHIP_PIN_BOOT_ROM_CE:
                        mask = 1 << BOOT_ROM_CE_PIN;
                        break;
                    case MODCHIP_PIN_BOOT_ROM_OE:
                        mask = 1 << BOOT_ROM_OE_PIN;
                        break;
                    case MODCHIP_PIN_RESET:
                        mask = 1 << RST_PIN;
                        break;
                    default:
                        boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
                        goto exit;
                }

                boot_rom_data_out_start_data_with_status_code(MODCHIP_CMD_RESULT_OK, &mask, sizeof(mask), true);
            }
            else
            {
                boot_rom_data_out_start_status_code(MODCHIP_CMD_RESULT_ERR);
            }
exit:
        [[fallthrough]];
        default:
            write_handler = handle_write_idle;
    }
}
