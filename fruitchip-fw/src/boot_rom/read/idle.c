#include "pico/platform/sections.h"

#include <boot_rom/handler.h>
#include "idle.h"
#include "osdsys.h"

void __time_critical_func(handle_read_idle)(uint8_t r)
{
    switch (r)
    {
        case 0x4F: read_handler = handle_read_find_osdsys_size; break;
    }
}
