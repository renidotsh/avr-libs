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
    power_enable_all_peripherals();

    uart_init(9600);
    sei();
    uart_send_string("Power Demo\r\n");

    DDRB |= (1 << PB5);  /* LED */

    /* --- Disable unused peripherals --- */
    power_disable_peripheral(POWER_ADC);
    power_disable_peripheral(POWER_SPI);
    power_disable_peripheral(POWER_TWI);
    power_disable_peripheral(POWER_TIMER1);
    power_disable_peripheral(POWER_TIMER2);

    /* --- Check peripheral status --- */
    uart_send_string("ADC enabled: ");
    uart_send_string(power_is_peripheral_enabled(POWER_ADC) ? "yes" : "no");
    uart_send_string("\r\n");
    uart_send_string("USART enabled: ");
    uart_send_string(power_is_peripheral_enabled(POWER_USART) ? "yes" : "no");
    uart_send_string("\r\n");

    /* --- Idle sleep demo (wakes on any interrupt, e.g. UART RX) --- */
    uart_send_string("Idle sleep 2s...\r\n");
    uart_flush_tx();
    wdt_set_callback(wdt_wakeup);
    wdt_configure(WDT_TIMEOUT_2S, WDT_MODE_INTERRUPT);
    power_sleep(POWER_SLEEP_IDLE);
    wdt_off();
    uart_send_string("Woke from Idle\r\n");

    /* --- SetSleepMode + EnterSleep separately --- */
    uart_send_string("Power-down 2s...\r\n");
    uart_flush_tx();
    wakeup_flag = 0;
    wdt_set_callback(wdt_wakeup);
    wdt_configure(WDT_TIMEOUT_2S, WDT_MODE_INTERRUPT);
    power_set_sleep_mode(POWER_SLEEP_POWER_DOWN);
    power_enter_sleep();
    wdt_off();
    uart_send_string("Woke from Power-down\r\n");

    /* --- DeepSleep (BOD disabled) --- */
    uart_send_string("Deep sleep 4s...\r\n");
    uart_flush_tx();
    wakeup_flag = 0;
    wdt_set_callback(wdt_wakeup);
    wdt_configure(WDT_TIMEOUT_4S, WDT_MODE_INTERRUPT);
    power_deep_sleep();
    wdt_off();
    uart_send_string("Woke from Deep sleep\r\n");

    /* --- Re-enable peripherals --- */
    power_enable_peripheral(POWER_ADC);
    uart_send_string("ADC re-enabled: ");
    uart_send_string(power_is_peripheral_enabled(POWER_ADC) ? "yes" : "no");
    uart_send_string("\r\n");

    /* --- Enable all --- */
    power_enable_all_peripherals();
    uart_send_string("All peripherals enabled\r\n");

    /* --- Disable all (then re-enable what we need) --- */
    power_disable_all_peripherals();
    power_enable_peripheral(POWER_USART);
    power_enable_peripheral(POWER_TIMER0);
    uart_send_string("Selective enable OK\r\n");

    power_enable_all_peripherals();  /* restore for clean exit */

    uart_send_string("Power demo complete\r\n");
    while (1) {
        PINB |= (1 << PB5);
        _delay_ms(1000);
    }
    return 0;
}
