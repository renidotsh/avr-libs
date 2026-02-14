/**
 * @file    adc_example.c
 * @brief   ATmega328P example exercising every avr_adc.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_ADC_IMPLEMENTATION
#include "../avr_adc.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"
#include <util/delay.h>

static volatile uint16_t fr_result = 0;

void adc_callback(uint16_t result)
{
    fr_result = result;
}

int main(void)
{
    UART_Init(9600);
    sei();
    UART_SendString("ADC Demo\r\n");

    /* --- Init with AVCC ref, auto prescaler --- */
    ADC_Init(ADC_REF_AVCC, ADC_PRESCALE_AUTO);

    /* --- Single-shot 10-bit read --- */
    uint16_t ch0 = ADC_ReadChannel(ADC_CH0);
    UART_SendString("CH0 (10-bit): ");
    UART_PrintU16(ch0);
    UART_SendString("\r\n");

    /* --- 8-bit read --- */
    uint8_t ch1_8 = ADC_ReadChannel8(ADC_CH1);
    UART_SendString("CH1 (8-bit): ");
    UART_PrintU16(ch1_8);
    UART_SendString("\r\n");

    /* --- Averaged read --- */
    uint16_t avg = ADC_ReadAverage(ADC_CH0, 16);
    UART_SendString("CH0 avg(16): ");
    UART_PrintU16(avg);
    UART_SendString("\r\n");

    /* --- Millivolt conversion --- */
    uint16_t mv = ADC_ToMillivolts(avg, 5000);
    UART_SendString("CH0 mV: ");
    UART_PrintU16(mv);
    UART_SendString("\r\n");

    /* --- Change reference voltage --- */
    ADC_SetReference(ADC_REF_INTERNAL);
    (void)ADC_ReadChannel(ADC_CH0);  /* discard first after ref change */
    uint16_t int_ref = ADC_ReadChannel(ADC_CH0);
    UART_SendString("CH0 (1.1V ref): ");
    UART_PrintU16(int_ref);
    UART_SendString("\r\n");
    ADC_SetReference(ADC_REF_AVCC);  /* restore */

    /* --- Read internal temperature --- */
    uint16_t temp = ADC_ReadChannel(ADC_CH_TEMP);
    UART_SendString("Temp raw: ");
    UART_PrintU16(temp);
    UART_SendString("\r\n");

    /* --- Free-running mode --- */
    UART_SendString("Free-running on CH0:\r\n");
    ADC_StartFreeRunning(ADC_CH0, adc_callback);

    for (uint8_t i = 0; i < 5; i++) {
        _delay_ms(500);
        uint16_t r = ADC_GetLastResult();
        UART_SendString("FR: ");
        UART_PrintU16(r);
        UART_SendString("\r\n");
    }

    /* --- Stop free-running --- */
    ADC_StopFreeRunning();
    UART_SendString("Free-run stopped\r\n");

    /* --- Disable ADC --- */
    ADC_Disable();
    UART_SendString("ADC disabled\r\n");

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
