#pragma once

#include <pico/platform/panic.h>

#include "led_color.h"

#define panic_with_led(color, ...)  { colored_status_led_set_on_with_color(color); panic(__VA_ARGS__); }
