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
    uart_init(9600);
    sei();
    uart_send_string("BitOps Demo\r\n");

    /* --- Extract8 --- */
    uint8_t v = 0b10110010;
    uint8_t f = bitops_extract8(v, 2, 3);  /* bits [4:2] = 0b100 = 4 */
    uart_send_string("Extract8 [4:2]: ");
    uart_print_u16(f);
    uart_send_string("\r\n");

    /* --- Insert8 --- */
    v = bitops_insert8(v, 0x07, 2, 3);     /* set [4:2] = 111 */
    uart_send_string("Insert8: 0x");
    uart_print_hex8(v);
    uart_send_string("\r\n");

    /* --- Extract16 / Insert16 --- */
    uint16_t v16 = 0xABCD;
    uint16_t f16 = bitops_extract16(v16, 4, 8);  /* bits [11:4] */
    uart_send_string("Extract16: 0x");
    uart_print_hex8((uint8_t)f16);
    uart_send_string("\r\n");
    v16 = bitops_insert16(v16, 0xFF, 4, 8);
    uart_send_string("Insert16: 0x");
    uart_print_hex8((uint8_t)(v16 >> 8));
    uart_print_hex8((uint8_t)v16);
    uart_send_string("\r\n");

    /* --- RotateLeft8 / RotateRight8 --- */
    uart_send_string("RotL8(0xA5,3): 0x");
    uart_print_hex8(bitops_rotate_left8(0xA5, 3));
    uart_send_string("\r\n");
    uart_send_string("RotR8(0xA5,3): 0x");
    uart_print_hex8(bitops_rotate_right8(0xA5, 3));
    uart_send_string("\r\n");

    /* --- RotateLeft16 / RotateRight16 --- */
    uint16_t rot = bitops_rotate_left16(0x1234, 4);
    uart_send_string("RotL16(0x1234,4): 0x");
    uart_print_hex8((uint8_t)(rot >> 8));
    uart_print_hex8((uint8_t)rot);
    uart_send_string("\r\n");

    /* --- PopCount8 / PopCount16 --- */
    uart_send_string("PopCount8(0xFF): ");
    uart_print_u16(bitops_pop_count8(0xFF));
    uart_send_string("\r\n");
    uart_send_string("PopCount16(0x1234): ");
    uart_print_u16(bitops_pop_count16(0x1234));
    uart_send_string("\r\n");

    /* --- CLZ8 / CTZ8 --- */
    uart_send_string("CLZ8(0x08): ");
    uart_print_u16(bitops_clz8(0x08));
    uart_send_string("  CTZ8(0x08): ");
    uart_print_u16(bitops_ctz8(0x08));
    uart_send_string("\r\n");

    /* --- CLZ16 / CTZ16 --- */
    uart_send_string("CLZ16(0x0080): ");
    uart_print_u16(bitops_clz16(0x0080));
    uart_send_string("  CTZ16(0x0080): ");
    uart_print_u16(bitops_ctz16(0x0080));
    uart_send_string("\r\n");

    /* --- Reverse8 --- */
    uart_send_string("Reverse8(0xA5): 0x");
    uart_print_hex8(bitops_reverse8(0xA5));
    uart_send_string("\r\n");

    /* --- ByteSwap16 / ByteSwap32 --- */
    uint16_t sw16 = bitops_byte_swap16(0x1234);
    uart_send_string("Swap16(0x1234): 0x");
    uart_print_hex8((uint8_t)(sw16 >> 8));
    uart_print_hex8((uint8_t)sw16);
    uart_send_string("\r\n");

    uint32_t sw32 = bitops_byte_swap32(0x12345678UL);
    uart_send_string("Swap32: 0x");
    uart_print_hex8((uint8_t)(sw32 >> 24));
    uart_print_hex8((uint8_t)(sw32 >> 16));
    uart_print_hex8((uint8_t)(sw32 >> 8));
    uart_print_hex8((uint8_t)sw32);
    uart_send_string("\r\n");

    /* --- NibbleSwap --- */
    uart_send_string("NibbleSwap(0xA5): 0x");
    uart_print_hex8(bitops_nibble_swap(0xA5));
    uart_send_string("\r\n");

    /* --- CRC-8 --- */
    const uint8_t msg[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t crc = bitops_crc8(msg, 4);
    uart_send_string("CRC-8: 0x");
    uart_print_hex8(crc);
    uart_send_string("\r\n");

    /* --- CRC-8 incremental --- */
    uint8_t crc2 = 0;
    crc2 = bitops_crc8_update(crc2, 0x01);
    crc2 = bitops_crc8_update(crc2, 0x02);
    crc2 = bitops_crc8_update(crc2, 0x03);
    crc2 = bitops_crc8_update(crc2, 0x04);
    uart_send_string("CRC-8 inc: 0x");
    uart_print_hex8(crc2);
    uart_send_string("\r\n");

    while (1);
    return 0;
}
