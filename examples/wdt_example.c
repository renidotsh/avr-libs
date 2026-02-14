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
    UART_Init(9600);
    sei();

    /* --- Get reset cause (call once at startup) --- */
    uint8_t cause = WDT_GetResetCause();
    UART_SendString("Reset cause: 0x");
    UART_PrintHex8(cause);
    UART_SendString("\r\n");

    if (cause & WDT_RESET_POWER_ON)
        UART_SendString("  Power-on\r\n");
    if (cause & WDT_RESET_EXTERNAL)
        UART_SendString("  External\r\n");
    if (cause & WDT_RESET_BROWN_OUT)
        UART_SendString("  Brown-out\r\n");
    if (cause & WDT_RESET_WATCHDOG)
        UART_SendString("  Watchdog\r\n");

    /* --- LED --- */
    DDRB |= (1 << PB5);

    /* --- Mode: System Reset (1s timeout) --- */
    UART_SendString("WDT Reset mode (1s)\r\n");
    WDT_Enable(WDT_TIMEOUT_1S, WDT_MODE_RESET);

    for (uint8_t i = 0; i < 3; i++) {
        WDT_Reset();            /* kick before timeout */
        UART_SendString("  kick\r\n");
        _delay_ms(500);
    }
    WDT_Disable();
    UART_SendString("  Disabled OK\r\n");

    /* --- Mode: Interrupt (500ms, periodic) --- */
    UART_SendString("WDT Interrupt mode\r\n");
    WDT_SetCallback(wdt_interrupt_handler);
    WDT_Enable(WDT_TIMEOUT_500MS, WDT_MODE_INTERRUPT);

    for (uint8_t i = 0; i < 4; i++) {
        while (!wdt_isr_flag);   /* wait for ISR */
        wdt_isr_flag = 0;
        PINB |= (1 << PB5);     /* toggle LED */
        UART_SendString("  WDT ISR #");
        UART_PrintU16(i + 1);
        UART_SendString("\r\n");
        /* Re-arm (WDIE auto-clears after ISR) */
        WDT_Enable(WDT_TIMEOUT_500MS, WDT_MODE_INTERRUPT);
    }
    WDT_Disable();

    /* --- Mode: Interrupt + Reset (combo) --- */
    UART_SendString("WDT Int+Reset mode\r\n");
    WDT_SetCallback(wdt_interrupt_handler);
    WDT_Enable(WDT_TIMEOUT_2S, WDT_MODE_INT_RESET);

    /* First timeout fires ISR, second would reset */
    while (!wdt_isr_flag);
    wdt_isr_flag = 0;
    UART_SendString("  ISR fired, kicking...\r\n");
    WDT_Reset();             /* prevent actual reset */
    WDT_Disable();

    UART_SendString("All tests passed\r\n");

    /* --- ForceReset demo (commented out to avoid reset loop) --- */
    /* UART_SendString("Forcing reset...\r\n");
       UART_FlushTx();
       WDT_ForceReset(); */

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
