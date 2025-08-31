#pragma once

#include <stdint.h>

extern void (*read_handler)(uint8_t);

void handle_read_idle(uint8_t r);
