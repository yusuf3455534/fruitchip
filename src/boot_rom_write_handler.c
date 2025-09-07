#include <stdio.h>

#include "boot_rom_write_handler.h"
#include "boot_rom_data_out.h"
#include "apps.h"

// ELF loader that launches stage 3 ELF
const uint8_t __in_flash("ee_stage2") LOADER_EE_STAGE_2[] = {
    #embed "../loader/ee-stage2/bin/ee-stage2.bin"
};

#define RESULT_OK 0x56276fb7
#define RESULT_ERR 0x0b4f7d09

void handle_write_idle(uint8_t w);

void (*write_handler)(uint8_t) = &handle_write_idle;

void __time_critical_func(handle_write_payload_ee_stage3)(uint8_t w)
{
    if (w == 0xCC) boot_rom_data_out_start_data_without_status_code(apps_partition_entry_addr(1) + 8, apps_partition_entry_length(1));
    write_handler = &handle_write_idle;
}

void __time_critical_func(handle_write_payload_ee_stage2)(uint8_t w)
{
    if (w == 0xCB) boot_rom_data_out_start_data_without_status_code(LOADER_EE_STAGE_2, sizeof(LOADER_EE_STAGE_2));
    write_handler = &handle_write_idle;
}

void __time_critical_func(handle_write_22)(uint8_t w)
{
    static uint8_t counter = 0;
    static uint32_t read_offset = 0;
    static uint32_t read_size = 0;

    counter += 1;

    switch (counter) {
        case 1: read_offset = w; break;
        case 2: read_offset |= w << 8; break;
        case 3: read_offset |= w << 16; break;
        case 4: read_offset |= w << 24; break;

        case 5: read_size = w; break;
        case 6: read_size |= w << 8; break;
        case 7: read_size |= w << 16; break;
        case 8: read_size |= w << 24; break;

        case 9:
            if (apps_partition_entries_count() > w && apps_partition_entry_length(w)  > read_offset)
            {
                uint8_t *addr = apps_partition_entry_addr(w) + read_offset;
                boot_rom_data_out_start_data_with_status_code(RESULT_OK, addr, read_size);
            }
            else
            {
                boot_rom_data_out_start_status_code(RESULT_ERR);
            }
            [[fallthrough]];

        default:
            write_handler = &handle_write_idle;
            counter = 0;
    }

}

void __time_critical_func(handle_write_78)(uint8_t w)
{
    if (w == 0x22) write_handler = &handle_write_22;
    else write_handler = &handle_write_idle;
}

void __time_critical_func(handle_write_idle)(uint8_t w)
{
    switch (w)
    {
        case 0xC0: write_handler = &handle_write_payload_ee_stage2; break;
        case 0xC1: write_handler = &handle_write_payload_ee_stage3; break;
        case 0x78: write_handler = &handle_write_78; break;
    }
}
