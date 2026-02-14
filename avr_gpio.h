/**
 * @file     avr_gpio.h
 * @brief    GPIO control library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Provides direct register-level control of GPIO pins with a simple and
 * efficient API.  Supports individual pin control and port-level operations
 * for high-speed I/O.  All functions compile to 1-3 instructions when
 * arguments are compile-time constants.
 *
 * @features
 * - Pin mode configuration (input / output / input-pullup)
 * - Atomic read / write operations
 * - Port-level bulk read / write
 * - Pin toggling via PINx write trick
 * - Bitmask helpers for multi-pin operations
 *
 * @example
 *   #define AVR_GPIO_IMPLEMENTATION
 *   #include "avr_gpio.h"
 *
 *   int main(void) {
 *       GPIO_SetMode(&DDRB, &PORTB, PB5, GPIO_OUTPUT);
 *       GPIO_WritePin(&PORTB, PB5, 1);
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_GPIO_H
#define AVR_GPIO_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>

/* ===== PUBLIC CONSTANTS ===== */

/** @brief Pin mode: high-impedance input (DDR=0, PORT=0) */
#define GPIO_INPUT          0

/** @brief Pin mode: totem-pole output (DDR=1) */
#define GPIO_OUTPUT         1

/** @brief Pin mode: input with internal pull-up enabled (DDR=0, PORT=1) */
#define GPIO_INPUT_PULLUP   2

/* ===== HELPER MACROS ===== */

/**
 * @brief  Quick pin operations when port is known at compile time.
 *         These resolve to single SBI/CBI instructions on most AVR cores.
 */
#define GPIO_SET_HIGH(port, pin)    do { (port) |=  (1U << (pin)); } while(0)
#define GPIO_SET_LOW(port, pin)     do { (port) &= ~(1U << (pin)); } while(0)
#define GPIO_TOGGLE(pinr, pin)      do { (pinr) |=  (1U << (pin)); } while(0)
#define GPIO_READ(pinr, pin)        (((pinr) >> (pin)) & 1U)

/**
 * @brief  Atomic wrapper – disables interrupts for a read-modify-write
 *         sequence, then restores SREG.
 */
#define GPIO_ATOMIC_BLOCK(code)                          \
    do {                                                 \
        uint8_t _sreg = SREG;                            \
        cli();                                           \
        { code; }                                        \
        SREG = _sreg;                                    \
    } while(0)

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Configure a single pin's direction and pull-up state.
 * @param  ddr   Pointer to DDRx register  (e.g. &DDRB)
 * @param  port  Pointer to PORTx register (e.g. &PORTB)
 * @param  pin   Pin number 0-7
 * @param  mode  GPIO_INPUT, GPIO_OUTPUT, or GPIO_INPUT_PULLUP
 * @note   Performs atomic read-modify-write (interrupts briefly disabled).
 */
void GPIO_SetMode(volatile uint8_t *ddr, volatile uint8_t *port,
                  uint8_t pin, uint8_t mode);

/**
 * @brief  Write a logic level to an output pin.
 * @param  port  Pointer to PORTx register
 * @param  pin   Pin number 0-7
 * @param  value Non-zero → HIGH, zero → LOW
 */
void GPIO_WritePin(volatile uint8_t *port, uint8_t pin, uint8_t value);

/**
 * @brief  Read the current logic level of a pin.
 * @param  pinr  Pointer to PINx register (e.g. &PINB)
 * @param  pin   Pin number 0-7
 * @return 1 if HIGH, 0 if LOW
 */
uint8_t GPIO_ReadPin(volatile uint8_t *pinr, uint8_t pin);

/**
 * @brief  Toggle an output pin (uses PINx write trick on modern AVR).
 * @param  pinr  Pointer to PINx register
 * @param  pin   Pin number 0-7
 */
void GPIO_TogglePin(volatile uint8_t *pinr, uint8_t pin);

/**
 * @brief  Write an entire 8-bit value to a port.
 * @param  port  Pointer to PORTx register
 * @param  value 8-bit value to write
 */
void GPIO_WritePort(volatile uint8_t *port, uint8_t value);

/**
 * @brief  Read an entire 8-bit port input register.
 * @param  pinr  Pointer to PINx register
 * @return 8-bit port value
 */
uint8_t GPIO_ReadPort(volatile uint8_t *pinr);

/**
 * @brief  Set the direction of every pin in a port at once.
 * @param  ddr   Pointer to DDRx register
 * @param  mask  Bitmask – 1 = output, 0 = input
 */
void GPIO_SetPortMode(volatile uint8_t *ddr, uint8_t mask);

/**
 * @brief  Atomically set multiple pins HIGH using a bitmask.
 * @param  port  Pointer to PORTx register
 * @param  mask  Bitmask of pins to set
 */
void GPIO_SetMask(volatile uint8_t *port, uint8_t mask);

/**
 * @brief  Atomically clear multiple pins LOW using a bitmask.
 * @param  port  Pointer to PORTx register
 * @param  mask  Bitmask of pins to clear
 */
void GPIO_ClearMask(volatile uint8_t *port, uint8_t mask);

/**
 * @brief  Toggle multiple pins at once.
 * @param  pinr  Pointer to PINx register
 * @param  mask  Bitmask of pins to toggle
 */
void GPIO_ToggleMask(volatile uint8_t *pinr, uint8_t mask);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_GPIO_IMPLEMENTATION

void GPIO_SetMode(volatile uint8_t *ddr, volatile uint8_t *port,
                  uint8_t pin, uint8_t mode)
{
    GPIO_ATOMIC_BLOCK(
        switch (mode) {
            case GPIO_OUTPUT:
                *ddr  |=  (1U << pin);          /* direction = output */
                break;
            case GPIO_INPUT_PULLUP:
                *ddr  &= ~(1U << pin);          /* direction = input  */
                *port |=  (1U << pin);          /* pull-up ON         */
                break;
            case GPIO_INPUT:  /* fall-through */
            default:
                *ddr  &= ~(1U << pin);          /* direction = input  */
                *port &= ~(1U << pin);          /* pull-up OFF        */
                break;
        }
    );
}

void GPIO_WritePin(volatile uint8_t *port, uint8_t pin, uint8_t value)
{
    GPIO_ATOMIC_BLOCK(
        if (value)
            *port |=  (1U << pin);
        else
            *port &= ~(1U << pin);
    );
}

uint8_t GPIO_ReadPin(volatile uint8_t *pinr, uint8_t pin)
{
    return (*pinr >> pin) & 1U;
}

void GPIO_TogglePin(volatile uint8_t *pinr, uint8_t pin)
{
    /* Writing 1 to a PINx bit toggles the corresponding PORTx bit
       on ATmega48A/PA/88A/PA/168A/PA/328/P and many newer AVRs. */
    *pinr = (1U << pin);
}

void GPIO_WritePort(volatile uint8_t *port, uint8_t value)
{
    *port = value;
}

uint8_t GPIO_ReadPort(volatile uint8_t *pinr)
{
    return *pinr;
}

void GPIO_SetPortMode(volatile uint8_t *ddr, uint8_t mask)
{
    *ddr = mask;
}

void GPIO_SetMask(volatile uint8_t *port, uint8_t mask)
{
    GPIO_ATOMIC_BLOCK( *port |= mask; );
}

void GPIO_ClearMask(volatile uint8_t *port, uint8_t mask)
{
    GPIO_ATOMIC_BLOCK( *port &= ~mask; );
}

void GPIO_ToggleMask(volatile uint8_t *pinr, uint8_t mask)
{
    *pinr = mask;   /* PINx write trick – toggles corresponding bits */
}

#endif /* AVR_GPIO_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
/*
 * Blink LED on PB5 (Arduino Uno on-board LED).
 * Read button on PD2 with pull-up.
 */
#define F_CPU 16000000UL
#define AVR_GPIO_IMPLEMENTATION
#include "avr_gpio.h"
#include <util/delay.h>

int main(void)
{
    /* LED output */
    GPIO_SetMode(&DDRB, &PORTB, PB5, GPIO_OUTPUT);

    /* Button input with pull-up */
    GPIO_SetMode(&DDRD, &PORTD, PD2, GPIO_INPUT_PULLUP);

    /* Port-level: set all of PORTC as outputs */
    GPIO_SetPortMode(&DDRC, 0xFF);

    while (1) {
        /* Toggle LED */
        GPIO_TogglePin(&PINB, PB5);

        /* Read button (active-low) */
        if (GPIO_ReadPin(&PIND, PD2) == 0) {
            GPIO_WritePort(&PORTC, 0xAA);   /* pattern on PORTC */
        } else {
            GPIO_ClearMask(&PORTC, 0xFF);
        }

        _delay_ms(250);
    }
    return 0;
}
#endif

#endif /* AVR_GPIO_H */
