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
    UART_Init(9600);
    sei();
    UART_SendString("EEPROM Demo\r\n");

    /* --- Byte write & read --- */
    EEPROM_WriteByte(SCRATCH_ADDR, 0x42);
    uint8_t b = EEPROM_ReadByte(SCRATCH_ADDR);
    UART_SendString("Byte: 0x");
    UART_PrintHex8(b);
    UART_SendString("\r\n");

    /* --- Update (wear-leveling) --- */
    EEPROM_UpdateByte(SCRATCH_ADDR, 0x42);  /* no write – same value */
    EEPROM_UpdateByte(SCRATCH_ADDR, 0x43);  /* writes – value changed */
    b = EEPROM_ReadByte(SCRATCH_ADDR);
    UART_SendString("Updated: 0x");
    UART_PrintHex8(b);
    UART_SendString("\r\n");

    /* --- Word write & read --- */
    EEPROM_WriteWord(SCRATCH_ADDR + 2, 0xBEEF);
    uint16_t w = EEPROM_ReadWord(SCRATCH_ADDR + 2);
    UART_SendString("Word: 0x");
    UART_PrintHex8((uint8_t)(w >> 8));
    UART_PrintHex8((uint8_t)w);
    UART_SendString("\r\n");

    /* --- Update word --- */
    EEPROM_UpdateWord(SCRATCH_ADDR + 2, 0xBEEF);  /* no write */

    /* --- Block write / read --- */
    uint8_t block_w[] = {0xDE, 0xAD, 0xCA, 0xFE};
    EEPROM_WriteBlock(SCRATCH_ADDR + 4, block_w, 4);
    uint8_t block_r[4];
    EEPROM_ReadBlock(SCRATCH_ADDR + 4, block_r, 4);
    UART_SendString("Block:");
    for (uint8_t i = 0; i < 4; i++) {
        UART_SendString(" 0x");
        UART_PrintHex8(block_r[i]);
    }
    UART_SendString("\r\n");

    /* --- Update block --- */
    EEPROM_UpdateBlock(SCRATCH_ADDR + 4, block_w, 4);

    /* --- CRC-8 over block --- */
    uint8_t crc = EEPROM_CRC8(SCRATCH_ADDR + 4, 4);
    UART_SendString("CRC-8: 0x");
    UART_PrintHex8(crc);
    UART_SendString("\r\n");

    /* --- Write block with CRC --- */
    settings_t cfg = {1, 128, 2};
    EEPROM_WriteBlockWithCRC(SETTINGS_ADDR, (const uint8_t*)&cfg,
                             sizeof(cfg));

    /* --- Read block and verify CRC --- */
    settings_t cfg_read;
    bool valid = EEPROM_ReadBlockVerifyCRC(SETTINGS_ADDR,
                     (uint8_t*)&cfg_read, sizeof(cfg_read));
    UART_SendString("CRC verify: ");
    UART_SendString(valid ? "OK" : "FAIL");
    UART_SendString("\r\n");
    UART_SendString("Boot#: ");
    UART_PrintU16(cfg_read.boot_count);
    UART_SendString("\r\n");

    /* --- Busy check --- */
    EEPROM_WriteByte(SCRATCH_ADDR + 10, 0xFF);
    UART_SendString("Busy: ");
    UART_SendString(EEPROM_IsBusy() ? "yes" : "no");
    UART_SendString("\r\n");

    while (1);
    return 0;
}
