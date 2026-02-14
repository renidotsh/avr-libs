/**
 * @file    gpio_example.c
 * @brief   ATmega328P example exercising every avr_gpio.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_GPIO_IMPLEMENTATION
#include "../avr_gpio.h"
#include <util/delay.h>

int main(void)
{
    /* --- Pin mode configuration --- */
    GPIO_SetMode(&DDRB, &PORTB, PB5, GPIO_OUTPUT);        /* LED           */
    GPIO_SetMode(&DDRD, &PORTD, PD2, GPIO_INPUT_PULLUP);  /* Button        */
    GPIO_SetMode(&DDRC, &PORTC, PC0, GPIO_INPUT);          /* Plain input   */

    /* --- Port-level bulk DDR --- */
    GPIO_SetPortMode(&DDRC, 0x0F);  /* PC0-3 output, PC4-7 input */

    /* --- Write & Read pin --- */
    GPIO_WritePin(&PORTB, PB5, 1);          /* LED ON  */
    _delay_ms(500);
    GPIO_WritePin(&PORTB, PB5, 0);          /* LED OFF */
    _delay_ms(500);

    uint8_t btn = GPIO_ReadPin(&PIND, PD2); /* read button */
    (void)btn;

    /* --- Toggle pin --- */
    GPIO_TogglePin(&PINB, PB5);
    _delay_ms(250);
    GPIO_TogglePin(&PINB, PB5);
    _delay_ms(250);

    /* --- Whole-port write & read --- */
    GPIO_WritePort(&PORTC, 0x0A);           /* pattern on lower nibble */
    uint8_t port_val = GPIO_ReadPort(&PINC);
    (void)port_val;

    /* --- Mask operations --- */
    GPIO_SetMask(&PORTC, 0x05);             /* set  bits 0,2 */
    GPIO_ClearMask(&PORTC, 0x0A);           /* clear bits 1,3 */
    GPIO_ToggleMask(&PINC, 0x0F);           /* toggle lower nibble */

    /* --- Macro-based direct access --- */
    GPIO_SET_HIGH(PORTB, PB5);
    _delay_ms(100);
    GPIO_SET_LOW(PORTB, PB5);
    GPIO_TOGGLE(PINB, PB5);
    uint8_t r = GPIO_READ(PIND, PD2);
    (void)r;

    /* --- Atomic block demo --- */
    GPIO_ATOMIC_BLOCK(
        PORTC |= (1 << PC0);
        PORTC &= ~(1 << PC1);
    );

    /* Main loop: blink LED + read button */
    while (1) {
        if (GPIO_ReadPin(&PIND, PD2) == 0) {
            GPIO_WritePort(&PORTC, 0x0F);
        } else {
            GPIO_ClearMask(&PORTC, 0x0F);
        }
        GPIO_TogglePin(&PINB, PB5);
        _delay_ms(250);
    }
    return 0;
}
