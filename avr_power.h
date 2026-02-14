/**
 * @file     avr_power.h
 * @brief    Sleep & power management library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Controls AVR sleep modes (Idle, ADC Noise Reduction, Power-down,
 * Power-save, Standby) and the Power Reduction Register (PRR) for
 * selectively disabling unused peripherals to minimise current draw.
 * Includes BOD-disable helper for deep sleep.
 *
 * @features
 * - Sleep mode selection (Idle / ADC NR / Power-down / Power-save / Standby)
 * - Enter and wake from sleep
 * - Peripheral power reduction (PRR bits)
 * - BOD disable during Power-down
 * - Wake-up source documentation helpers
 *
 * @example
 *   #define AVR_POWER_IMPLEMENTATION
 *   #include "avr_power.h"
 *
 *   int main(void) {
 *       POWER_DisablePeripheral(POWER_ADC);
 *       POWER_SetSleepMode(POWER_SLEEP_IDLE);
 *       sei();
 *       POWER_EnterSleep();
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_POWER_H
#define AVR_POWER_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== SLEEP MODES ===== */
typedef enum {
    POWER_SLEEP_IDLE        = SLEEP_MODE_IDLE,
#ifdef SLEEP_MODE_ADC
    POWER_SLEEP_ADC_NR      = SLEEP_MODE_ADC,
#endif
    POWER_SLEEP_POWER_DOWN  = SLEEP_MODE_PWR_DOWN,
#ifdef SLEEP_MODE_PWR_SAVE
    POWER_SLEEP_POWER_SAVE  = SLEEP_MODE_PWR_SAVE,
#endif
#ifdef SLEEP_MODE_STANDBY
    POWER_SLEEP_STANDBY     = SLEEP_MODE_STANDBY,
#endif
#ifdef SLEEP_MODE_EXT_STANDBY
    POWER_SLEEP_EXT_STANDBY = SLEEP_MODE_EXT_STANDBY,
#endif
} power_sleep_e;

/* ===== PERIPHERAL IDS (PRR bits on ATmega328P) ===== */
typedef enum {
    POWER_ADC   = 0,   /**< ADC                        (PRADC)   */
    POWER_USART = 1,   /**< USART0                     (PRUSART0)*/
    POWER_SPI   = 2,   /**< SPI                        (PRSPI)   */
    POWER_TIMER1 = 3,  /**< Timer/Counter 1            (PRTIM1)  */
    POWER_TIMER0 = 5,  /**< Timer/Counter 0            (PRTIM0)  */
    POWER_TIMER2 = 6,  /**< Timer/Counter 2            (PRTIM2)  */
    POWER_TWI   = 7    /**< I2C/TWI                    (PRTWI)   */
} power_periph_e;

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Select a sleep mode (does NOT enter sleep yet).
 * @param  mode  One of the POWER_SLEEP_* values
 */
void POWER_SetSleepMode(power_sleep_e mode);

/**
 * @brief  Enter the currently-selected sleep mode.
 *         CPU halts until a configured wake-up interrupt fires.
 * @note   Interrupts must be globally enabled (sei()) before calling.
 */
void POWER_EnterSleep(void);

/**
 * @brief  Convenience: set mode + enter sleep in one call.
 * @param  mode  Sleep mode
 */
void POWER_Sleep(power_sleep_e mode);

/**
 * @brief  Enter Power-down sleep with Brown-Out Detector disabled.
 *         Achieves lowest possible current draw.
 * @note   Only effective on devices with software BOD disable
 *         (ATmega328P and similar).
 */
void POWER_DeepSleep(void);

/**
 * @brief  Disable a peripheral via the Power Reduction Register.
 *         Peripheral clock is gated off, register access becomes
 *         undefined until re-enabled.
 * @param  periph  Peripheral to disable (POWER_ADC, POWER_SPI, etc.)
 */
void POWER_DisablePeripheral(power_periph_e periph);

/**
 * @brief  Re-enable a peripheral that was disabled via PRR.
 * @param  periph  Peripheral to enable
 */
void POWER_EnablePeripheral(power_periph_e periph);

/**
 * @brief  Disable all peripherals (maximum power savings).
 *         Call POWER_EnablePeripheral() for each peripheral you need.
 */
void POWER_DisableAllPeripherals(void);

/**
 * @brief  Enable all peripherals (default state after reset).
 */
void POWER_EnableAllPeripherals(void);

/**
 * @brief  Check if a peripheral is currently enabled.
 * @return true if enabled
 */
bool POWER_IsPeripheralEnabled(power_periph_e periph);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_POWER_IMPLEMENTATION

void POWER_SetSleepMode(power_sleep_e mode)
{
    set_sleep_mode((uint8_t)mode);
}

void POWER_EnterSleep(void)
{
    sleep_enable();
    sei();           /* ensure interrupts are on */
    sleep_cpu();     /* Zzz … wakes here after ISR */
    sleep_disable(); /* disable sleep immediately after waking */
}

void POWER_Sleep(power_sleep_e mode)
{
    POWER_SetSleepMode(mode);
    POWER_EnterSleep();
}

void POWER_DeepSleep(void)
{
    POWER_SetSleepMode(POWER_SLEEP_POWER_DOWN);
    sleep_enable();

#if defined(BODS) && defined(BODSE)
    /*
     * BOD disable sequence (timed – must enter sleep within 3 cycles):
     * 1. Set BODS and BODSE
     * 2. Clear BODSE within 4 cycles
     * 3. Execute SLEEP within 3 cycles
     */
    uint8_t sreg = SREG;
    cli();
    MCUCR |= (1 << BODS) | (1 << BODSE);
    MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS);
    sei();
    sleep_cpu();
    SREG = sreg;
#else
    sei();
    sleep_cpu();
#endif

    sleep_disable();
}

void POWER_DisablePeripheral(power_periph_e periph)
{
#if defined(PRR)
    PRR |= (1 << (uint8_t)periph);
#elif defined(PRR0)
    PRR0 |= (1 << (uint8_t)periph);
#endif
}

void POWER_EnablePeripheral(power_periph_e periph)
{
#if defined(PRR)
    PRR &= ~(1 << (uint8_t)periph);
#elif defined(PRR0)
    PRR0 &= ~(1 << (uint8_t)periph);
#endif
}

void POWER_DisableAllPeripherals(void)
{
#if defined(PRR)
    PRR = 0xFF;
#elif defined(PRR0)
    PRR0 = 0xFF;
#endif
}

void POWER_EnableAllPeripherals(void)
{
#if defined(PRR)
    PRR = 0x00;
#elif defined(PRR0)
    PRR0 = 0x00;
#endif
}

bool POWER_IsPeripheralEnabled(power_periph_e periph)
{
#if defined(PRR)
    return !(PRR & (1 << (uint8_t)periph));
#elif defined(PRR0)
    return !(PRR0 & (1 << (uint8_t)periph));
#else
    (void)periph;
    return true;
#endif
}

#endif /* AVR_POWER_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
/*
 * Deep-sleep demo: wake up every ~8 s via Watchdog interrupt,
 * toggle LED, go back to sleep.
 */
#define F_CPU 16000000UL
#define AVR_POWER_IMPLEMENTATION
#include "avr_power.h"
#define AVR_WDT_IMPLEMENTATION
#include "avr_wdt.h"
#include <avr/io.h>

static volatile uint8_t woke_up = 0;

void wdt_wake(void)
{
    woke_up = 1;
}

int main(void)
{
    /* LED on PB5 */
    DDRB  |= (1 << PB5);
    PORTB &= ~(1 << PB5);

    /* Disable unused peripherals */
    POWER_DisablePeripheral(POWER_ADC);
    POWER_DisablePeripheral(POWER_SPI);
    POWER_DisablePeripheral(POWER_TWI);
    POWER_DisablePeripheral(POWER_USART);
    POWER_DisablePeripheral(POWER_TIMER1);
    POWER_DisablePeripheral(POWER_TIMER2);

    /* Use WDT interrupt to wake every ~8 s */
    WDT_SetCallback(wdt_wake);
    WDT_Enable(WDT_TIMEOUT_8S, WDT_MODE_INTERRUPT);
    sei();

    while (1) {
        POWER_DeepSleep();

        if (woke_up) {
            woke_up = 0;
            PINB |= (1 << PB5);  /* toggle LED */
            /* Re-arm WDT interrupt */
            WDT_Enable(WDT_TIMEOUT_8S, WDT_MODE_INTERRUPT);
        }
    }
    return 0;
}
#endif

#endif /* AVR_POWER_H */
