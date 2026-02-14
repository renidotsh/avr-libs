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
    uart_init(9600);
    sei();
    uart_send_string("ADC Demo\r\n");

    /* --- Init with AVCC ref, auto prescaler --- */
    adc_init(ADC_REF_AVCC, ADC_PRESCALE_AUTO);

    /* --- Single-shot 10-bit read --- */
    uint16_t ch0 = adc_read_channel(ADC_CH0);
    uart_send_string("CH0 (10-bit): ");
    uart_print_u16(ch0);
    uart_send_string("\r\n");

    /* --- 8-bit read --- */
    uint8_t ch1_8 = adc_read_channel8(ADC_CH1);
    uart_send_string("CH1 (8-bit): ");
    uart_print_u16(ch1_8);
    uart_send_string("\r\n");

    /* --- Averaged read --- */
    uint16_t avg = adc_read_average(ADC_CH0, 16);
    uart_send_string("CH0 avg(16): ");
    uart_print_u16(avg);
    uart_send_string("\r\n");

    /* --- Millivolt conversion --- */
    uint16_t mv = adc_to_millivolts(avg, 5000);
    uart_send_string("CH0 mV: ");
    uart_print_u16(mv);
    uart_send_string("\r\n");

    /* --- Change reference voltage --- */
    adc_set_reference(ADC_REF_INTERNAL);
    (void)adc_read_channel(ADC_CH0);  /* discard first after ref change */
    uint16_t int_ref = adc_read_channel(ADC_CH0);
    uart_send_string("CH0 (1.1V ref): ");
    uart_print_u16(int_ref);
    uart_send_string("\r\n");
    adc_set_reference(ADC_REF_AVCC);  /* restore */

    /* --- Read internal temperature --- */
    uint16_t temp = adc_read_channel(ADC_CH_TEMP);
    uart_send_string("Temp raw: ");
    uart_print_u16(temp);
    uart_send_string("\r\n");

    /* --- Free-running mode --- */
    uart_send_string("Free-running on CH0:\r\n");
    adc_start_free_running(ADC_CH0, adc_callback);

    for (uint8_t i = 0; i < 5; i++) {
        _delay_ms(500);
        uint16_t r = adc_get_last_result();
        uart_send_string("FR: ");
        uart_print_u16(r);
        uart_send_string("\r\n");
    }

    /* --- Stop free-running --- */
    adc_stop_free_running();
    uart_send_string("Free-run stopped\r\n");

    /* --- Disable ADC --- */
    adc_disable();
    uart_send_string("ADC disabled\r\n");

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
