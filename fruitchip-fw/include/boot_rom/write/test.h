#pragma once

#include <stdint.h>

void handle_write_test_data_out_with_status_no_data_no_crc(uint8_t w);
void handle_write_test_data_out_with_status_with_data_no_crc(uint8_t w);
void handle_write_test_data_out_with_status_with_data_with_crc(uint8_t w);
void handle_write_test_data_out_no_status_with_data_with_crc(uint8_t w);
void handle_write_test_data_out_no_status_with_data_no_crc(uint8_t w);
void handle_write_test_data_out_busy_code(uint8_t w);
