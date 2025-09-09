#pragma once

#include <stdio.h>

#include <tamtypes.h>

#include "usleep.h"
#include "errno.h"

#define BOOT_ROM_ADDR (0x1FC00000 + 0x000000C0) // points to 0x00000000

inline static void modchip_poke_u8(u8 byte)
{
    *(volatile u8*)BOOT_ROM_ADDR = byte;
    // if writing "too fast", CE will stay high the entire time,
    // without WE wired up, modchip can only see a single byte,
    // force CE low by waiting for a bit
    usleep(2);
}

inline static void modchip_poke_u16(u16 v)
{
    modchip_poke_u8(((u8 *)&v)[0]);
    modchip_poke_u8(((u8 *)&v)[1]);
}

inline static void modchip_poke_u32(u32 v)
{
    modchip_poke_u8(((u8 *)&v)[0]);
    modchip_poke_u8(((u8 *)&v)[1]);
    modchip_poke_u8(((u8 *)&v)[2]);
    modchip_poke_u8(((u8 *)&v)[3]);
}

inline static u8 modchip_peek_u8()
{
    return *(volatile u8 *)BOOT_ROM_ADDR;
}

inline static u16 modchip_peek_u16()
{
    return *(volatile u16 *)BOOT_ROM_ADDR;
}

inline static u32 modchip_peek_u32()
{
    return *(volatile u32 *)BOOT_ROM_ADDR;
}

u32 modchip_apps_read(u32 offset, u32 size, u8 app_idx, void *dst)
{
    modchip_poke_u8(0x78);
    modchip_poke_u8(0x22);
    modchip_poke_u32(offset);
    modchip_poke_u32(offset ^ 0xFFFFFFFF);
    modchip_poke_u32(size);
    modchip_poke_u32(size ^ 0xFFFFFFFF);
    modchip_poke_u8(app_idx);
    modchip_poke_u8(app_idx ^ 0xFF);

    int r = modchip_peek_u32();
    if (r != MODCHIP_CMD_RESULT_OK)
        return r;

    for (uiptr i = 0; i < size / 4; i += 4)
        *(volatile u32 *)(dst + i) = modchip_peek_u32();

    for (uiptr i = 0; i < size % 4; i += 1)
        *(volatile u32 *)(dst + i) = modchip_peek_u8();

    return MODCHIP_CMD_RESULT_OK;
}
