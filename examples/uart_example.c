/**
 * @file    uart_example.c
 * @brief   ATmega328P example exercising every avr_uart.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"
#include <util/delay.h>

int main(void)
{
    /* --- Init at 9600 baud --- */
    uart_init(9600);
    sei();

    /* --- printf redirect --- */
    stdout = uart_get_stream();
    printf("UART Demo @ 9600\n");

    /* --- SendByte --- */
    uart_send_byte('A');
    uart_send_byte('\r');
    uart_send_byte('\n');

    /* --- SendString --- */
    uart_send_string("Hello from ATmega328P\r\n");

    /* --- SendBuffer --- */
    const uint8_t buf[] = {0x48, 0x49, 0x0D, 0x0A}; /* "HI\r\n" */
    uart_send_buffer(buf, sizeof(buf));

    /* --- PrintU16 / PrintS16 --- */
    uart_send_string("U16: ");
    uart_print_u16(65535);
    uart_send_string("  S16: ");
    uart_print_s16(-123);
    uart_send_string("\r\n");

    /* --- PrintHex8 --- */
    uart_send_string("Hex: 0x");
    uart_print_hex8(0xDE);
    uart_send_string(" 0x");
    uart_print_hex8(0xAD);
    uart_send_string("\r\n");

    /* --- SendByteBlocking --- */
    uart_send_byte_blocking('Z');
    uart_send_string("\r\n");

    /* --- FlushTx --- */
    uart_send_string("Flushing TX...");
    uart_flush_tx();
    uart_send_string(" done\r\n");

    /* --- Echo loop with error checking --- */
    uart_send_string("Type to echo:\r\n");

    while (1) {
        /* Non-blocking check */
        if (uart_available()) {
            uint8_t c = uart_read_byte();
            uart_send_byte(c);               /* echo */

            /* Error detection */
            uint8_t err = uart_get_errors();
            if (err & UART_ERR_FRAME)
                uart_send_string("[FE]");
            if (err & UART_ERR_PARITY)
                uart_send_string("[PE]");
            if (err & UART_ERR_OVERRUN)
                uart_send_string("[OVR]");
            if (err & UART_ERR_BUFOVF)
                uart_send_string("[BOF]");
        }
    }
    return 0;
}
