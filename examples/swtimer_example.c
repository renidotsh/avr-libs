/**
 * @file    swtimer_example.c
 * @brief   ATmega328P example exercising every avr_swtimer.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_SWTIMER_IMPLEMENTATION
#include "../avr_swtimer.h"
#define AVR_TIMER_IMPLEMENTATION
#include "../avr_timer.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"

/* --- Hardware tick: Timer0 compare-match drives SWTIMER_Tick --- */
static void hw_tick(void)
{
    SWTIMER_Tick();
}

/* --- Callbacks --- */
static volatile uint16_t blink_count = 0;

void blink_cb(void)
{
    PINB |= (1 << PB5);  /* toggle LED */
    blink_count++;
}

void one_shot_cb(void)
{
    UART_SendString("One-shot fired!\r\n");
}

void fast_cb(void)
{
    PINB |= (1 << PB4);  /* toggle PB4 */
}

int main(void)
{
    DDRB |= (1 << PB5) | (1 << PB4);
    UART_Init(9600);
    sei();
    UART_SendString("SwTimer Demo\r\n");

    /* --- Init --- */
    SWTIMER_Init();

    /* --- 1 ms HW tick on Timer0 --- */
    TIMER_InitSystemTick();
    TIMER0_SetCallbacks(0, hw_tick);

    /* --- Create periodic timer: 500 ms blink --- */
    swtimer_handle_t blink_h = SWTIMER_Create(500, blink_cb, false);
    UART_SendString("Blink timer handle: ");
    UART_PrintU16(blink_h);
    UART_SendString("\r\n");

    /* --- Create one-shot timer: 3000 ms --- */
    swtimer_handle_t oneshot_h = SWTIMER_Create(3000, one_shot_cb, true);

    /* --- Create a fast timer: 100 ms --- */
    swtimer_handle_t fast_h = SWTIMER_Create(100, fast_cb, false);

    /* --- ActiveCount --- */
    UART_SendString("Active timers: ");
    UART_PrintU16(SWTIMER_ActiveCount());
    UART_SendString("\r\n");

    /* --- IsRunning --- */
    UART_SendString("Blink running: ");
    UART_SendString(SWTIMER_IsRunning(blink_h) ? "yes" : "no");
    UART_SendString("\r\n");

    /* Run for ~5 seconds */
    for (uint16_t i = 0; i < 500; i++) {
        SWTIMER_Process();
        /* Approximate 10 ms idle */
        for (volatile uint16_t d = 0; d < 4000; d++);
    }

    /* --- Remaining time --- */
    uint16_t rem = SWTIMER_Remaining(blink_h);
    UART_SendString("Blink remaining: ");
    UART_PrintU16(rem);
    UART_SendString(" ms\r\n");

    /* --- Stop timer --- */
    SWTIMER_Stop(fast_h);
    UART_SendString("Fast stopped, running: ");
    UART_SendString(SWTIMER_IsRunning(fast_h) ? "yes" : "no");
    UART_SendString("\r\n");

    /* --- Restart timer --- */
    SWTIMER_Restart(fast_h);
    UART_SendString("Fast restarted\r\n");

    /* --- SetPeriod --- */
    SWTIMER_SetPeriod(blink_h, 1000);  /* change to 1 s */
    UART_SendString("Blink period -> 1000 ms\r\n");

    /* --- Delete one-shot (already fired by now) --- */
    SWTIMER_Delete(oneshot_h);
    UART_SendString("Active timers after delete: ");
    UART_PrintU16(SWTIMER_ActiveCount());
    UART_SendString("\r\n");

    /* --- Delete fast timer --- */
    SWTIMER_Delete(fast_h);

    UART_SendString("Blink count so far: ");
    UART_PrintU16(blink_count);
    UART_SendString("\r\n");

    /* Continue indefinitely with just blink */
    UART_SendString("Running blink only...\r\n");
    while (1) {
        SWTIMER_Process();
    }
    return 0;
}
