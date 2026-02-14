/**
 * @file    timer_example.c
 * @brief   ATmega328P example exercising every avr_timer.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_TIMER_IMPLEMENTATION
#include "../avr_timer.h"
#include <avr/io.h>

/* --- Callbacks --- */
static volatile uint8_t ovf_count = 0;

static void timer2_ovf_cb(void)
{
    ovf_count++;
}

static void timer2_compa_cb(void)
{
    /* Toggle PB4 every compare match */
    PINB |= (1 << PB4);
}

static void timer1_compa_cb(void)
{
    /* Toggle PB3 every 100 ms */
    PINB |= (1 << PB3);
}

int main(void)
{
    /* LED outputs */
    DDRB |= (1 << PB5) | (1 << PB4) | (1 << PB3);

    /* --- System tick (1 ms via Timer0 CTC) --- */
    TIMER_InitSystemTick();
    sei();

    /* --- Timer1: CTC mode, fires every ~100 ms --- */
    /* OCR1A = (F_CPU / prescale / freq) - 1 = (16e6/256/10)-1 = 6249 */
    TIMER1_Init(TIMER_MODE_CTC, TIMER01_PRESCALE_256, 6249);
    TIMER1_SetCallbacks(/*ovf=*/0, /*compa=*/timer1_compa_cb);

    /* --- Timer2: Normal mode, overflow callback --- */
    TIMER2_Init(TIMER_MODE_NORMAL, TIMER2_PRESCALE_1024, 0);
    TIMER2_SetCallbacks(timer2_ovf_cb, 0);

    /* Also set Timer2 compare-match */
    TIMER2_Init(TIMER_MODE_CTC, TIMER2_PRESCALE_1024, 155);
    TIMER2_SetCallbacks(timer2_ovf_cb, timer2_compa_cb);

    /* --- DelayMs demo --- */
    PORTB |= (1 << PB5);
    TIMER_DelayMs(1000);  /* 1 second on */
    PORTB &= ~(1 << PB5);
    TIMER_DelayMs(500);

    /* --- DelayUs demo --- */
    PORTB |= (1 << PB5);
    TIMER_DelayUs(100);   /* 100 µs pulse */
    PORTB &= ~(1 << PB5);

    /* --- GetMillis demo --- */
    uint32_t start = TIMER_GetMillis();

    /* --- Stop Timer2 --- */
    TIMER_Stop(TIMER_2);

    /* --- Restart Timer2 --- */
    TIMER2_Init(TIMER_MODE_CTC, TIMER2_PRESCALE_1024, 155);
    TIMER2_SetCallbacks(0, timer2_compa_cb);

    /* Main loop */
    while (1) {
        uint32_t now = TIMER_GetMillis();
        if ((now - start) >= 2000) {
            PINB |= (1 << PB5);  /* toggle every 2 s */
            start = now;
        }
    }
    return 0;
}
