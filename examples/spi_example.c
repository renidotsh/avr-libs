/**
 * @file    spi_example.c
 * @brief   ATmega328P example exercising every avr_spi.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_SPI_IMPLEMENTATION
#include "../avr_spi.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"
#include <util/delay.h>

int main(void)
{
    UART_Init(9600);
    sei();
    UART_SendString("SPI Demo\r\n");

    /* --- Master init (Mode 0, fosc/16 = 1 MHz) --- */
    SPI_MasterInit(SPI_MODE0, SPI_SPEED_DIV16);

    /* --- CS pin setup (PB2) --- */
    SPI_CS_Init(&DDRB, &PORTB, PB2);

    /* --- Data order --- */
    SPI_SetDataOrder(SPI_MSB_FIRST);

    /* --- Single byte transfer --- */
    SPI_CS_LOW(&PORTB, PB2);
    uint8_t rx = SPI_TransferByte(0x9F);  /* e.g. JEDEC ID command */
    UART_SendString("TX:0x9F RX:0x");
    UART_PrintHex8(rx);
    UART_SendString("\r\n");

    /* --- Read 3 response bytes --- */
    uint8_t mfr  = SPI_ReceiveByte();
    uint8_t type = SPI_ReceiveByte();
    uint8_t cap  = SPI_ReceiveByte();
    SPI_CS_HIGH(&PORTB, PB2);

    UART_SendString("MFR:0x");  UART_PrintHex8(mfr);
    UART_SendString(" TYPE:0x"); UART_PrintHex8(type);
    UART_SendString(" CAP:0x");  UART_PrintHex8(cap);
    UART_SendString("\r\n");

    /* --- Buffer transfer --- */
    uint8_t tx_buf[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx_buf[4];
    SPI_CS_LOW(&PORTB, PB2);
    SPI_TransferBuffer(tx_buf, rx_buf, 4);
    SPI_CS_HIGH(&PORTB, PB2);

    UART_SendString("BufRX:");
    for (uint8_t i = 0; i < 4; i++) {
        UART_SendString(" 0x");
        UART_PrintHex8(rx_buf[i]);
    }
    UART_SendString("\r\n");

    /* --- SendByte (no read) --- */
    SPI_CS_LOW(&PORTB, PB2);
    SPI_SendByte(0xAA);
    SPI_CS_HIGH(&PORTB, PB2);

    /* --- LSB-first mode --- */
    SPI_SetDataOrder(SPI_LSB_FIRST);
    SPI_CS_LOW(&PORTB, PB2);
    SPI_SendByte(0x55);
    SPI_CS_HIGH(&PORTB, PB2);
    SPI_SetDataOrder(SPI_MSB_FIRST); /* restore */

    /* --- Disable SPI --- */
    SPI_Disable();
    UART_SendString("SPI disabled\r\n");

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
