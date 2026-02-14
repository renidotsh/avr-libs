/**
 * @file     avr_pwm.h
 * @brief    PWM signal generation library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Configures Timer0, Timer1, and Timer2 for PWM output on their respective
 * OC pins.  Supports Fast PWM and Phase-Correct PWM in 8-bit (Timer0/2)
 * and 16-bit (Timer1) modes.  Provides percentage-based and raw duty-cycle
 * control, plus Timer1 custom frequency via ICR1 top value.
 *
 * @features
 * - Fast PWM and Phase-Correct PWM
 * - 8-bit (Timer0, Timer2) and 16-bit (Timer1) modes
 * - Duty cycle 0-100 % with raw register access
 * - Frequency configuration for Timer1 via ICR1
 * - Multi-channel: OC0A/B, OC1A/B, OC2A/B
 *
 * @example
 *   #define AVR_PWM_IMPLEMENTATION
 *   #include "avr_pwm.h"
 *
 *   int main(void) {
 *       pwm_init(PWM_CH_OC0A, PWM_FAST, PWM01_PRE_64);
 *       pwm_set_duty(PWM_CH_OC0A, 50);   // 50 %
 *       while(1);
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_PWM_H
#define AVR_PWM_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== CONFIGURATION ===== */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* ===== CHANNEL IDENTIFIERS ===== */
typedef enum {
    PWM_CH_OC0A = 0,   /**< Timer0 channel A – PD6 on ATmega328P */
    PWM_CH_OC0B = 1,   /**< Timer0 channel B – PD5               */
    PWM_CH_OC1A = 2,   /**< Timer1 channel A – PB1               */
    PWM_CH_OC1B = 3,   /**< Timer1 channel B – PB2               */
    PWM_CH_OC2A = 4,   /**< Timer2 channel A – PB3               */
    PWM_CH_OC2B = 5    /**< Timer2 channel B – PD3               */
} pwm_channel_e;

/* ===== PWM MODE ===== */
typedef enum {
    PWM_FAST         = 0,   /**< Fast PWM                         */
    PWM_PHASE_CORRECT = 1   /**< Phase-correct (dual-slope) PWM   */
} pwm_mode_e;

/* ===== PRESCALER (Timer0 / Timer1) ===== */
typedef enum {
    PWM01_PRE_OFF  = 0,
    PWM01_PRE_1    = 1,
    PWM01_PRE_8    = 2,
    PWM01_PRE_64   = 3,
    PWM01_PRE_256  = 4,
    PWM01_PRE_1024 = 5
} pwm01_prescale_e;

/* ===== PRESCALER (Timer2) ===== */
typedef enum {
    PWM2_PRE_OFF  = 0,
    PWM2_PRE_1    = 1,
    PWM2_PRE_8    = 2,
    PWM2_PRE_32   = 3,
    PWM2_PRE_64   = 4,
    PWM2_PRE_128  = 5,
    PWM2_PRE_256  = 6,
    PWM2_PRE_1024 = 7
} pwm2_prescale_e;

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Initialise a PWM channel (8-bit, default TOP = 0xFF).
 * @param  ch       Channel identifier
 * @param  mode     PWM_FAST or PWM_PHASE_CORRECT
 * @param  prescale Prescaler value (use PWM01_PRE_* for ch 0-3,
 *                  PWM2_PRE_* for ch 4-5; cast is fine since the CS
 *                  bit-fields are the same width)
 * @note   Also sets the OC pin as output.  For Timer1, uses 8-bit
 *         top (0x00FF); for custom frequency, use pwm_init_timer1_freq().
 */
void pwm_init(pwm_channel_e ch, pwm_mode_e mode, uint8_t prescale);

/**
 * @brief  Initialise Timer1 PWM with a custom frequency.
 *         Uses ICR1 as TOP for adjustable frequency.
 * @param  mode      PWM_FAST or PWM_PHASE_CORRECT
 * @param  prescale  Prescaler value (PWM01_PRE_*)
 * @param  top       ICR1 value – determines PWM frequency:
 *                   f_pwm = F_CPU / (prescale * (1 + top))  (fast)
 *                   f_pwm = F_CPU / (prescale * 2 * top)    (phase-correct)
 * @note   OC1A and OC1B share the same frequency (ICR1 is common).
 */
void pwm_init_timer1_freq(pwm_mode_e mode, pwm01_prescale_e prescale,
                        uint16_t top);

/**
 * @brief  Set duty cycle as a percentage (0-100).
 * @param  ch      Channel
 * @param  percent Duty cycle 0-100
 */
void pwm_set_duty(pwm_channel_e ch, uint8_t percent);

/**
 * @brief  Set raw duty cycle value directly.
 * @param  ch     Channel
 * @param  value  OCRx value (0-255 for 8-bit, 0-TOP for Timer1)
 */
void pwm_set_duty_raw(pwm_channel_e ch, uint16_t value);

/**
 * @brief  Stop PWM output on a channel (disconnect OC pin).
 * @param  ch  Channel to stop
 */
void pwm_stop(pwm_channel_e ch);

/**
 * @brief  Calculate the PWM frequency for a Timer0/2 channel.
 * @param  prescale  Prescaler divider value (actual: 1,8,64,256,1024)
 * @param  mode      PWM mode
 * @return Frequency in Hz
 */
uint32_t pwm_calc_freq8(uint16_t prescale, pwm_mode_e mode);

/**
 * @brief  Calculate the PWM frequency for Timer1 with a custom TOP.
 * @param  prescale  Prescaler divider value (actual)
 * @param  top       ICR1 value
 * @param  mode      PWM mode
 * @return Frequency in Hz
 */
uint32_t pwm_calc_freq16(uint16_t prescale, uint16_t top, pwm_mode_e mode);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_PWM_IMPLEMENTATION

/* Helper: actual prescaler divider values for Timer0/1 */
static const uint16_t _pwm01_div[] = {0, 1, 8, 64, 256, 1024};

/* ---- Per-channel OC pin setup ---- */

static void _pwm_set_oc_output(pwm_channel_e ch)
{
    /* Set the OC pin as output */
    switch (ch) {
#if defined(DDD6) /* ATmega328P */
        case PWM_CH_OC0A: DDRD |= (1 << DDD6); break;  /* PD6 */
        case PWM_CH_OC0B: DDRD |= (1 << DDD5); break;  /* PD5 */
#endif
        case PWM_CH_OC1A: DDRB |= (1 << DDB1); break;  /* PB1 */
        case PWM_CH_OC1B: DDRB |= (1 << DDB2); break;  /* PB2 */
        case PWM_CH_OC2A: DDRB |= (1 << DDB3); break;  /* PB3 */
#if defined(DDD3)
        case PWM_CH_OC2B: DDRD |= (1 << DDD3); break;  /* PD3 */
#endif
        default: break;
    }
}

/* ---- Init ---- */

void pwm_init(pwm_channel_e ch, pwm_mode_e mode, uint8_t prescale)
{
    _pwm_set_oc_output(ch);

    switch (ch) {
        case PWM_CH_OC0A:
        case PWM_CH_OC0B: {
            /* Timer0 */
            TCCR0A = 0;
            TCCR0B = 0;
            if (mode == PWM_FAST) {
                /* Fast PWM, TOP=0xFF: WGM0[2:0]=011 */
                TCCR0A |= (1 << WGM01) | (1 << WGM00);
            } else {
                /* Phase-correct PWM, TOP=0xFF: WGM0[2:0]=001 */
                TCCR0A |= (1 << WGM00);
            }
            /* Connect OC0x – non-inverting */
            if (ch == PWM_CH_OC0A)
                TCCR0A |= (1 << COM0A1);
            else
                TCCR0A |= (1 << COM0B1);
            TCCR0B |= (prescale & 0x07);
            break;
        }

        case PWM_CH_OC1A:
        case PWM_CH_OC1B: {
            /* Timer1 – 8-bit mode (TOP=0x00FF) */
            TCCR1A = 0;
            TCCR1B = 0;
            if (mode == PWM_FAST) {
                /* Fast PWM 8-bit: WGM1[3:0]=0101 */
                TCCR1A |= (1 << WGM10);
                TCCR1B |= (1 << WGM12);
            } else {
                /* Phase-correct 8-bit: WGM1[3:0]=0001 */
                TCCR1A |= (1 << WGM10);
            }
            if (ch == PWM_CH_OC1A)
                TCCR1A |= (1 << COM1A1);
            else
                TCCR1A |= (1 << COM1B1);
            TCCR1B |= (prescale & 0x07);
            break;
        }

        case PWM_CH_OC2A:
        case PWM_CH_OC2B: {
            /* Timer2 */
            TCCR2A = 0;
            TCCR2B = 0;
            if (mode == PWM_FAST) {
                TCCR2A |= (1 << WGM21) | (1 << WGM20);
            } else {
                TCCR2A |= (1 << WGM20);
            }
            if (ch == PWM_CH_OC2A)
                TCCR2A |= (1 << COM2A1);
            else
                TCCR2A |= (1 << COM2B1);
            TCCR2B |= (prescale & 0x07);
            break;
        }
        default: break;
    }
}

void pwm_init_timer1_freq(pwm_mode_e mode, pwm01_prescale_e prescale,
                        uint16_t top)
{
    TCCR1A = 0;
    TCCR1B = 0;
    ICR1   = top;

    if (mode == PWM_FAST) {
        /* Fast PWM, TOP=ICR1: WGM1[3:0]=1110 */
        TCCR1A |= (1 << WGM11);
        TCCR1B |= (1 << WGM13) | (1 << WGM12);
    } else {
        /* Phase & frequency correct, TOP=ICR1: WGM1[3:0]=1000 */
        TCCR1B |= (1 << WGM13);
    }

    /* Non-inverting on both channels */
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1);

    TCCR1B |= ((uint8_t)prescale & 0x07);

    /* Set OC pins as outputs */
    DDRB |= (1 << DDB1) | (1 << DDB2);
}

/* ---- Duty cycle ---- */

void pwm_set_duty(pwm_channel_e ch, uint8_t percent)
{
    if (percent > 100) percent = 100;

    switch (ch) {
        case PWM_CH_OC0A:
        case PWM_CH_OC0B:
        case PWM_CH_OC2A:
        case PWM_CH_OC2B: {
            /* 8-bit: value = 255 * percent / 100 */
            uint8_t val = (uint8_t)(((uint16_t)percent * 255U) / 100U);
            pwm_set_duty_raw(ch, val);
            break;
        }
        case PWM_CH_OC1A:
        case PWM_CH_OC1B: {
            /* 16-bit: value = TOP * percent / 100 */
            uint16_t top = ICR1 ? ICR1 : 0x00FF;
            uint16_t val = (uint16_t)(((uint32_t)percent * top) / 100UL);
            pwm_set_duty_raw(ch, val);
            break;
        }
        default: break;
    }
}

void pwm_set_duty_raw(pwm_channel_e ch, uint16_t value)
{
    switch (ch) {
        case PWM_CH_OC0A: OCR0A = (uint8_t)value; break;
        case PWM_CH_OC0B: OCR0B = (uint8_t)value; break;
        case PWM_CH_OC1A: OCR1A = value;           break;
        case PWM_CH_OC1B: OCR1B = value;           break;
        case PWM_CH_OC2A: OCR2A = (uint8_t)value; break;
        case PWM_CH_OC2B: OCR2B = (uint8_t)value; break;
        default: break;
    }
}

/* ---- Stop ---- */

void pwm_stop(pwm_channel_e ch)
{
    switch (ch) {
        case PWM_CH_OC0A: TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0)); break;
        case PWM_CH_OC0B: TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0)); break;
        case PWM_CH_OC1A: TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0)); break;
        case PWM_CH_OC1B: TCCR1A &= ~((1 << COM1B1) | (1 << COM1B0)); break;
        case PWM_CH_OC2A: TCCR2A &= ~((1 << COM2A1) | (1 << COM2A0)); break;
        case PWM_CH_OC2B: TCCR2A &= ~((1 << COM2B1) | (1 << COM2B0)); break;
        default: break;
    }
}

/* ---- Frequency calculators ---- */

uint32_t pwm_calc_freq8(uint16_t prescale, pwm_mode_e mode)
{
    if (prescale == 0) return 0;
    if (mode == PWM_FAST)
        return F_CPU / ((uint32_t)prescale * 256UL);          /* TOP=255 */
    else
        return F_CPU / ((uint32_t)prescale * 510UL);          /* dual-slope */
}

uint32_t pwm_calc_freq16(uint16_t prescale, uint16_t top, pwm_mode_e mode)
{
    if (prescale == 0 || top == 0) return 0;
    if (mode == PWM_FAST)
        return F_CPU / ((uint32_t)prescale * ((uint32_t)top + 1));
    else
        return F_CPU / ((uint32_t)prescale * 2 * (uint32_t)top);
}

#endif /* AVR_PWM_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_PWM_IMPLEMENTATION
#include "avr_pwm.h"
#include <util/delay.h>

int main(void)
{
    /* OC0A – Fast PWM, prescaler 64 → ~977 Hz */
    pwm_init(PWM_CH_OC0A, PWM_FAST, PWM01_PRE_64);

    /* OC1A – custom frequency via ICR1: 50 Hz for servo */
    /* top = F_CPU / (prescale * freq) - 1 = 16e6 / (8*50) - 1 = 39999 */
    pwm_init_timer1_freq(PWM_FAST, PWM01_PRE_8, 39999);

    /* OC2B – Phase-correct PWM */
    pwm_init(PWM_CH_OC2B, PWM_PHASE_CORRECT, PWM2_PRE_64);
    pwm_set_duty(PWM_CH_OC2B, 25);   /* 25 % */

    /* Fade OC0A up and down */
    uint8_t duty = 0;
    int8_t dir = 1;

    while (1) {
        pwm_set_duty(PWM_CH_OC0A, duty);
        /* Servo: 1ms-2ms pulse for 0-180° (1ms=50/40000*1000≈ */
        /* OCR1A = 2000 .. 4000 for 1ms..2ms @ 50Hz/8pre) */
        pwm_set_duty_raw(PWM_CH_OC1A, 2000 + (uint16_t)duty * 20);

        duty += dir;
        if (duty >= 100) dir = -1;
        if (duty == 0) dir = 1;

        _delay_ms(20);
    }
    return 0;
}
#endif

#endif /* AVR_PWM_H */
