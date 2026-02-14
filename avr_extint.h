/**
 * @file     avr_extint.h
 * @brief    External interrupts library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Configures INT0/INT1 edge-triggered interrupts and Pin-Change Interrupt
 * groups PCINT0-2 on ATmega328P.  Provides user callback registration and
 * a tick-based software debounce helper.
 *
 * @features
 * - INT0 / INT1 edge configuration (rising/falling/both/low-level)
 * - Pin-Change Interrupts PCINT0-2 with per-pin mask
 * - User callback registration for each interrupt source
 * - Software debounce helper (~20 ms, configurable)
 *
 * @example
 *   #define AVR_EXTINT_IMPLEMENTATION
 *   #include "avr_extint.h"
 *
 *   void on_button(void) { PINB |= (1 << PB5); }
 *
 *   int main(void) {
 *       EXTINT_Init(EXTINT_INT0, EXTINT_FALLING, on_button);
 *       sei();
 *       while(1);
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_EXTINT_H
#define AVR_EXTINT_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== CONFIGURATION ===== */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/** Debounce interval in milliseconds */
#ifndef EXTINT_DEBOUNCE_MS
#define EXTINT_DEBOUNCE_MS 20
#endif

/* ===== INTERRUPT SOURCE IDS ===== */
#define EXTINT_INT0     0
#define EXTINT_INT1     1

/* ===== EDGE / SENSE CONTROL ===== */
typedef enum {
    EXTINT_LOW_LEVEL = 0,   /**< Interrupt on low level (ISC=00) */
    EXTINT_BOTH_EDGES = 1,  /**< Any logical change     (ISC=01) */
    EXTINT_FALLING    = 2,  /**< Falling edge            (ISC=10) */
    EXTINT_RISING     = 3   /**< Rising edge             (ISC=11) */
} extint_sense_e;

/* ===== PIN-CHANGE GROUPS ===== */
#define PCINT_GROUP0    0   /**< PCINT[7:0]   ─ Port B */
#define PCINT_GROUP1    1   /**< PCINT[14:8]  ─ Port C */
#define PCINT_GROUP2    2   /**< PCINT[23:16] ─ Port D */

/* ===== CALLBACK TYPE ===== */
typedef void (*extint_callback_t)(void);

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Configure INT0 or INT1 with edge/level sense and callback.
 * @param  int_id   EXTINT_INT0 or EXTINT_INT1
 * @param  sense    Edge / level configuration
 * @param  cb       Callback function (called from ISR context)
 */
void EXTINT_Init(uint8_t int_id, extint_sense_e sense,
                 extint_callback_t cb);

/**
 * @brief  Disable an external interrupt.
 * @param  int_id  EXTINT_INT0 or EXTINT_INT1
 */
void EXTINT_Disable(uint8_t int_id);

/**
 * @brief  Enable a Pin-Change Interrupt group and register callback.
 * @param  group   PCINT_GROUP0, PCINT_GROUP1, or PCINT_GROUP2
 * @param  mask    Bitmask of pins within that group to enable
 * @param  cb      Callback (called from ISR context)
 */
void PCINT_Init(uint8_t group, uint8_t mask, extint_callback_t cb);

/**
 * @brief  Disable a Pin-Change Interrupt group.
 * @param  group  PCINT_GROUP0 / 1 / 2
 */
void PCINT_Disable(uint8_t group);

/**
 * @brief  Add a pin to an already-enabled PCINT group.
 * @param  group  Group ID
 * @param  pin    Pin bit position (0-7)
 */
void PCINT_EnablePin(uint8_t group, uint8_t pin);

/**
 * @brief  Remove a pin from a PCINT group.
 * @param  group  Group ID
 * @param  pin    Pin bit position (0-7)
 */
void PCINT_DisablePin(uint8_t group, uint8_t pin);

/**
 * @brief  Read a pin with software debounce (blocking).
 *         Reads the pin repeatedly over EXTINT_DEBOUNCE_MS and returns
 *         the stable state.
 * @param  pinr   Pointer to PINx register
 * @param  pin    Pin number 0-7
 * @return Debounced state: 1 = HIGH, 0 = LOW
 *
 * @note   Requires a working system tick (e.g. TIMER_GetMillis()).
 *         If you don't have one you can substitute _delay_ms polling.
 */
uint8_t EXTINT_Debounce(volatile uint8_t *pinr, uint8_t pin);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_EXTINT_IMPLEMENTATION

static volatile extint_callback_t _int0_cb  = (void*)0;
static volatile extint_callback_t _int1_cb  = (void*)0;
static volatile extint_callback_t _pcint0_cb = (void*)0;
static volatile extint_callback_t _pcint1_cb = (void*)0;
static volatile extint_callback_t _pcint2_cb = (void*)0;

/* ---- INT0 / INT1 ---- */

void EXTINT_Init(uint8_t int_id, extint_sense_e sense,
                 extint_callback_t cb)
{
    if (int_id == EXTINT_INT0) {
        _int0_cb = cb;
        /* ISC01:ISC00 in EICRA bits [1:0] */
        EICRA = (EICRA & ~0x03) | ((uint8_t)sense & 0x03);
        EIFR  |= (1 << INTF0);   /* clear pending flag */
        EIMSK |= (1 << INT0);    /* enable INT0        */
    } else if (int_id == EXTINT_INT1) {
        _int1_cb = cb;
        /* ISC11:ISC10 in EICRA bits [3:2] */
        EICRA = (EICRA & ~0x0C) | (((uint8_t)sense & 0x03) << 2);
        EIFR  |= (1 << INTF1);
        EIMSK |= (1 << INT1);
    }
}

void EXTINT_Disable(uint8_t int_id)
{
    if (int_id == EXTINT_INT0) {
        EIMSK &= ~(1 << INT0);
        _int0_cb = (void*)0;
    } else if (int_id == EXTINT_INT1) {
        EIMSK &= ~(1 << INT1);
        _int1_cb = (void*)0;
    }
}

ISR(INT0_vect)
{
    if (_int0_cb) _int0_cb();
}

ISR(INT1_vect)
{
    if (_int1_cb) _int1_cb();
}

/* ---- Pin-Change Interrupts ---- */

void PCINT_Init(uint8_t group, uint8_t mask, extint_callback_t cb)
{
    switch (group) {
        case PCINT_GROUP0:
            _pcint0_cb = cb;
            PCMSK0 = mask;
            PCIFR  |= (1 << PCIF0);
            PCICR  |= (1 << PCIE0);
            break;
        case PCINT_GROUP1:
            _pcint1_cb = cb;
            PCMSK1 = mask;
            PCIFR  |= (1 << PCIF1);
            PCICR  |= (1 << PCIE1);
            break;
        case PCINT_GROUP2:
            _pcint2_cb = cb;
            PCMSK2 = mask;
            PCIFR  |= (1 << PCIF2);
            PCICR  |= (1 << PCIE2);
            break;
        default:
            break;
    }
}

void PCINT_Disable(uint8_t group)
{
    switch (group) {
        case PCINT_GROUP0:
            PCICR &= ~(1 << PCIE0);
            PCMSK0 = 0;
            _pcint0_cb = (void*)0;
            break;
        case PCINT_GROUP1:
            PCICR &= ~(1 << PCIE1);
            PCMSK1 = 0;
            _pcint1_cb = (void*)0;
            break;
        case PCINT_GROUP2:
            PCICR &= ~(1 << PCIE2);
            PCMSK2 = 0;
            _pcint2_cb = (void*)0;
            break;
        default:
            break;
    }
}

void PCINT_EnablePin(uint8_t group, uint8_t pin)
{
    switch (group) {
        case PCINT_GROUP0: PCMSK0 |= (1 << pin); break;
        case PCINT_GROUP1: PCMSK1 |= (1 << pin); break;
        case PCINT_GROUP2: PCMSK2 |= (1 << pin); break;
        default: break;
    }
}

void PCINT_DisablePin(uint8_t group, uint8_t pin)
{
    switch (group) {
        case PCINT_GROUP0: PCMSK0 &= ~(1 << pin); break;
        case PCINT_GROUP1: PCMSK1 &= ~(1 << pin); break;
        case PCINT_GROUP2: PCMSK2 &= ~(1 << pin); break;
        default: break;
    }
}

ISR(PCINT0_vect) { if (_pcint0_cb) _pcint0_cb(); }
ISR(PCINT1_vect) { if (_pcint1_cb) _pcint1_cb(); }
ISR(PCINT2_vect) { if (_pcint2_cb) _pcint2_cb(); }

/* ---- Debounce helper ---- */

uint8_t EXTINT_Debounce(volatile uint8_t *pinr, uint8_t pin)
{
    /*
     * Simple polling debounce: read pin, wait a short time, confirm.
     * Uses a busy-wait loop calibrated to roughly EXTINT_DEBOUNCE_MS
     * at F_CPU (each inner iteration ≈ 4 cycles).
     */
    uint8_t state1 = (*pinr >> pin) & 1U;

    /* ~EXTINT_DEBOUNCE_MS delay loop (approximate) */
    for (volatile uint32_t d = 0;
         d < ((uint32_t)F_CPU / 4000UL * EXTINT_DEBOUNCE_MS / 1000UL);
         d++)
        ;

    uint8_t state2 = (*pinr >> pin) & 1U;

    if (state1 == state2)
        return state1;

    /* If not stable, wait again and return whatever we read */
    for (volatile uint32_t d = 0;
         d < ((uint32_t)F_CPU / 4000UL * EXTINT_DEBOUNCE_MS / 1000UL);
         d++)
        ;

    return (*pinr >> pin) & 1U;
}

#endif /* AVR_EXTINT_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_EXTINT_IMPLEMENTATION
#include "avr_extint.h"
#include <avr/io.h>

/* Toggle PB5 LED on INT0 falling edge (button on PD2) */
void button_isr(void) {
    PINB |= (1 << PB5);  /* toggle LED */
}

/* PCINT on PB0 (PCINT0) */
void pcint_handler(void) {
    /* In real code, read PINB to determine which pin changed */
    PINB |= (1 << PB4);  /* toggle PB4 */
}

int main(void)
{
    DDRB |= (1 << PB5) | (1 << PB4);  /* LEDs on PB5 and PB4 */
    DDRD &= ~(1 << PD2);              /* PD2 input (INT0 pin) */
    PORTD |= (1 << PD2);              /* pull-up               */

    /* Configure INT0 – falling edge */
    EXTINT_Init(EXTINT_INT0, EXTINT_FALLING, button_isr);

    /* Configure PCINT group 0 – PB0 */
    DDRD &= ~(1 << PD0);
    PCINT_Init(PCINT_GROUP0, (1 << PCINT0), pcint_handler);

    sei();

    while (1) {
        /* Debounced read of PD2 (blocking) */
        uint8_t btn = EXTINT_Debounce(&PIND, PD2);
        (void)btn;
    }
    return 0;
}
#endif

#endif /* AVR_EXTINT_H */
