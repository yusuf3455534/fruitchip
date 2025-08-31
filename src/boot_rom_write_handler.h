#pragma once

#include <stdint.h>

extern void (*write_handler)(uint8_t);

void handle_write_idle(uint8_t w);
