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

/* --- Hardware tick: Timer0 compare-match drives swtimer_tick --- */
static void hw_tick(void)
{
    swtimer_tick();
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
    uart_send_string("One-shot fired!\r\n");
}

void fast_cb(void)
{
    PINB |= (1 << PB4);  /* toggle PB4 */
}

int main(void)
{
    DDRB |= (1 << PB5) | (1 << PB4);
    uart_init(9600);
    sei();
    uart_send_string("SwTimer Demo\r\n");

    /* --- Init --- */
    swtimer_init();

    /* --- 1 ms HW tick on Timer0 --- */
    timer_init_system_tick();
    timer0_set_callbacks(0, hw_tick);

    /* --- Create periodic timer: 500 ms blink --- */
    swtimer_handle_t blink_h = swtimer_create(500, blink_cb, false);
    uart_send_string("Blink timer handle: ");
    uart_print_u16(blink_h);
    uart_send_string("\r\n");

    /* --- Create one-shot timer: 3000 ms --- */
    swtimer_handle_t oneshot_h = swtimer_create(3000, one_shot_cb, true);

    /* --- Create a fast timer: 100 ms --- */
    swtimer_handle_t fast_h = swtimer_create(100, fast_cb, false);

    /* --- ActiveCount --- */
    uart_send_string("Active timers: ");
    uart_print_u16(swtimer_active_count());
    uart_send_string("\r\n");

    /* --- IsRunning --- */
    uart_send_string("Blink running: ");
    uart_send_string(swtimer_is_running(blink_h) ? "yes" : "no");
    uart_send_string("\r\n");

    /* Run for ~5 seconds */
    for (uint16_t i = 0; i < 500; i++) {
        swtimer_process();
        /* Approximate 10 ms idle */
        for (volatile uint16_t d = 0; d < 4000; d++);
    }

    /* --- Remaining time --- */
    uint16_t rem = swtimer_remaining(blink_h);
    uart_send_string("Blink remaining: ");
    uart_print_u16(rem);
    uart_send_string(" ms\r\n");

    /* --- Stop timer --- */
    swtimer_stop(fast_h);
    uart_send_string("Fast stopped, running: ");
    uart_send_string(swtimer_is_running(fast_h) ? "yes" : "no");
    uart_send_string("\r\n");

    /* --- Restart timer --- */
    swtimer_restart(fast_h);
    uart_send_string("Fast restarted\r\n");

    /* --- SetPeriod --- */
    swtimer_set_period(blink_h, 1000);  /* change to 1 s */
    uart_send_string("Blink period -> 1000 ms\r\n");

    /* --- Delete one-shot (already fired by now) --- */
    swtimer_delete(oneshot_h);
    uart_send_string("Active timers after delete: ");
    uart_print_u16(swtimer_active_count());
    uart_send_string("\r\n");

    /* --- Delete fast timer --- */
    swtimer_delete(fast_h);

    uart_send_string("Blink count so far: ");
    uart_print_u16(blink_count);
    uart_send_string("\r\n");

    /* Continue indefinitely with just blink */
    uart_send_string("Running blink only...\r\n");
    while (1) {
        swtimer_process();
    }
    return 0;
}
