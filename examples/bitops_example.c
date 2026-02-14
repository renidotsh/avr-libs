/**
 * @file    bitops_example.c
 * @brief   ATmega328P example exercising every avr_bitops.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_BITOPS_IMPLEMENTATION
#include "../avr_bitops.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"

int main(void)
{
    UART_Init(9600);
    sei();
    UART_SendString("BitOps Demo\r\n");

    /* --- Extract8 --- */
    uint8_t v = 0b10110010;
    uint8_t f = BITOPS_Extract8(v, 2, 3);  /* bits [4:2] = 0b100 = 4 */
    UART_SendString("Extract8 [4:2]: ");
    UART_PrintU16(f);
    UART_SendString("\r\n");

    /* --- Insert8 --- */
    v = BITOPS_Insert8(v, 0x07, 2, 3);     /* set [4:2] = 111 */
    UART_SendString("Insert8: 0x");
    UART_PrintHex8(v);
    UART_SendString("\r\n");

    /* --- Extract16 / Insert16 --- */
    uint16_t v16 = 0xABCD;
    uint16_t f16 = BITOPS_Extract16(v16, 4, 8);  /* bits [11:4] */
    UART_SendString("Extract16: 0x");
    UART_PrintHex8((uint8_t)f16);
    UART_SendString("\r\n");
    v16 = BITOPS_Insert16(v16, 0xFF, 4, 8);
    UART_SendString("Insert16: 0x");
    UART_PrintHex8((uint8_t)(v16 >> 8));
    UART_PrintHex8((uint8_t)v16);
    UART_SendString("\r\n");

    /* --- RotateLeft8 / RotateRight8 --- */
    UART_SendString("RotL8(0xA5,3): 0x");
    UART_PrintHex8(BITOPS_RotateLeft8(0xA5, 3));
    UART_SendString("\r\n");
    UART_SendString("RotR8(0xA5,3): 0x");
    UART_PrintHex8(BITOPS_RotateRight8(0xA5, 3));
    UART_SendString("\r\n");

    /* --- RotateLeft16 / RotateRight16 --- */
    uint16_t rot = BITOPS_RotateLeft16(0x1234, 4);
    UART_SendString("RotL16(0x1234,4): 0x");
    UART_PrintHex8((uint8_t)(rot >> 8));
    UART_PrintHex8((uint8_t)rot);
    UART_SendString("\r\n");

    /* --- PopCount8 / PopCount16 --- */
    UART_SendString("PopCount8(0xFF): ");
    UART_PrintU16(BITOPS_PopCount8(0xFF));
    UART_SendString("\r\n");
    UART_SendString("PopCount16(0x1234): ");
    UART_PrintU16(BITOPS_PopCount16(0x1234));
    UART_SendString("\r\n");

    /* --- CLZ8 / CTZ8 --- */
    UART_SendString("CLZ8(0x08): ");
    UART_PrintU16(BITOPS_CLZ8(0x08));
    UART_SendString("  CTZ8(0x08): ");
    UART_PrintU16(BITOPS_CTZ8(0x08));
    UART_SendString("\r\n");

    /* --- CLZ16 / CTZ16 --- */
    UART_SendString("CLZ16(0x0080): ");
    UART_PrintU16(BITOPS_CLZ16(0x0080));
    UART_SendString("  CTZ16(0x0080): ");
    UART_PrintU16(BITOPS_CTZ16(0x0080));
    UART_SendString("\r\n");

    /* --- Reverse8 --- */
    UART_SendString("Reverse8(0xA5): 0x");
    UART_PrintHex8(BITOPS_Reverse8(0xA5));
    UART_SendString("\r\n");

    /* --- ByteSwap16 / ByteSwap32 --- */
    uint16_t sw16 = BITOPS_ByteSwap16(0x1234);
    UART_SendString("Swap16(0x1234): 0x");
    UART_PrintHex8((uint8_t)(sw16 >> 8));
    UART_PrintHex8((uint8_t)sw16);
    UART_SendString("\r\n");

    uint32_t sw32 = BITOPS_ByteSwap32(0x12345678UL);
    UART_SendString("Swap32: 0x");
    UART_PrintHex8((uint8_t)(sw32 >> 24));
    UART_PrintHex8((uint8_t)(sw32 >> 16));
    UART_PrintHex8((uint8_t)(sw32 >> 8));
    UART_PrintHex8((uint8_t)sw32);
    UART_SendString("\r\n");

    /* --- NibbleSwap --- */
    UART_SendString("NibbleSwap(0xA5): 0x");
    UART_PrintHex8(BITOPS_NibbleSwap(0xA5));
    UART_SendString("\r\n");

    /* --- CRC-8 --- */
    const uint8_t msg[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t crc = BITOPS_CRC8(msg, 4);
    UART_SendString("CRC-8: 0x");
    UART_PrintHex8(crc);
    UART_SendString("\r\n");

    /* --- CRC-8 incremental --- */
    uint8_t crc2 = 0;
    crc2 = BITOPS_CRC8_Update(crc2, 0x01);
    crc2 = BITOPS_CRC8_Update(crc2, 0x02);
    crc2 = BITOPS_CRC8_Update(crc2, 0x03);
    crc2 = BITOPS_CRC8_Update(crc2, 0x04);
    UART_SendString("CRC-8 inc: 0x");
    UART_PrintHex8(crc2);
    UART_SendString("\r\n");

    while (1);
    return 0;
}
