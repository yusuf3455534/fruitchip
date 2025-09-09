#include <stdio.h>

#include "data_out.h"

uint32_t _boot_rom_data_out_status_code;
uint8_t _zero = 0;

void __time_critical_func(boot_rom_byte_out_irq_handler)()
{
    if (!pio_sm_is_tx_fifo_empty(pio0, BOOT_ROM_DATA_OUT_SM))
        goto exit;

    boot_rom_data_out_stop();
    boot_rom_sniffers_start();
    boot_rom_data_out_reset();

exit:
    pio_interrupt_clear(pio0, BOOT_ROM_BYTE_OUT_IRQ);
}
