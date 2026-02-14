/**
 * @file     avr_bitops.h
 * @brief    Advanced bit manipulation library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Provides optimised bit-field extraction/insertion, rotate/shift
 * operations, population count, leading/trailing zero count, CRC-8
 * with PROGMEM look-up table, and endianness helpers.  All operations
 * are tuned for the 8-bit AVR instruction set.
 *
 * @features
 * - Bit field extract / insert
 * - Rotate left / right (8-bit and 16-bit)
 * - Population count (number of set bits)
 * - Count leading / trailing zeros
 * - CRC-8 (polynomial 0x07) with PROGMEM LUT
 * - Byte / word swap, endianness conversion
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_BITOPS_H
#define AVR_BITOPS_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== BIT FIELD OPERATIONS ===== */

/**
 * @brief  Extract a bit field from an 8-bit value.
 * @param  val   Source byte
 * @param  pos   Bit position of LSB of the field (0-7)
 * @param  width Number of bits (1-8)
 * @return Extracted field, right-aligned
 */
static inline uint8_t BITOPS_Extract8(uint8_t val, uint8_t pos, uint8_t width)
{
    return (val >> pos) & ((1U << width) - 1);
}

/**
 * @brief  Insert a value into a bit field of an 8-bit target.
 * @param  target  Original 8-bit value
 * @param  val     Value to insert (right-aligned, only 'width' bits used)
 * @param  pos     Bit position of LSB
 * @param  width   Number of bits
 * @return Modified byte
 */
static inline uint8_t BITOPS_Insert8(uint8_t target, uint8_t val,
                                     uint8_t pos, uint8_t width)
{
    uint8_t mask = ((1U << width) - 1) << pos;
    return (target & ~mask) | ((val << pos) & mask);
}

/**
 * @brief  Extract bit field from a 16-bit value.
 */
static inline uint16_t BITOPS_Extract16(uint16_t val, uint8_t pos,
                                        uint8_t width)
{
    return (val >> pos) & ((1UL << width) - 1);
}

/**
 * @brief  Insert into a 16-bit bit field.
 */
static inline uint16_t BITOPS_Insert16(uint16_t target, uint16_t val,
                                       uint8_t pos, uint8_t width)
{
    uint16_t mask = ((1UL << width) - 1) << pos;
    return (target & ~mask) | ((val << pos) & mask);
}

/* ===== ROTATE ===== */

/**
 * @brief  Rotate an 8-bit value left by n positions.
 */
static inline uint8_t BITOPS_RotateLeft8(uint8_t val, uint8_t n)
{
    n &= 7;
    return (val << n) | (val >> (8 - n));
}

/**
 * @brief  Rotate an 8-bit value right by n positions.
 */
static inline uint8_t BITOPS_RotateRight8(uint8_t val, uint8_t n)
{
    n &= 7;
    return (val >> n) | (val << (8 - n));
}

/**
 * @brief  Rotate a 16-bit value left by n positions.
 */
static inline uint16_t BITOPS_RotateLeft16(uint16_t val, uint8_t n)
{
    n &= 15;
    return (val << n) | (val >> (16 - n));
}

/**
 * @brief  Rotate a 16-bit value right by n positions.
 */
static inline uint16_t BITOPS_RotateRight16(uint16_t val, uint8_t n)
{
    n &= 15;
    return (val >> n) | (val << (16 - n));
}

/* ===== POPULATION COUNT ===== */

/**
 * @brief  Count the number of set bits in an 8-bit value.
 *         Uses Brian Kernighan's algorithm.
 * @return Number of 1-bits (0-8)
 */
static inline uint8_t BITOPS_PopCount8(uint8_t val)
{
    uint8_t count = 0;
    while (val) {
        val &= (val - 1);   /* clear lowest set bit */
        count++;
    }
    return count;
}

/**
 * @brief  Count set bits in a 16-bit value.
 */
static inline uint8_t BITOPS_PopCount16(uint16_t val)
{
    return BITOPS_PopCount8((uint8_t)val) +
           BITOPS_PopCount8((uint8_t)(val >> 8));
}

/* ===== LEADING / TRAILING ZEROS ===== */

/**
 * @brief  Count leading zeros in an 8-bit value.
 * @return 0-8 (8 if val == 0)
 */
static inline uint8_t BITOPS_CLZ8(uint8_t val)
{
    if (val == 0) return 8;
    uint8_t n = 0;
    if ((val & 0xF0) == 0) { n += 4; val <<= 4; }
    if ((val & 0xC0) == 0) { n += 2; val <<= 2; }
    if ((val & 0x80) == 0) { n += 1; }
    return n;
}

/**
 * @brief  Count trailing zeros in an 8-bit value.
 * @return 0-8 (8 if val == 0)
 */
static inline uint8_t BITOPS_CTZ8(uint8_t val)
{
    if (val == 0) return 8;
    uint8_t n = 0;
    if ((val & 0x0F) == 0) { n += 4; val >>= 4; }
    if ((val & 0x03) == 0) { n += 2; val >>= 2; }
    if ((val & 0x01) == 0) { n += 1; }
    return n;
}

/**
 * @brief  Count leading zeros in a 16-bit value.
 */
static inline uint8_t BITOPS_CLZ16(uint16_t val)
{
    if (val == 0) return 16;
    uint8_t hi = (uint8_t)(val >> 8);
    if (hi) return BITOPS_CLZ8(hi);
    return 8 + BITOPS_CLZ8((uint8_t)val);
}

/**
 * @brief  Count trailing zeros in a 16-bit value.
 */
static inline uint8_t BITOPS_CTZ16(uint16_t val)
{
    if (val == 0) return 16;
    uint8_t lo = (uint8_t)val;
    if (lo) return BITOPS_CTZ8(lo);
    return 8 + BITOPS_CTZ8((uint8_t)(val >> 8));
}

/* ===== BYTE / WORD SWAP ===== */

/**
 * @brief  Reverse the bits in an 8-bit value.
 */
static inline uint8_t BITOPS_Reverse8(uint8_t val)
{
    val = ((val & 0xF0) >> 4) | ((val & 0x0F) << 4);
    val = ((val & 0xCC) >> 2) | ((val & 0x33) << 2);
    val = ((val & 0xAA) >> 1) | ((val & 0x55) << 1);
    return val;
}

/**
 * @brief  Swap the two bytes of a 16-bit value (endianness conversion).
 */
static inline uint16_t BITOPS_ByteSwap16(uint16_t val)
{
    return (val >> 8) | (val << 8);
}

/**
 * @brief  Swap bytes in a 32-bit value.
 */
static inline uint32_t BITOPS_ByteSwap32(uint32_t val)
{
    return ((val & 0xFF000000UL) >> 24) |
           ((val & 0x00FF0000UL) >> 8)  |
           ((val & 0x0000FF00UL) << 8)  |
           ((val & 0x000000FFUL) << 24);
}

/**
 * @brief  Swap nibbles in a byte (high ↔ low 4 bits).
 */
static inline uint8_t BITOPS_NibbleSwap(uint8_t val)
{
    return (val >> 4) | (val << 4);
}

/* ===== CRC-8 WITH PROGMEM LUT ===== */

#ifdef AVR_BITOPS_IMPLEMENTATION

/**
 * CRC-8 look-up table (polynomial 0x07) stored in flash via PROGMEM.
 * Saves 256 bytes of SRAM at the cost of slightly slower access.
 */
static const uint8_t _crc8_lut[256] PROGMEM = {
    0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,
    0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,
    0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,
    0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D,
    0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,
    0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD,
    0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,
    0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD,
    0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,
    0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,
    0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,
    0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A,
    0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,
    0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A,
    0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,
    0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A,
    0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,0x9C,
    0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,
    0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,
    0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,
    0x69,0x6E,0x67,0x60,0x75,0x72,0x7B,0x7C,
    0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44,
    0x19,0x1E,0x17,0x10,0x05,0x02,0x0B,0x0C,
    0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34,
    0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,
    0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x63,
    0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,
    0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,
    0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,
    0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x83,
    0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,
    0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF3
};

#endif /* AVR_BITOPS_IMPLEMENTATION */

/**
 * @brief  Compute CRC-8 over a byte buffer using PROGMEM look-up table.
 * @param  data  Byte buffer
 * @param  len   Number of bytes
 * @return CRC-8 value
 */
uint8_t BITOPS_CRC8(const uint8_t *data, uint16_t len);

/**
 * @brief  Update a running CRC-8 with one byte.
 * @param  crc   Current CRC value
 * @param  byte  Next data byte
 * @return Updated CRC
 */
uint8_t BITOPS_CRC8_Update(uint8_t crc, uint8_t byte);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_BITOPS_IMPLEMENTATION

uint8_t BITOPS_CRC8_Update(uint8_t crc, uint8_t byte)
{
    return pgm_read_byte(&_crc8_lut[crc ^ byte]);
}

uint8_t BITOPS_CRC8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++)
        crc = BITOPS_CRC8_Update(crc, data[i]);
    return crc;
}

#endif /* AVR_BITOPS_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_BITOPS_IMPLEMENTATION
#include "avr_bitops.h"

#define AVR_UART_IMPLEMENTATION
#include "avr_uart.h"

int main(void)
{
    UART_Init(9600);
    sei();

    uint8_t val = 0b10110010;

    /* Bit field extract: bits [4:2] → 0b100 = 4 */
    uint8_t field = BITOPS_Extract8(val, 2, 3);
    UART_SendString("Field [4:2]: ");
    UART_PrintU16(field);
    UART_SendString("\r\n");

    /* Insert 0b111 at [4:2] */
    val = BITOPS_Insert8(val, 0x07, 2, 3);
    UART_SendString("After insert: 0x");
    UART_PrintHex8(val);
    UART_SendString("\r\n");

    /* Rotate */
    UART_SendString("RotL 0xA5 by 3: 0x");
    UART_PrintHex8(BITOPS_RotateLeft8(0xA5, 3));
    UART_SendString("\r\n");

    /* Population count */
    UART_SendString("PopCount 0xFF: ");
    UART_PrintU16(BITOPS_PopCount8(0xFF));
    UART_SendString("\r\n");

    /* CLZ / CTZ */
    UART_SendString("CLZ(0x08): ");
    UART_PrintU16(BITOPS_CLZ8(0x08));
    UART_SendString("  CTZ(0x08): ");
    UART_PrintU16(BITOPS_CTZ8(0x08));
    UART_SendString("\r\n");

    /* CRC-8 */
    const uint8_t msg[] = {0x01, 0x02, 0x03};
    uint8_t crc = BITOPS_CRC8(msg, 3);
    UART_SendString("CRC-8: 0x");
    UART_PrintHex8(crc);
    UART_SendString("\r\n");

    /* Byte swap */
    uint16_t w = 0x1234;
    w = BITOPS_ByteSwap16(w);
    UART_SendString("Swap 0x1234 → 0x");
    UART_PrintHex8((uint8_t)(w >> 8));
    UART_PrintHex8((uint8_t)w);
    UART_SendString("\r\n");

    /* Bit reverse */
    UART_SendString("Reverse 0xA5: 0x");
    UART_PrintHex8(BITOPS_Reverse8(0xA5));
    UART_SendString("\r\n");

    while (1);
    return 0;
}
#endif

#endif /* AVR_BITOPS_H */
