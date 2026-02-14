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
    uart_init(9600);
    sei();
    uart_send_string("SPI Demo\r\n");

    /* --- Master init (Mode 0, fosc/16 = 1 MHz) --- */
    spi_master_init(SPI_MODE0, SPI_SPEED_DIV16);

    /* --- CS pin setup (PB2) --- */
    spi_cs_init(&DDRB, &PORTB, PB2);

    /* --- Data order --- */
    spi_set_data_order(SPI_MSB_FIRST);

    /* --- Single byte transfer --- */
    SPI_CS_LOW(&PORTB, PB2);
    uint8_t rx = spi_transfer_byte(0x9F);  /* e.g. JEDEC ID command */
    uart_send_string("TX:0x9F RX:0x");
    uart_print_hex8(rx);
    uart_send_string("\r\n");

    /* --- Read 3 response bytes --- */
    uint8_t mfr  = spi_receive_byte();
    uint8_t type = spi_receive_byte();
    uint8_t cap  = spi_receive_byte();
    SPI_CS_HIGH(&PORTB, PB2);

    uart_send_string("MFR:0x");  uart_print_hex8(mfr);
    uart_send_string(" TYPE:0x"); uart_print_hex8(type);
    uart_send_string(" CAP:0x");  uart_print_hex8(cap);
    uart_send_string("\r\n");

    /* --- Buffer transfer --- */
    uint8_t tx_buf[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx_buf[4];
    SPI_CS_LOW(&PORTB, PB2);
    spi_transfer_buffer(tx_buf, rx_buf, 4);
    SPI_CS_HIGH(&PORTB, PB2);

    uart_send_string("BufRX:");
    for (uint8_t i = 0; i < 4; i++) {
        uart_send_string(" 0x");
        uart_print_hex8(rx_buf[i]);
    }
    uart_send_string("\r\n");

    /* --- SendByte (no read) --- */
    SPI_CS_LOW(&PORTB, PB2);
    spi_send_byte(0xAA);
    SPI_CS_HIGH(&PORTB, PB2);

    /* --- LSB-first mode --- */
    spi_set_data_order(SPI_LSB_FIRST);
    SPI_CS_LOW(&PORTB, PB2);
    spi_send_byte(0x55);
    SPI_CS_HIGH(&PORTB, PB2);
    spi_set_data_order(SPI_MSB_FIRST); /* restore */

    /* --- Disable SPI --- */
    spi_disable();
    uart_send_string("SPI disabled\r\n");

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
