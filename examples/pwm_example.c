/**
 * @file    pwm_example.c
 * @brief   ATmega328P example exercising every avr_pwm.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_PWM_IMPLEMENTATION
#include "../avr_pwm.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"
#include <util/delay.h>

int main(void)
{
    uart_init(9600);
    sei();
    uart_send_string("PWM Demo\r\n");

    /* --- OC0A: Fast PWM, prescaler 64 --- */
    pwm_init(PWM_CH_OC0A, PWM_FAST, PWM01_PRE_64);
    pwm_set_duty(PWM_CH_OC0A, 50);  /* 50 % */

    /* --- OC0B: Phase-correct PWM --- */
    pwm_init(PWM_CH_OC0B, PWM_PHASE_CORRECT, PWM01_PRE_64);
    pwm_set_duty(PWM_CH_OC0B, 25);

    /* --- OC2A: Fast PWM --- */
    pwm_init(PWM_CH_OC2A, PWM_FAST, PWM2_PRE_64);
    pwm_set_duty_raw(PWM_CH_OC2A, 128);

    /* --- OC2B: Phase-correct --- */
    pwm_init(PWM_CH_OC2B, PWM_PHASE_CORRECT, PWM2_PRE_64);
    pwm_set_duty(PWM_CH_OC2B, 75);

    /* --- Timer1: custom frequency 50 Hz for servo --- */
    /* top = 16e6 / (8 * 50) - 1 = 39999 */
    pwm_init_timer1_freq(PWM_FAST, PWM01_PRE_8, 39999);
    pwm_set_duty_raw(PWM_CH_OC1A, 2000);   /* ~1 ms pulse → 0° */
    pwm_set_duty_raw(PWM_CH_OC1B, 3000);   /* ~1.5 ms → 90°     */

    /* --- Frequency calculation --- */
    uint32_t f8 = pwm_calc_freq8(64, PWM_FAST);
    uart_send_string("8-bit Fast f: ");
    uart_print_u16((uint16_t)f8);
    uart_send_string(" Hz\r\n");

    uint32_t f16 = pwm_calc_freq16(8, 39999, PWM_FAST);
    uart_send_string("16-bit 50Hz f: ");
    uart_print_u16((uint16_t)f16);
    uart_send_string(" Hz\r\n");

    /* --- Fade OC0A up and down --- */
    uint8_t duty = 0;
    int8_t dir = 1;

    while (1) {
        pwm_set_duty(PWM_CH_OC0A, duty);

        /* Sweep servo from 0° to 180° */
        uint16_t servo_val = 2000 + (uint16_t)duty * 20;
        pwm_set_duty_raw(PWM_CH_OC1A, servo_val);

        duty += dir;
        if (duty >= 100) dir = -1;
        if (duty == 0)   dir =  1;

        _delay_ms(20);
    }

    /* --- Stop (unreachable, for API coverage) --- */
    pwm_stop(PWM_CH_OC0A);
    pwm_stop(PWM_CH_OC1A);
    pwm_stop(PWM_CH_OC2A);
    pwm_stop(PWM_CH_OC2B);

    return 0;
}
