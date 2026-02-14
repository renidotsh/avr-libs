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
    UART_Init(9600);
    sei();

    /* --- printf redirect --- */
    stdout = UART_GetStream();
    printf("UART Demo @ 9600\n");

    /* --- SendByte --- */
    UART_SendByte('A');
    UART_SendByte('\r');
    UART_SendByte('\n');

    /* --- SendString --- */
    UART_SendString("Hello from ATmega328P\r\n");

    /* --- SendBuffer --- */
    const uint8_t buf[] = {0x48, 0x49, 0x0D, 0x0A}; /* "HI\r\n" */
    UART_SendBuffer(buf, sizeof(buf));

    /* --- PrintU16 / PrintS16 --- */
    UART_SendString("U16: ");
    UART_PrintU16(65535);
    UART_SendString("  S16: ");
    UART_PrintS16(-123);
    UART_SendString("\r\n");

    /* --- PrintHex8 --- */
    UART_SendString("Hex: 0x");
    UART_PrintHex8(0xDE);
    UART_SendString(" 0x");
    UART_PrintHex8(0xAD);
    UART_SendString("\r\n");

    /* --- SendByteBlocking --- */
    UART_SendByteBlocking('Z');
    UART_SendString("\r\n");

    /* --- FlushTx --- */
    UART_SendString("Flushing TX...");
    UART_FlushTx();
    UART_SendString(" done\r\n");

    /* --- Echo loop with error checking --- */
    UART_SendString("Type to echo:\r\n");

    while (1) {
        /* Non-blocking check */
        if (UART_Available()) {
            uint8_t c = UART_ReadByte();
            UART_SendByte(c);               /* echo */

            /* Error detection */
            uint8_t err = UART_GetErrors();
            if (err & UART_ERR_FRAME)
                UART_SendString("[FE]");
            if (err & UART_ERR_PARITY)
                UART_SendString("[PE]");
            if (err & UART_ERR_OVERRUN)
                UART_SendString("[OVR]");
            if (err & UART_ERR_BUFOVF)
                UART_SendString("[BOF]");
        }
    }
    return 0;
}
