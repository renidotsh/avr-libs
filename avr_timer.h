/**
 * @file     avr_timer.h
 * @brief    Timer/Counter management library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Configures Timer0 (8-bit), Timer1 (16-bit), and Timer2 (8-bit) in Normal
 * or CTC modes with selectable prescaler.  Provides overflow / compare-match
 * ISR callback registration, millisecond delay, and a free-running
 * millisecond tick counter driven by Timer0.
 *
 * @features
 * - Timer0/1/2 initialisation with prescaler enum
 * - CTC and Normal mode support
 * - Overflow and Compare-match interrupt callbacks
 * - Millisecond system tick (Timer0-based)
 * - Blocking delay_ms / delay_us helpers
 *
 * @example
 *   #define AVR_TIMER_IMPLEMENTATION
 *   #include "avr_timer.h"
 *
 *   int main(void) {
 *       TIMER_InitSystemTick();   // start 1 ms tick on Timer0
 *       sei();
 *       while (1) {
 *           TIMER_DelayMs(500);
 *       }
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_TIMER_H
#define AVR_TIMER_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== CONFIGURATION ===== */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* ===== TIMER IDENTIFIERS ===== */
#define TIMER_0   0
#define TIMER_1   1
#define TIMER_2   2

/* ===== PRESCALER VALUES ===== */
/** Timer0 / Timer1 prescaler (CS1x / CS0x values) */
typedef enum {
    TIMER01_PRESCALE_OFF  = 0,
    TIMER01_PRESCALE_1    = 1,
    TIMER01_PRESCALE_8    = 2,
    TIMER01_PRESCALE_64   = 3,
    TIMER01_PRESCALE_256  = 4,
    TIMER01_PRESCALE_1024 = 5
} timer01_prescale_e;

/** Timer2 prescaler (CS2x values – different encoding) */
typedef enum {
    TIMER2_PRESCALE_OFF  = 0,
    TIMER2_PRESCALE_1    = 1,
    TIMER2_PRESCALE_8    = 2,
    TIMER2_PRESCALE_32   = 3,
    TIMER2_PRESCALE_64   = 4,
    TIMER2_PRESCALE_128  = 5,
    TIMER2_PRESCALE_256  = 6,
    TIMER2_PRESCALE_1024 = 7
} timer2_prescale_e;

/* ===== TIMER MODE ===== */
typedef enum {
    TIMER_MODE_NORMAL = 0,    /**< Free-running, overflow at TOP */
    TIMER_MODE_CTC    = 1     /**< Clear-on-compare-match        */
} timer_mode_e;

/* ===== CALLBACK TYPE ===== */

/** Function pointer for timer interrupt callbacks */
typedef void (*timer_callback_t)(void);

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Initialise Timer0 (8-bit).
 * @param  mode      TIMER_MODE_NORMAL or TIMER_MODE_CTC
 * @param  prescale  Prescaler value (timer01_prescale_e)
 * @param  ocr_val   Output-compare value for CTC (ignored in NORMAL)
 */
void TIMER0_Init(timer_mode_e mode, timer01_prescale_e prescale,
                 uint8_t ocr_val);

/**
 * @brief  Initialise Timer1 (16-bit).
 * @param  mode      TIMER_MODE_NORMAL or TIMER_MODE_CTC
 * @param  prescale  Prescaler value (timer01_prescale_e)
 * @param  ocr_val   16-bit compare value for CTC (ignored in NORMAL)
 */
void TIMER1_Init(timer_mode_e mode, timer01_prescale_e prescale,
                 uint16_t ocr_val);

/**
 * @brief  Initialise Timer2 (8-bit).
 * @param  mode      TIMER_MODE_NORMAL or TIMER_MODE_CTC
 * @param  prescale  Prescaler value (timer2_prescale_e)
 * @param  ocr_val   Output-compare value for CTC (ignored in NORMAL)
 */
void TIMER2_Init(timer_mode_e mode, timer2_prescale_e prescale,
                 uint8_t ocr_val);

/**
 * @brief  Register a callback for Timer0 overflow or compare-match.
 * @param  on_ovf   Callback for TIMER0_OVF  (NULL to disable)
 * @param  on_compa Callback for TIMER0_COMPA (NULL to disable)
 */
void TIMER0_SetCallbacks(timer_callback_t on_ovf,
                         timer_callback_t on_compa);

/**
 * @brief  Register a callback for Timer1 overflow or compare-match A.
 * @param  on_ovf   Callback for TIMER1_OVF  (NULL to disable)
 * @param  on_compa Callback for TIMER1_COMPA (NULL to disable)
 */
void TIMER1_SetCallbacks(timer_callback_t on_ovf,
                         timer_callback_t on_compa);

/**
 * @brief  Register a callback for Timer2 overflow or compare-match A.
 * @param  on_ovf   Callback for TIMER2_OVF  (NULL to disable)
 * @param  on_compa Callback for TIMER2_COMPA (NULL to disable)
 */
void TIMER2_SetCallbacks(timer_callback_t on_ovf,
                         timer_callback_t on_compa);

/**
 * @brief  Initialise Timer0 as 1 ms system-tick source.
 *         Uses CTC mode with prescaler 64.
 *         OCR0A = (F_CPU / 64 / 1000) - 1  ⟶ 249 @ 16 MHz.
 * @note   Enables Timer0 COMPA interrupt.  Call sei() afterwards.
 */
void TIMER_InitSystemTick(void);

/**
 * @brief  Return the number of milliseconds since TIMER_InitSystemTick().
 * @return 32-bit millisecond counter (wraps after ~49.7 days).
 */
uint32_t TIMER_GetMillis(void);

/**
 * @brief  Blocking delay using the system-tick counter.
 * @param  ms  Delay in milliseconds.
 * @note   Requires TIMER_InitSystemTick() + sei() beforehand.
 */
void TIMER_DelayMs(uint16_t ms);

/**
 * @brief  Blocking microsecond delay using Timer1 in CTC mode.
 * @param  us  Delay in microseconds (1 – 65535).
 * @note   Temporarily reconfigures Timer1.  Restores original state.
 */
void TIMER_DelayUs(uint16_t us);

/**
 * @brief  Stop a timer (clear its clock-select bits).
 * @param  timer_id  TIMER_0, TIMER_1, or TIMER_2
 */
void TIMER_Stop(uint8_t timer_id);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_TIMER_IMPLEMENTATION

/* ---- internal callback storage ---- */
static volatile timer_callback_t _t0_ovf_cb  = (void*)0;
static volatile timer_callback_t _t0_compa_cb = (void*)0;
static volatile timer_callback_t _t1_ovf_cb  = (void*)0;
static volatile timer_callback_t _t1_compa_cb = (void*)0;
static volatile timer_callback_t _t2_ovf_cb  = (void*)0;
static volatile timer_callback_t _t2_compa_cb = (void*)0;

/* ---- system tick counter ---- */
static volatile uint32_t _sys_millis = 0;

/* ---------- Timer0 ---------- */

void TIMER0_Init(timer_mode_e mode, timer01_prescale_e prescale,
                 uint8_t ocr_val)
{
    TCCR0A = 0;
    TCCR0B = 0;
    TCNT0  = 0;

    if (mode == TIMER_MODE_CTC) {
        TCCR0A |= (1 << WGM01);            /* CTC mode (WGM01=1) */
        OCR0A = ocr_val;
    }

    TCCR0B |= (prescale & 0x07);           /* clock-select bits  */
}

void TIMER0_SetCallbacks(timer_callback_t on_ovf,
                         timer_callback_t on_compa)
{
    _t0_ovf_cb  = on_ovf;
    _t0_compa_cb = on_compa;

    if (on_ovf)
        TIMSK0 |= (1 << TOIE0);
    else
        TIMSK0 &= ~(1 << TOIE0);

    if (on_compa)
        TIMSK0 |= (1 << OCIE0A);
    else
        TIMSK0 &= ~(1 << OCIE0A);
}

ISR(TIMER0_OVF_vect)
{
    if (_t0_ovf_cb) _t0_ovf_cb();
}

ISR(TIMER0_COMPA_vect)
{
    if (_t0_compa_cb) _t0_compa_cb();
}

/* ---------- Timer1 ---------- */

void TIMER1_Init(timer_mode_e mode, timer01_prescale_e prescale,
                 uint16_t ocr_val)
{
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    if (mode == TIMER_MODE_CTC) {
        TCCR1B |= (1 << WGM12);            /* CTC mode, TOP=OCR1A */
        OCR1A = ocr_val;
    }

    TCCR1B |= (prescale & 0x07);
}

void TIMER1_SetCallbacks(timer_callback_t on_ovf,
                         timer_callback_t on_compa)
{
    _t1_ovf_cb  = on_ovf;
    _t1_compa_cb = on_compa;

    if (on_ovf)
        TIMSK1 |= (1 << TOIE1);
    else
        TIMSK1 &= ~(1 << TOIE1);

    if (on_compa)
        TIMSK1 |= (1 << OCIE1A);
    else
        TIMSK1 &= ~(1 << OCIE1A);
}

ISR(TIMER1_OVF_vect)
{
    if (_t1_ovf_cb) _t1_ovf_cb();
}

ISR(TIMER1_COMPA_vect)
{
    if (_t1_compa_cb) _t1_compa_cb();
}

/* ---------- Timer2 ---------- */

void TIMER2_Init(timer_mode_e mode, timer2_prescale_e prescale,
                 uint8_t ocr_val)
{
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2  = 0;

    if (mode == TIMER_MODE_CTC) {
        TCCR2A |= (1 << WGM21);
        OCR2A = ocr_val;
    }

    TCCR2B |= (prescale & 0x07);
}

void TIMER2_SetCallbacks(timer_callback_t on_ovf,
                         timer_callback_t on_compa)
{
    _t2_ovf_cb  = on_ovf;
    _t2_compa_cb = on_compa;

    if (on_ovf)
        TIMSK2 |= (1 << TOIE2);
    else
        TIMSK2 &= ~(1 << TOIE2);

    if (on_compa)
        TIMSK2 |= (1 << OCIE2A);
    else
        TIMSK2 &= ~(1 << OCIE2A);
}

ISR(TIMER2_OVF_vect)
{
    if (_t2_ovf_cb) _t2_ovf_cb();
}

ISR(TIMER2_COMPA_vect)
{
    if (_t2_compa_cb) _t2_compa_cb();
}

/* ---------- System tick ---------- */

static void _sys_tick_handler(void)
{
    _sys_millis++;
}

void TIMER_InitSystemTick(void)
{
    /*
     * Timer0 CTC mode, prescaler 64
     * OCR0A = (F_CPU / 64 / 1000) - 1
     *       = (16000000 / 64 / 1000) - 1 = 249 @ 16 MHz
     */
    TIMER0_Init(TIMER_MODE_CTC, TIMER01_PRESCALE_64,
                (uint8_t)((F_CPU / 64UL / 1000UL) - 1));
    TIMER0_SetCallbacks(/*ovf=*/0, /*compa=*/_sys_tick_handler);
}

uint32_t TIMER_GetMillis(void)
{
    uint32_t ms;
    uint8_t sreg = SREG;
    cli();
    ms = _sys_millis;
    SREG = sreg;
    return ms;
}

void TIMER_DelayMs(uint16_t ms)
{
    uint32_t start = TIMER_GetMillis();
    while ((TIMER_GetMillis() - start) < ms)
        ;  /* busy-wait */
}

void TIMER_DelayUs(uint16_t us)
{
    /*
     * Use Timer1 in CTC mode, prescaler 1.
     * Each tick = 1 / F_CPU seconds.
     * Ticks needed = us * (F_CPU / 1000000).
     * If the value exceeds 16-bit, clamp.
     */
    uint8_t sreg = SREG;
    cli();

    /* Save Timer1 state */
    uint8_t saved_tccr1a = TCCR1A;
    uint8_t saved_tccr1b = TCCR1B;
    uint8_t saved_timsk1 = TIMSK1;

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    uint32_t ticks = (uint32_t)us * (F_CPU / 1000000UL);
    if (ticks > 65535UL) ticks = 65535UL;
    if (ticks == 0) ticks = 1;

    OCR1A = (uint16_t)(ticks - 1);
    TIFR1  |= (1 << OCF1A);                /* clear any pending flag */
    TCCR1B  = (1 << WGM12) | (1 << CS10);  /* CTC, prescaler 1      */

    SREG = sreg;                            /* restore interrupt flag */

    while (!(TIFR1 & (1 << OCF1A)))
        ;  /* busy-wait */

    TIFR1 |= (1 << OCF1A);                 /* clear flag             */

    /* Restore Timer1 */
    TCCR1B = 0;
    TCCR1A = saved_tccr1a;
    TCCR1B = saved_tccr1b;
    TIMSK1 = saved_timsk1;
}

void TIMER_Stop(uint8_t timer_id)
{
    switch (timer_id) {
        case TIMER_0:
            TCCR0B &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));
            break;
        case TIMER_1:
            TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
            break;
        case TIMER_2:
            TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
            break;
        default:
            break;
    }
}

#endif /* AVR_TIMER_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_TIMER_IMPLEMENTATION
#include "avr_timer.h"
#include <avr/io.h>

static void blink_callback(void)
{
    PINB |= (1 << PB5);   /* toggle LED */
}

int main(void)
{
    DDRB |= (1 << PB5);   /* PB5 output */

    /* 1 ms system tick */
    TIMER_InitSystemTick();
    sei();

    /* Timer2 CTC: callback every ~4 ms →
       OCR2A = 249, prescaler 256 → 4.096 ms @ 16 MHz */
    TIMER2_Init(TIMER_MODE_CTC, TIMER2_PRESCALE_256, 249);
    TIMER2_SetCallbacks(0, blink_callback);

    while (1) {
        TIMER_DelayMs(1000);
        /* actions every second */
    }
    return 0;
}
#endif

#endif /* AVR_TIMER_H */
