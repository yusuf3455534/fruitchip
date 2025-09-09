#pragma once

#include <stdint.h>

extern void (*read_handler)(uint8_t);

extern void (*write_handler)(uint8_t);
