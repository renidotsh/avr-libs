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
    UART_Init(9600);
    sei();
    UART_SendString("PWM Demo\r\n");

    /* --- OC0A: Fast PWM, prescaler 64 --- */
    PWM_Init(PWM_CH_OC0A, PWM_FAST, PWM01_PRE_64);
    PWM_SetDuty(PWM_CH_OC0A, 50);  /* 50 % */

    /* --- OC0B: Phase-correct PWM --- */
    PWM_Init(PWM_CH_OC0B, PWM_PHASE_CORRECT, PWM01_PRE_64);
    PWM_SetDuty(PWM_CH_OC0B, 25);

    /* --- OC2A: Fast PWM --- */
    PWM_Init(PWM_CH_OC2A, PWM_FAST, PWM2_PRE_64);
    PWM_SetDutyRaw(PWM_CH_OC2A, 128);

    /* --- OC2B: Phase-correct --- */
    PWM_Init(PWM_CH_OC2B, PWM_PHASE_CORRECT, PWM2_PRE_64);
    PWM_SetDuty(PWM_CH_OC2B, 75);

    /* --- Timer1: custom frequency 50 Hz for servo --- */
    /* top = 16e6 / (8 * 50) - 1 = 39999 */
    PWM_InitTimer1Freq(PWM_FAST, PWM01_PRE_8, 39999);
    PWM_SetDutyRaw(PWM_CH_OC1A, 2000);   /* ~1 ms pulse → 0° */
    PWM_SetDutyRaw(PWM_CH_OC1B, 3000);   /* ~1.5 ms → 90°     */

    /* --- Frequency calculation --- */
    uint32_t f8 = PWM_CalcFreq8(64, PWM_FAST);
    UART_SendString("8-bit Fast f: ");
    UART_PrintU16((uint16_t)f8);
    UART_SendString(" Hz\r\n");

    uint32_t f16 = PWM_CalcFreq16(8, 39999, PWM_FAST);
    UART_SendString("16-bit 50Hz f: ");
    UART_PrintU16((uint16_t)f16);
    UART_SendString(" Hz\r\n");

    /* --- Fade OC0A up and down --- */
    uint8_t duty = 0;
    int8_t dir = 1;

    while (1) {
        PWM_SetDuty(PWM_CH_OC0A, duty);

        /* Sweep servo from 0° to 180° */
        uint16_t servo_val = 2000 + (uint16_t)duty * 20;
        PWM_SetDutyRaw(PWM_CH_OC1A, servo_val);

        duty += dir;
        if (duty >= 100) dir = -1;
        if (duty == 0)   dir =  1;

        _delay_ms(20);
    }

    /* --- Stop (unreachable, for API coverage) --- */
    PWM_Stop(PWM_CH_OC0A);
    PWM_Stop(PWM_CH_OC1A);
    PWM_Stop(PWM_CH_OC2A);
    PWM_Stop(PWM_CH_OC2B);

    return 0;
}
