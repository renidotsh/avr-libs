/**
 * @file    wdt_example.c
 * @brief   ATmega328P example exercising every avr_wdt.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_WDT_IMPLEMENTATION
#include "../avr_wdt.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"
#include <util/delay.h>

static volatile uint8_t wdt_isr_flag = 0;

void wdt_interrupt_handler(void)
{
    wdt_isr_flag = 1;
}

int main(void)
{
    uart_init(9600);
    sei();

    /* --- Get reset cause (call once at startup) --- */
    uint8_t cause = wdt_get_reset_cause();
    uart_send_string("Reset cause: 0x");
    uart_print_hex8(cause);
    uart_send_string("\r\n");

    if (cause & WDT_RESET_POWER_ON)
        uart_send_string("  Power-on\r\n");
    if (cause & WDT_RESET_EXTERNAL)
        uart_send_string("  External\r\n");
    if (cause & WDT_RESET_BROWN_OUT)
        uart_send_string("  Brown-out\r\n");
    if (cause & WDT_RESET_WATCHDOG)
        uart_send_string("  Watchdog\r\n");

    /* --- LED --- */
    DDRB |= (1 << PB5);

    /* --- Mode: System Reset (1s timeout) --- */
    uart_send_string("WDT Reset mode (1s)\r\n");
    wdt_configure(WDT_TIMEOUT_1S, WDT_MODE_RESET);

    for (uint8_t i = 0; i < 3; i++) {
        wdt_kick();            /* kick before timeout */
        uart_send_string("  kick\r\n");
        _delay_ms(500);
    }
    wdt_off();
    uart_send_string("  Disabled OK\r\n");

    /* --- Mode: Interrupt (500ms, periodic) --- */
    uart_send_string("WDT Interrupt mode\r\n");
    wdt_set_callback(wdt_interrupt_handler);
    wdt_configure(WDT_TIMEOUT_500MS, WDT_MODE_INTERRUPT);

    for (uint8_t i = 0; i < 4; i++) {
        while (!wdt_isr_flag);   /* wait for ISR */
        wdt_isr_flag = 0;
        PINB |= (1 << PB5);     /* toggle LED */
        uart_send_string("  WDT ISR #");
        uart_print_u16(i + 1);
        uart_send_string("\r\n");
        /* Re-arm (WDIE auto-clears after ISR) */
        wdt_configure(WDT_TIMEOUT_500MS, WDT_MODE_INTERRUPT);
    }
    wdt_off();

    /* --- Mode: Interrupt + Reset (combo) --- */
    uart_send_string("WDT Int+Reset mode\r\n");
    wdt_set_callback(wdt_interrupt_handler);
    wdt_configure(WDT_TIMEOUT_2S, WDT_MODE_INT_RESET);

    /* First timeout fires ISR, second would reset */
    while (!wdt_isr_flag);
    wdt_isr_flag = 0;
    uart_send_string("  ISR fired, kicking...\r\n");
    wdt_kick();             /* prevent actual reset */
    wdt_off();

    uart_send_string("All tests passed\r\n");

    /* --- ForceReset demo (commented out to avoid reset loop) --- */
    /* uart_send_string("Forcing reset...\r\n");
       uart_flush_tx();
       wdt_force_reset(); */

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
