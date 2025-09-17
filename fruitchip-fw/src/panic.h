#include <pico/status_led.h>
#include <pico/platform/panic.h>

#define panic_with_led(color, ...)  { colored_status_led_set_on_with_color(color); panic(__VA_ARGS__); }

#define RGB_OK           PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0x00, 0x10, 0x00) // green
#define RGB_ERR_APPS     PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0x10, 0x00, 0x00) // red
#define RGB_ERR_SETTINGS PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(0x00, 0x00, 0x10) // blue
