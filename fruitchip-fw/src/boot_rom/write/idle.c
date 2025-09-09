#include "pico/platform/sections.h"

#include <boot_rom/handler.h>
#include <stdio.h>
#include "idle.h"
#include "disable_next_osdsys_hook.h"
#include "get_payload.h"
#include "read_app.h"

void __time_critical_func(handle_write_idle)(uint8_t w)
{
    printf("w %x\n", w);
    return;

    switch (w)
    {
        case 0x6D: write_handler = &handle_write_disable_next_osdsys_hook; break; // 0x6d5a611c
        case 0x2F: write_handler = &handle_write_get_ee_stage1_size; break; // 0x2f5c08d0
        case 0x6B: write_handler = &handle_write_get_ee_stage1; break; // 0x6b6f2aef
        case 0x45: write_handler = &handle_write_get_ee_stage2_size; break; // 0x451e6ec4
        case 0x15: write_handler = &handle_write_get_ee_stage2; break; // 0x15232c88
        case 0x78: write_handler = &handle_write_read_apps_partition; break; // 0x78226c0a
    }
}
