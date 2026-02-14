/**
 * @file    eeprom_example.c
 * @brief   ATmega328P example exercising every avr_eeprom.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_EEPROM_IMPLEMENTATION
#include "../avr_eeprom.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"

typedef struct {
    uint16_t boot_count;
    uint8_t  brightness;
    uint8_t  mode;
} settings_t;

#define SETTINGS_ADDR  0x00
#define SCRATCH_ADDR   0x20

int main(void)
{
    uart_init(9600);
    sei();
    uart_send_string("EEPROM Demo\r\n");

    /* --- Byte write & read --- */
    eeprom_write_byte(SCRATCH_ADDR, 0x42);
    uint8_t b = eeprom_read_byte(SCRATCH_ADDR);
    uart_send_string("Byte: 0x");
    uart_print_hex8(b);
    uart_send_string("\r\n");

    /* --- Update (wear-leveling) --- */
    eeprom_update_byte(SCRATCH_ADDR, 0x42);  /* no write – same value */
    eeprom_update_byte(SCRATCH_ADDR, 0x43);  /* writes – value changed */
    b = eeprom_read_byte(SCRATCH_ADDR);
    uart_send_string("Updated: 0x");
    uart_print_hex8(b);
    uart_send_string("\r\n");

    /* --- Word write & read --- */
    eeprom_write_word(SCRATCH_ADDR + 2, 0xBEEF);
    uint16_t w = eeprom_read_word(SCRATCH_ADDR + 2);
    uart_send_string("Word: 0x");
    uart_print_hex8((uint8_t)(w >> 8));
    uart_print_hex8((uint8_t)w);
    uart_send_string("\r\n");

    /* --- Update word --- */
    eeprom_update_word(SCRATCH_ADDR + 2, 0xBEEF);  /* no write */

    /* --- Block write / read --- */
    uint8_t block_w[] = {0xDE, 0xAD, 0xCA, 0xFE};
    eeprom_write_block(SCRATCH_ADDR + 4, block_w, 4);
    uint8_t block_r[4];
    eeprom_read_block(SCRATCH_ADDR + 4, block_r, 4);
    uart_send_string("Block:");
    for (uint8_t i = 0; i < 4; i++) {
        uart_send_string(" 0x");
        uart_print_hex8(block_r[i]);
    }
    uart_send_string("\r\n");

    /* --- Update block --- */
    eeprom_update_block(SCRATCH_ADDR + 4, block_w, 4);

    /* --- CRC-8 over block --- */
    uint8_t crc = eeprom_crc8(SCRATCH_ADDR + 4, 4);
    uart_send_string("CRC-8: 0x");
    uart_print_hex8(crc);
    uart_send_string("\r\n");

    /* --- Write block with CRC --- */
    settings_t cfg = {1, 128, 2};
    eeprom_write_block_with_crc(SETTINGS_ADDR, (const uint8_t*)&cfg,
                             sizeof(cfg));

    /* --- Read block and verify CRC --- */
    settings_t cfg_read;
    bool valid = eeprom_read_block_verify_crc(SETTINGS_ADDR,
                     (uint8_t*)&cfg_read, sizeof(cfg_read));
    uart_send_string("CRC verify: ");
    uart_send_string(valid ? "OK" : "FAIL");
    uart_send_string("\r\n");
    uart_send_string("Boot#: ");
    uart_print_u16(cfg_read.boot_count);
    uart_send_string("\r\n");

    /* --- Busy check --- */
    eeprom_write_byte(SCRATCH_ADDR + 10, 0xFF);
    uart_send_string("Busy: ");
    uart_send_string(eeprom_is_busy() ? "yes" : "no");
    uart_send_string("\r\n");

    while (1);
    return 0;
}
