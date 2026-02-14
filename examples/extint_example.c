/**
 * @file    extint_example.c
 * @brief   ATmega328P example exercising every avr_extint.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_EXTINT_IMPLEMENTATION
#include "../avr_extint.h"
#include <avr/io.h>
#include <util/delay.h>

/* --- Callbacks --- */
static volatile uint8_t int0_flag = 0;
static volatile uint8_t pcint_flag = 0;

void int0_handler(void)
{
    int0_flag = 1;
    PINB |= (1 << PB5);  /* toggle LED immediately */
}

void int1_handler(void)
{
    PINB |= (1 << PB4);  /* toggle PB4 */
}

void pcint0_handler(void)
{
    pcint_flag = 1;
}

int main(void)
{
    /* Outputs */
    DDRB |= (1 << PB5) | (1 << PB4) | (1 << PB3);

    /* INT0 on PD2 – falling edge */
    DDRD  &= ~(1 << PD2);
    PORTD |=  (1 << PD2);  /* pull-up */
    EXTINT_Init(EXTINT_INT0, EXTINT_FALLING, int0_handler);

    /* INT1 on PD3 – rising edge */
    DDRD  &= ~(1 << PD3);
    PORTD |=  (1 << PD3);
    EXTINT_Init(EXTINT_INT1, EXTINT_RISING, int1_handler);

    /* PCINT group 0: PB0 (PCINT0) */
    DDRB  &= ~(1 << PB0);
    PORTB |=  (1 << PB0);
    PCINT_Init(PCINT_GROUP0, (1 << PCINT0), pcint0_handler);

    /* Enable additional pin in same group */
    DDRB  &= ~(1 << PB1);
    PORTB |=  (1 << PB1);
    PCINT_EnablePin(PCINT_GROUP0, PCINT1);

    sei();

    /* Main loop */
    while (1) {
        /* Process INT0 flag */
        if (int0_flag) {
            int0_flag = 0;
            /* Debounced re-read */
            uint8_t stable = EXTINT_Debounce(&PIND, PD2);
            if (stable == 0) {
                PINB |= (1 << PB3);  /* confirmed press */
            }
        }

        /* Process PCINT flag */
        if (pcint_flag) {
            pcint_flag = 0;
            PINB |= (1 << PB4); /* toggle on pin change */
        }

        _delay_ms(10);
    }

    /* --- Disable demos (unreachable, for API coverage) --- */
    EXTINT_Disable(EXTINT_INT0);
    EXTINT_Disable(EXTINT_INT1);
    PCINT_DisablePin(PCINT_GROUP0, PCINT1);
    PCINT_Disable(PCINT_GROUP0);

    return 0;
}
