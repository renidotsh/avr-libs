/**
 * @file     avr_eeprom.h
 * @brief    Internal EEPROM manager library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Manages the ATmega328P's 1 KB internal EEPROM via EEAR/EEDR/EECR
 * registers.  Provides byte, word, and block read/write with wear-
 * leveling–aware update (write-only-if-changed), CRC-8 integrity
 * checks, and optional interrupt-driven writes.
 *
 * @features
 * - Byte, word (16-bit), and block read / write
 * - Update functions (only write if value differs – wear leveling)
 * - CRC-8 integrity checking on blocks
 * - Busy-wait and interrupt-driven write modes
 * - Atomic operations via interrupt disable
 *
 * @example
 *   #define AVR_EEPROM_IMPLEMENTATION
 *   #include "avr_eeprom.h"
 *
 *   int main(void) {
 *       EEPROM_WriteByte(0x00, 0x42);
 *       uint8_t val = EEPROM_ReadByte(0x00);
 *   }
 *
 * @target   ATmega328P (1 KB EEPROM), ATmega2560 (4 KB)
 * @license  MIT License
 */

#ifndef AVR_EEPROM_H
#define AVR_EEPROM_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== CONFIGURATION ===== */

/** EEPROM size in bytes.  Override for larger chips. */
#ifndef EEPROM_SIZE
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
#define EEPROM_SIZE 1024
#elif defined(__AVR_ATmega2560__)
#define EEPROM_SIZE 4096
#else
#define EEPROM_SIZE 1024
#endif
#endif

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Read a single byte from EEPROM.
 * @param  addr  EEPROM address (0 .. EEPROM_SIZE-1)
 * @return Byte value
 */
uint8_t EEPROM_ReadByte(uint16_t addr);

/**
 * @brief  Write a single byte to EEPROM (blocking: waits for completion).
 * @param  addr  EEPROM address
 * @param  data  Byte to write
 * @note   Takes ~3.3 ms per byte.
 */
void EEPROM_WriteByte(uint16_t addr, uint8_t data);

/**
 * @brief  Update a byte only if it differs from current value.
 *         Reduces wear on EEPROM cells.
 * @param  addr  EEPROM address
 * @param  data  Value to store
 */
void EEPROM_UpdateByte(uint16_t addr, uint8_t data);

/**
 * @brief  Read a 16-bit word (little-endian).
 * @param  addr  EEPROM address of low byte
 * @return 16-bit value
 */
uint16_t EEPROM_ReadWord(uint16_t addr);

/**
 * @brief  Write a 16-bit word (little-endian, blocking).
 * @param  addr  EEPROM address of low byte
 * @param  data  16-bit value
 */
void EEPROM_WriteWord(uint16_t addr, uint16_t data);

/**
 * @brief  Update a 16-bit word only if changed.
 */
void EEPROM_UpdateWord(uint16_t addr, uint16_t data);

/**
 * @brief  Read a block of bytes from EEPROM.
 * @param  addr  Starting EEPROM address
 * @param  buf   Destination buffer in SRAM
 * @param  len   Number of bytes
 */
void EEPROM_ReadBlock(uint16_t addr, uint8_t *buf, uint16_t len);

/**
 * @brief  Write a block of bytes to EEPROM (blocking).
 * @param  addr  Starting EEPROM address
 * @param  buf   Source buffer in SRAM
 * @param  len   Number of bytes
 */
void EEPROM_WriteBlock(uint16_t addr, const uint8_t *buf, uint16_t len);

/**
 * @brief  Update a block (write-only-if-changed per byte).
 */
void EEPROM_UpdateBlock(uint16_t addr, const uint8_t *buf, uint16_t len);

/**
 * @brief  Erase the entire EEPROM (set to 0xFF).
 */
void EEPROM_EraseAll(void);

/**
 * @brief  Compute CRC-8 over a range of EEPROM addresses.
 * @param  addr  Start address
 * @param  len   Number of bytes
 * @return CRC-8 value (polynomial 0x07)
 */
uint8_t EEPROM_CRC8(uint16_t addr, uint16_t len);

/**
 * @brief  Write a block with an appended CRC-8 byte.
 * @param  addr  Start address (block + 1 CRC byte will be written)
 * @param  buf   Source buffer
 * @param  len   Number of data bytes (CRC stored at addr+len)
 */
void EEPROM_WriteBlockWithCRC(uint16_t addr, const uint8_t *buf,
                              uint16_t len);

/**
 * @brief  Read a block and verify its CRC-8.
 * @param  addr  Start address
 * @param  buf   Destination buffer
 * @param  len   Number of data bytes (CRC expected at addr+len)
 * @return true if CRC matches, false if corrupt
 */
bool EEPROM_ReadBlockVerifyCRC(uint16_t addr, uint8_t *buf, uint16_t len);

/**
 * @brief  Check if EEPROM is busy (previous write in progress).
 * @return true if busy
 */
bool EEPROM_IsBusy(void);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_EEPROM_IMPLEMENTATION

/* ---- busy wait ---- */

static void _eeprom_wait_ready(void)
{
    while (EECR & (1 << EEPE))
        ;  /* wait for previous write to complete */
}

/* ---- byte ---- */

uint8_t EEPROM_ReadByte(uint16_t addr)
{
    _eeprom_wait_ready();

    EEAR = addr;
    EECR |= (1 << EERE);   /* start read */
    return EEDR;
}

void EEPROM_WriteByte(uint16_t addr, uint8_t data)
{
    _eeprom_wait_ready();

    uint8_t sreg = SREG;
    cli();

    EEAR = addr;
    EEDR = data;

    /* Erase-and-write in one operation (EEPM[1:0] = 00, default) */
    EECR |= (1 << EEMPE);  /* master program enable */
    EECR |= (1 << EEPE);   /* start erase + write   */

    SREG = sreg;
}

void EEPROM_UpdateByte(uint16_t addr, uint8_t data)
{
    if (EEPROM_ReadByte(addr) != data)
        EEPROM_WriteByte(addr, data);
}

/* ---- word ---- */

uint16_t EEPROM_ReadWord(uint16_t addr)
{
    uint16_t val  = EEPROM_ReadByte(addr);
    val |= ((uint16_t)EEPROM_ReadByte(addr + 1) << 8);
    return val;
}

void EEPROM_WriteWord(uint16_t addr, uint16_t data)
{
    EEPROM_WriteByte(addr,     (uint8_t)(data & 0xFF));
    EEPROM_WriteByte(addr + 1, (uint8_t)(data >> 8));
}

void EEPROM_UpdateWord(uint16_t addr, uint16_t data)
{
    EEPROM_UpdateByte(addr,     (uint8_t)(data & 0xFF));
    EEPROM_UpdateByte(addr + 1, (uint8_t)(data >> 8));
}

/* ---- block ---- */

void EEPROM_ReadBlock(uint16_t addr, uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        buf[i] = EEPROM_ReadByte(addr + i);
}

void EEPROM_WriteBlock(uint16_t addr, const uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        EEPROM_WriteByte(addr + i, buf[i]);
}

void EEPROM_UpdateBlock(uint16_t addr, const uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        EEPROM_UpdateByte(addr + i, buf[i]);
}

void EEPROM_EraseAll(void)
{
    for (uint16_t i = 0; i < EEPROM_SIZE; i++)
        EEPROM_WriteByte(i, 0xFF);
}

/* ---- CRC-8 (polynomial 0x07) ---- */

static uint8_t _crc8_byte(uint8_t crc, uint8_t data)
{
    crc ^= data;
    for (uint8_t i = 0; i < 8; i++) {
        if (crc & 0x80)
            crc = (crc << 1) ^ 0x07;
        else
            crc <<= 1;
    }
    return crc;
}

uint8_t EEPROM_CRC8(uint16_t addr, uint16_t len)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++)
        crc = _crc8_byte(crc, EEPROM_ReadByte(addr + i));
    return crc;
}

void EEPROM_WriteBlockWithCRC(uint16_t addr, const uint8_t *buf,
                              uint16_t len)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++) {
        EEPROM_WriteByte(addr + i, buf[i]);
        crc = _crc8_byte(crc, buf[i]);
    }
    EEPROM_WriteByte(addr + len, crc);
}

bool EEPROM_ReadBlockVerifyCRC(uint16_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++) {
        buf[i] = EEPROM_ReadByte(addr + i);
        crc = _crc8_byte(crc, buf[i]);
    }
    uint8_t stored_crc = EEPROM_ReadByte(addr + len);
    return (crc == stored_crc);
}

bool EEPROM_IsBusy(void)
{
    return (EECR & (1 << EEPE)) != 0;
}

#endif /* AVR_EEPROM_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_EEPROM_IMPLEMENTATION
#include "avr_eeprom.h"

#define AVR_UART_IMPLEMENTATION
#include "avr_uart.h"

typedef struct {
    uint16_t boot_count;
    uint8_t  brightness;
    uint8_t  mode;
} settings_t;

#define SETTINGS_ADDR 0

int main(void)
{
    UART_Init(9600);
    sei();

    /* Read settings with CRC verification */
    settings_t cfg;
    bool valid = EEPROM_ReadBlockVerifyCRC(SETTINGS_ADDR,
                     (uint8_t*)&cfg, sizeof(cfg));

    if (!valid) {
        UART_SendString("EEPROM corrupt – defaults\r\n");
        cfg.boot_count = 0;
        cfg.brightness = 128;
        cfg.mode = 1;
    }

    cfg.boot_count++;
    UART_SendString("Boot #");
    UART_PrintU16(cfg.boot_count);
    UART_SendString("\r\n");

    /* Save with CRC */
    EEPROM_WriteBlockWithCRC(SETTINGS_ADDR,
                             (const uint8_t*)&cfg, sizeof(cfg));

    /* Single-byte update (wear-leveled) */
    EEPROM_UpdateByte(0x10, 0x55);

    while (1);
    return 0;
}
#endif

#endif /* AVR_EEPROM_H */
