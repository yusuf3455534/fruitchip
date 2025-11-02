#pragma once

#include <stdint.h>

extern bool flash_write_lock;

void handle_write_get_partition_size(uint8_t);
void handle_write_get_flash_sector_size(uint8_t w);
void handle_write_write_lock(uint8_t);
void handle_write_erase_flash_sector(uint8_t);
void handle_write_write_flash_sector(uint8_t);
void handle_write_reboot(uint8_t);
void handle_write_ping(uint8_t);
