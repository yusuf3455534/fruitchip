#include <string.h>

#include <kernel.h>
#include <sio.h>

#include <modchip/io.h>
#include "load_elf.h"

#define USER_MEM_START_ADDR 0x100000
#define USER_MEM_END_ADDR 0x2000000

DISABLE_PATCHED_FUNCTIONS();

u32 crc32(const u8* src, u32 src_len)
{
    u32 crc = 0xffffffff;

    for (u32 i = 0; i < src_len; i++)
    {
        crc = crc ^ src[i];

        for (u8 j = 8; j > 0; --j)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }

    return crc ^ 0xffffffff;
}

void __attribute__((section(".entry"))) __start()
{
    sio_puts("EE2: clear");
    memset((void *)USER_MEM_START_ADDR, 0, USER_MEM_END_ADDR - USER_MEM_START_ADDR);

    sio_puts("EE2: read size");
    u32 size;
    u32 r = modchip_apps_read(0, sizeof(size), 1, &size);
    if (r != MODCHIP_CMD_RESULT_OK)
        sio_puts("EE2: read size err");

    sio_puts("EE2: read");
    r = modchip_apps_read(8, size, 1, (void *)EE_STAGE_3_ADDR);
    if (r != MODCHIP_CMD_RESULT_OK)
        sio_puts("EE2: read err");

    sio_puts("EE2: crc");
    u32 expected_crc;
    r = modchip_apps_read(8 + size, sizeof(expected_crc), 1, &expected_crc);
    if (r != MODCHIP_CMD_RESULT_OK)
        sio_puts("EE2: crc err");

    u32 crc = crc32((void *)EE_STAGE_3_ADDR, size);
    if (crc == expected_crc)
        sio_puts("EE2: checksum ok");
    else
        sio_puts("EE2: checksum bad");

    sio_puts("EE2: init");
    _InitSys();

    sio_puts("EE2: exec");
    static char *argv[2];
    argv[0] = "payload";
    ExecPS2FromMemory((void *)EE_STAGE_3_ADDR, 1, argv);
}