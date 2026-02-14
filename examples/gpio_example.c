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
    gpio_set_mode(&DDRB, &PORTB, PB5, GPIO_OUTPUT);        /* LED           */
    gpio_set_mode(&DDRD, &PORTD, PD2, GPIO_INPUT_PULLUP);  /* Button        */
    gpio_set_mode(&DDRC, &PORTC, PC0, GPIO_INPUT);          /* Plain input   */

    /* --- Port-level bulk DDR --- */
    gpio_set_port_mode(&DDRC, 0x0F);  /* PC0-3 output, PC4-7 input */

    /* --- Write & Read pin --- */
    gpio_write_pin(&PORTB, PB5, 1);          /* LED ON  */
    _delay_ms(500);
    gpio_write_pin(&PORTB, PB5, 0);          /* LED OFF */
    _delay_ms(500);

    uint8_t btn = gpio_read_pin(&PIND, PD2); /* read button */
    (void)btn;

    /* --- Toggle pin --- */
    gpio_toggle_pin(&PINB, PB5);
    _delay_ms(250);
    gpio_toggle_pin(&PINB, PB5);
    _delay_ms(250);

    /* --- Whole-port write & read --- */
    gpio_write_port(&PORTC, 0x0A);           /* pattern on lower nibble */
    uint8_t port_val = gpio_read_port(&PINC);
    (void)port_val;

    /* --- Mask operations --- */
    gpio_set_mask(&PORTC, 0x05);             /* set  bits 0,2 */
    gpio_clear_mask(&PORTC, 0x0A);           /* clear bits 1,3 */
    gpio_toggle_mask(&PINC, 0x0F);           /* toggle lower nibble */

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
        if (gpio_read_pin(&PIND, PD2) == 0) {
            gpio_write_port(&PORTC, 0x0F);
        } else {
            gpio_clear_mask(&PORTC, 0x0F);
        }
        gpio_toggle_pin(&PINB, PB5);
        _delay_ms(250);
    }
    return 0;
}
