/**
 * @file    power_example.c
 * @brief   ATmega328P example exercising every avr_power.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_POWER_IMPLEMENTATION
#include "../avr_power.h"
#define AVR_WDT_IMPLEMENTATION
#include "../avr_wdt.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"
#include <util/delay.h>

static volatile uint8_t wakeup_flag = 0;

void wdt_wakeup(void)
{
    wakeup_flag = 1;
}

int main(void)
{
    /* Re-enable USART peripheral (in case PRR left set after reset) */
    POWER_EnableAllPeripherals();

    UART_Init(9600);
    sei();
    UART_SendString("Power Demo\r\n");

    DDRB |= (1 << PB5);  /* LED */

    /* --- Disable unused peripherals --- */
    POWER_DisablePeripheral(POWER_ADC);
    POWER_DisablePeripheral(POWER_SPI);
    POWER_DisablePeripheral(POWER_TWI);
    POWER_DisablePeripheral(POWER_TIMER1);
    POWER_DisablePeripheral(POWER_TIMER2);

    /* --- Check peripheral status --- */
    UART_SendString("ADC enabled: ");
    UART_SendString(POWER_IsPeripheralEnabled(POWER_ADC) ? "yes" : "no");
    UART_SendString("\r\n");
    UART_SendString("USART enabled: ");
    UART_SendString(POWER_IsPeripheralEnabled(POWER_USART) ? "yes" : "no");
    UART_SendString("\r\n");

    /* --- Idle sleep demo (wakes on any interrupt, e.g. UART RX) --- */
    UART_SendString("Idle sleep 2s...\r\n");
    UART_FlushTx();
    WDT_SetCallback(wdt_wakeup);
    WDT_Enable(WDT_TIMEOUT_2S, WDT_MODE_INTERRUPT);
    POWER_Sleep(POWER_SLEEP_IDLE);
    WDT_Disable();
    UART_SendString("Woke from Idle\r\n");

    /* --- SetSleepMode + EnterSleep separately --- */
    UART_SendString("Power-down 2s...\r\n");
    UART_FlushTx();
    wakeup_flag = 0;
    WDT_SetCallback(wdt_wakeup);
    WDT_Enable(WDT_TIMEOUT_2S, WDT_MODE_INTERRUPT);
    POWER_SetSleepMode(POWER_SLEEP_POWER_DOWN);
    POWER_EnterSleep();
    WDT_Disable();
    UART_SendString("Woke from Power-down\r\n");

    /* --- DeepSleep (BOD disabled) --- */
    UART_SendString("Deep sleep 4s...\r\n");
    UART_FlushTx();
    wakeup_flag = 0;
    WDT_SetCallback(wdt_wakeup);
    WDT_Enable(WDT_TIMEOUT_4S, WDT_MODE_INTERRUPT);
    POWER_DeepSleep();
    WDT_Disable();
    UART_SendString("Woke from Deep sleep\r\n");

    /* --- Re-enable peripherals --- */
    POWER_EnablePeripheral(POWER_ADC);
    UART_SendString("ADC re-enabled: ");
    UART_SendString(POWER_IsPeripheralEnabled(POWER_ADC) ? "yes" : "no");
    UART_SendString("\r\n");

    /* --- Enable all --- */
    POWER_EnableAllPeripherals();
    UART_SendString("All peripherals enabled\r\n");

    /* --- Disable all (then re-enable what we need) --- */
    POWER_DisableAllPeripherals();
    POWER_EnablePeripheral(POWER_USART);
    POWER_EnablePeripheral(POWER_TIMER0);
    UART_SendString("Selective enable OK\r\n");

    POWER_EnableAllPeripherals();  /* restore for clean exit */

    UART_SendString("Power demo complete\r\n");
    while (1) {
        PINB |= (1 << PB5);
        _delay_ms(1000);
    }
    return 0;
}
