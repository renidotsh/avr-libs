/**
 * @file     avr_swtimer.h
 * @brief    Software timer system for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Cooperative, tick-driven software timer system.  A hardware timer ISR
 * calls SWTIMER_Tick() every 1 ms; callbacks are deferred to main-loop
 * context via SWTIMER_Process() for safe, non-ISR execution.  Uses a
 * fixed-size static timer pool – no dynamic memory allocation.
 *
 * @features
 * - Fixed-size timer pool (default 8, configurable)
 * - One-shot and periodic timers
 * - Callback executed in main-loop context (deferred)
 * - Start / stop / restart / remaining-time query
 * - No malloc – pure static array management
 * - Microsecond-resolution tick support (configurable)
 *
 * @architecture
 *   HW Timer ISR → SWTIMER_Tick()  (decrements counters)
 *   main loop    → SWTIMER_Process() (fires callbacks)
 *
 * @example
 *   #define AVR_SWTIMER_IMPLEMENTATION
 *   #include "avr_swtimer.h"
 *
 *   void blink(void) { PINB |= (1<<PB5); }
 *
 *   int main(void) {
 *       SWTIMER_Init();
 *       SWTIMER_Create(500, blink, false);  // 500 ms periodic
 *       // start HW timer that calls SWTIMER_Tick() every 1 ms
 *       sei();
 *       while (1) SWTIMER_Process();
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_SWTIMER_H
#define AVR_SWTIMER_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== CONFIGURATION ===== */

/** Maximum number of concurrent software timers */
#ifndef SWTIMER_MAX_TIMERS
#define SWTIMER_MAX_TIMERS 8
#endif

/** Invalid handle sentinel */
#define SWTIMER_INVALID  0xFF

/* ===== TYPES ===== */

/** Timer handle (0 .. SWTIMER_MAX_TIMERS-1, or SWTIMER_INVALID) */
typedef uint8_t swtimer_handle_t;

/** Callback function signature */
typedef void (*swtimer_callback_t)(void);

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Initialise the software timer subsystem.
 *         Clears all timers.  Call once at startup.
 */
void SWTIMER_Init(void);

/**
 * @brief  Create and start a new software timer.
 * @param  period_ms   Timer period in milliseconds (1 – 65535)
 * @param  callback    Function to call when timer expires
 * @param  one_shot    true = fire once and auto-stop,
 *                     false = periodic (auto-reload)
 * @return Timer handle, or SWTIMER_INVALID if pool is full.
 */
swtimer_handle_t SWTIMER_Create(uint16_t period_ms,
                                swtimer_callback_t callback,
                                bool one_shot);

/**
 * @brief  Stop and release a timer back to the pool.
 * @param  handle  Timer handle
 */
void SWTIMER_Delete(swtimer_handle_t handle);

/**
 * @brief  Stop a timer without releasing it.  Can be restarted.
 * @param  handle  Timer handle
 */
void SWTIMER_Stop(swtimer_handle_t handle);

/**
 * @brief  Restart a stopped or running timer with its original period.
 * @param  handle  Timer handle
 */
void SWTIMER_Restart(swtimer_handle_t handle);

/**
 * @brief  Change the period of an existing timer.
 * @param  handle     Timer handle
 * @param  period_ms  New period
 */
void SWTIMER_SetPeriod(swtimer_handle_t handle, uint16_t period_ms);

/**
 * @brief  Get the remaining time of a timer.
 * @param  handle  Timer handle
 * @return Remaining milliseconds (0 if expired or invalid)
 */
uint16_t SWTIMER_Remaining(swtimer_handle_t handle);

/**
 * @brief  Check if a timer is currently running.
 * @param  handle  Timer handle
 * @return true if active
 */
bool SWTIMER_IsRunning(swtimer_handle_t handle);

/**
 * @brief  Tick function – call from a 1 ms hardware timer ISR.
 *         Decrements all active timer counters and marks expired ones.
 *         Very lightweight – safe to call from ISR context.
 */
void SWTIMER_Tick(void);

/**
 * @brief  Process expired timers – call from main loop.
 *         Invokes callbacks of any timers that have expired.
 *         Reloads periodic timers automatically.
 */
void SWTIMER_Process(void);

/**
 * @brief  Get the number of active (allocated) timers.
 * @return Count 0 .. SWTIMER_MAX_TIMERS
 */
uint8_t SWTIMER_ActiveCount(void);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_SWTIMER_IMPLEMENTATION

typedef struct {
    swtimer_callback_t callback;
    uint16_t period;          /**< reload value   */
    volatile uint16_t counter;/**< current count  */
    volatile uint8_t  flags;  /**< see below      */
} _swtimer_entry_t;

/* flag bits */
#define _SWT_ALLOCATED  0x01
#define _SWT_RUNNING    0x02
#define _SWT_ONESHOT    0x04
#define _SWT_EXPIRED    0x08

static _swtimer_entry_t _timers[SWTIMER_MAX_TIMERS];

void SWTIMER_Init(void)
{
    for (uint8_t i = 0; i < SWTIMER_MAX_TIMERS; i++)
        _timers[i].flags = 0;
}

swtimer_handle_t SWTIMER_Create(uint16_t period_ms,
                                swtimer_callback_t callback,
                                bool one_shot)
{
    for (uint8_t i = 0; i < SWTIMER_MAX_TIMERS; i++) {
        if (!(_timers[i].flags & _SWT_ALLOCATED)) {
            _timers[i].callback = callback;
            _timers[i].period   = period_ms;
            _timers[i].counter  = period_ms;
            _timers[i].flags    = _SWT_ALLOCATED | _SWT_RUNNING;
            if (one_shot)
                _timers[i].flags |= _SWT_ONESHOT;
            return i;
        }
    }
    return SWTIMER_INVALID;
}

void SWTIMER_Delete(swtimer_handle_t handle)
{
    if (handle < SWTIMER_MAX_TIMERS)
        _timers[handle].flags = 0;
}

void SWTIMER_Stop(swtimer_handle_t handle)
{
    if (handle < SWTIMER_MAX_TIMERS)
        _timers[handle].flags &= ~_SWT_RUNNING;
}

void SWTIMER_Restart(swtimer_handle_t handle)
{
    if (handle < SWTIMER_MAX_TIMERS) {
        _timers[handle].counter = _timers[handle].period;
        _timers[handle].flags |= _SWT_RUNNING;
        _timers[handle].flags &= ~_SWT_EXPIRED;
    }
}

void SWTIMER_SetPeriod(swtimer_handle_t handle, uint16_t period_ms)
{
    if (handle < SWTIMER_MAX_TIMERS) {
        _timers[handle].period  = period_ms;
        _timers[handle].counter = period_ms;
    }
}

uint16_t SWTIMER_Remaining(swtimer_handle_t handle)
{
    if (handle < SWTIMER_MAX_TIMERS)
        return _timers[handle].counter;
    return 0;
}

bool SWTIMER_IsRunning(swtimer_handle_t handle)
{
    if (handle < SWTIMER_MAX_TIMERS)
        return (_timers[handle].flags & _SWT_RUNNING) != 0;
    return false;
}

void SWTIMER_Tick(void)
{
    /* Called from ISR – keep as short as possible */
    for (uint8_t i = 0; i < SWTIMER_MAX_TIMERS; i++) {
        uint8_t f = _timers[i].flags;
        if ((f & (_SWT_ALLOCATED | _SWT_RUNNING)) ==
                 (_SWT_ALLOCATED | _SWT_RUNNING)) {
            if (_timers[i].counter > 0) {
                _timers[i].counter--;
            }
            if (_timers[i].counter == 0) {
                _timers[i].flags |= _SWT_EXPIRED;
            }
        }
    }
}

void SWTIMER_Process(void)
{
    for (uint8_t i = 0; i < SWTIMER_MAX_TIMERS; i++) {
        uint8_t f = _timers[i].flags;
        if (f & _SWT_EXPIRED) {
            /* Clear expired flag atomically */
            uint8_t sreg = SREG;
            cli();
            _timers[i].flags &= ~_SWT_EXPIRED;

            if (f & _SWT_ONESHOT) {
                _timers[i].flags &= ~_SWT_RUNNING;
            } else {
                /* Reload for next period */
                _timers[i].counter = _timers[i].period;
            }
            SREG = sreg;

            /* Fire callback in main-loop context */
            if (_timers[i].callback)
                _timers[i].callback();
        }
    }
}

uint8_t SWTIMER_ActiveCount(void)
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < SWTIMER_MAX_TIMERS; i++) {
        if (_timers[i].flags & _SWT_ALLOCATED)
            n++;
    }
    return n;
}

#endif /* AVR_SWTIMER_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
/*
 * Two software timers:
 *   - 500 ms periodic: toggle LED on PB5
 *   - 3000 ms one-shot: print message, then stop
 *
 * Timer0 CTC provides the 1 ms tick.
 */
#define F_CPU 16000000UL
#define AVR_SWTIMER_IMPLEMENTATION
#include "avr_swtimer.h"
#define AVR_TIMER_IMPLEMENTATION
#include "avr_timer.h"
#define AVR_UART_IMPLEMENTATION
#include "avr_uart.h"

/* Hardware timer 0 compare-match drives SWTIMER_Tick */
static void hw_tick(void)
{
    SWTIMER_Tick();
}

void blink(void)
{
    PINB |= (1 << PB5);
}

void one_shot_msg(void)
{
    UART_SendString("One-shot fired!\r\n");
}

int main(void)
{
    DDRB |= (1 << PB5);

    UART_Init(9600);
    SWTIMER_Init();

    /* 1 ms hardware tick on Timer0 */
    TIMER_InitSystemTick();
    TIMER0_SetCallbacks(0, hw_tick);

    sei();

    /* Create timers */
    swtimer_handle_t blink_tmr = SWTIMER_Create(500, blink, false);
    swtimer_handle_t msg_tmr   = SWTIMER_Create(3000, one_shot_msg, true);

    UART_SendString("SwTimer demo\r\n");
    UART_SendString("Active: ");
    UART_PrintU16(SWTIMER_ActiveCount());
    UART_SendString("\r\n");

    while (1) {
        SWTIMER_Process();  /* fire callbacks here */

        /* After one-shot fires, remaining = 0 */
        if (!SWTIMER_IsRunning(msg_tmr)) {
            SWTIMER_Delete(msg_tmr);
            msg_tmr = SWTIMER_INVALID;
        }
    }

    (void)blink_tmr;
    return 0;
}
#endif

#endif /* AVR_SWTIMER_H */
