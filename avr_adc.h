/**
 * @file     avr_adc.h
 * @brief    Analog-to-Digital Converter library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Provides single-shot and free-running ADC conversion with selectable
 * reference voltage, automatic prescaler selection, and optional interrupt
 * callback.  Supports 10-bit and 8-bit (right-adjusted) result modes.
 *
 * @features
 * - Reference voltage selection (AREF / AVCC / Internal 1.1V)
 * - Single-shot blocking conversion
 * - Free-running mode with ISR callback
 * - Prescaler auto-configuration for best accuracy
 * - 10-bit and 8-bit result modes
 * - Temperature sensor channel (ATmega328P)
 *
 * @example
 *   #define AVR_ADC_IMPLEMENTATION
 *   #include "avr_adc.h"
 *
 *   int main(void) {
 *       ADC_Init(ADC_REF_AVCC, ADC_PRESCALE_AUTO);
 *       uint16_t val = ADC_ReadChannel(0);
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_ADC_H
#define AVR_ADC_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== CONFIGURATION ===== */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* ===== REFERENCE VOLTAGE ===== */
typedef enum {
    ADC_REF_AREF     = 0,   /**< External AREF pin                        */
    ADC_REF_AVCC     = 1,   /**< AVCC with external cap on AREF           */
    ADC_REF_INTERNAL = 3    /**< Internal 1.1V bandgap (ATmega328P)       */
} adc_ref_e;

/* ===== PRESCALER ===== */
typedef enum {
    ADC_PRESCALE_2    = 1,
    ADC_PRESCALE_4    = 2,
    ADC_PRESCALE_8    = 3,
    ADC_PRESCALE_16   = 4,
    ADC_PRESCALE_32   = 5,
    ADC_PRESCALE_64   = 6,
    ADC_PRESCALE_128  = 7,
    ADC_PRESCALE_AUTO = 0xFF /**< Auto-select for 50-200 kHz ADC clock */
} adc_prescale_e;

/* ===== CHANNEL DEFINITIONS ===== */
#define ADC_CH0             0
#define ADC_CH1             1
#define ADC_CH2             2
#define ADC_CH3             3
#define ADC_CH4             4
#define ADC_CH5             5
#define ADC_CH6             6
#define ADC_CH7             7
#define ADC_CH_TEMP         8   /**< Internal temperature sensor            */
#define ADC_CH_1V1          14  /**< Internal 1.1V reference (self-measure) */
#define ADC_CH_GND          15  /**< 0V  (GND)                             */

/* ===== CALLBACK TYPE ===== */
typedef void (*adc_callback_t)(uint16_t result);

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Initialise the ADC peripheral.
 * @param  ref       Reference voltage selection
 * @param  prescale  Clock prescaler (or ADC_PRESCALE_AUTO)
 */
void ADC_Init(adc_ref_e ref, adc_prescale_e prescale);

/**
 * @brief  Perform a single blocking 10-bit conversion.
 * @param  channel  ADC channel (0-7, ADC_CH_TEMP, etc.)
 * @return 10-bit result (0-1023)
 */
uint16_t ADC_ReadChannel(uint8_t channel);

/**
 * @brief  Perform a single blocking 8-bit conversion (high byte only).
 * @param  channel  ADC channel
 * @return 8-bit result (right-adjusted high byte)
 */
uint8_t ADC_ReadChannel8(uint8_t channel);

/**
 * @brief  Perform multiple conversions and return the average.
 * @param  channel  ADC channel
 * @param  samples  Number of samples to average (1-255)
 * @return Averaged 10-bit result
 */
uint16_t ADC_ReadAverage(uint8_t channel, uint8_t samples);

/**
 * @brief  Start free-running mode on a given channel.
 * @param  channel   ADC channel
 * @param  callback  Function called from ISR with each result (or NULL)
 * @note   Call sei() after this.  To stop, call ADC_StopFreeRunning().
 */
void ADC_StartFreeRunning(uint8_t channel, adc_callback_t callback);

/**
 * @brief  Stop free-running conversions and disable ADC interrupt.
 */
void ADC_StopFreeRunning(void);

/**
 * @brief  Read the latest free-running result.
 * @return Most recent 10-bit ADC result
 */
uint16_t ADC_GetLastResult(void);

/**
 * @brief  Change the reference voltage at runtime.
 * @param  ref  New reference voltage
 * @note   After changing, first conversion may be inaccurate (discard it).
 */
void ADC_SetReference(adc_ref_e ref);

/**
 * @brief  Disable the ADC to save power.
 */
void ADC_Disable(void);

/**
 * @brief  Convert a 10-bit ADC value to millivolts.
 * @param  raw       10-bit ADC reading
 * @param  ref_mV    Reference voltage in millivolts (e.g. 5000, 3300, 1100)
 * @return Voltage in millivolts
 */
uint16_t ADC_ToMillivolts(uint16_t raw, uint16_t ref_mV);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_ADC_IMPLEMENTATION

static volatile uint16_t _adc_last_result = 0;
static volatile adc_callback_t _adc_callback = (void*)0;
static uint8_t _adc_ref_bits = 0;

/* ---- auto prescaler ---- */
static uint8_t _adc_auto_prescaler(void)
{
    /*
     * ADC clock should be 50-200 kHz for full 10-bit resolution.
     * Try each prescaler: 2, 4, 8, 16, 32, 64, 128 (values 1-7).
     */
    const uint32_t target_max = 200000UL;

    for (uint8_t p = 1; p <= 7; p++) {
        uint32_t adc_clk = F_CPU / (1UL << p);
        if (adc_clk <= target_max)
            return p;
    }
    return 7;  /* fallback: /128 */
}

/* ---- init ---- */

void ADC_Init(adc_ref_e ref, adc_prescale_e prescale)
{
    _adc_ref_bits = ((uint8_t)ref & 0x03) << REFS0;

    uint8_t ps = (prescale == ADC_PRESCALE_AUTO)
                     ? _adc_auto_prescaler()
                     : (uint8_t)prescale;

    /* ADMUX: reference + right-adjust (ADLAR=0) + channel 0 default */
    ADMUX = _adc_ref_bits;

    /* ADCSRA: enable ADC + prescaler */
    ADCSRA = (1 << ADEN) | (ps & 0x07);

    /* Disable digital input on ADC pins to save power (optional) */
#if defined(DIDR0)
    DIDR0 = 0x3F;  /* Disable digital on ADC0-5 */
#endif
}

/* ---- single shot ---- */

uint16_t ADC_ReadChannel(uint8_t channel)
{
    /* Select channel (lower 4 bits of ADMUX) */
    ADMUX = _adc_ref_bits | (channel & 0x0F);

    /* Start conversion */
    ADCSRA |= (1 << ADSC);

    /* Wait for completion */
    while (ADCSRA & (1 << ADSC))
        ;

    /* Read ADCL first, then ADCH (mandatory order) */
    uint16_t result = ADCL;
    result |= ((uint16_t)ADCH << 8);
    return result;
}

uint8_t ADC_ReadChannel8(uint8_t channel)
{
    /* Left-adjust for 8-bit mode */
    ADMUX = _adc_ref_bits | (1 << ADLAR) | (channel & 0x0F);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC))
        ;

    uint8_t result = ADCH;  /* only need high byte */

    /* Restore right-adjust */
    ADMUX = _adc_ref_bits | (channel & 0x0F);
    return result;
}

uint16_t ADC_ReadAverage(uint8_t channel, uint8_t samples)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++)
        sum += ADC_ReadChannel(channel);
    return (uint16_t)(sum / samples);
}

/* ---- free running ---- */

void ADC_StartFreeRunning(uint8_t channel, adc_callback_t callback)
{
    _adc_callback = callback;

    ADMUX = _adc_ref_bits | (channel & 0x0F);

    /* Enable: ADC, auto-trigger, interrupt */
    ADCSRA |= (1 << ADATE) | (1 << ADIE);

    /* Auto-trigger source: free-running (ADTS[2:0] = 000, default) */
#if defined(ADCSRB)
    ADCSRB &= ~((1 << ADTS2) | (1 << ADTS1) | (1 << ADTS0));
#endif

    /* Start first conversion */
    ADCSRA |= (1 << ADSC);
}

void ADC_StopFreeRunning(void)
{
    ADCSRA &= ~((1 << ADATE) | (1 << ADIE));
    _adc_callback = (void*)0;
}

uint16_t ADC_GetLastResult(void)
{
    uint16_t r;
    uint8_t sreg = SREG;
    cli();
    r = _adc_last_result;
    SREG = sreg;
    return r;
}

/* ---- helpers ---- */

void ADC_SetReference(adc_ref_e ref)
{
    _adc_ref_bits = ((uint8_t)ref & 0x03) << REFS0;
    ADMUX = (ADMUX & 0x3F) | _adc_ref_bits;
}

void ADC_Disable(void)
{
    ADCSRA &= ~(1 << ADEN);
}

uint16_t ADC_ToMillivolts(uint16_t raw, uint16_t ref_mV)
{
    return (uint16_t)(((uint32_t)raw * ref_mV) / 1023UL);
}

/* ---- ISR ---- */

ISR(ADC_vect)
{
    _adc_last_result  = ADCL;
    _adc_last_result |= ((uint16_t)ADCH << 8);

    if (_adc_callback)
        _adc_callback(_adc_last_result);
}

#endif /* AVR_ADC_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_ADC_IMPLEMENTATION
#include "avr_adc.h"

/* Also need UART for printing results */
#define AVR_UART_IMPLEMENTATION
#include "avr_uart.h"
#include <util/delay.h>

static volatile uint16_t free_run_result = 0;

void adc_free_run_cb(uint16_t result)
{
    free_run_result = result;
}

int main(void)
{
    UART_Init(9600);
    ADC_Init(ADC_REF_AVCC, ADC_PRESCALE_AUTO);
    sei();

    UART_SendString("ADC Demo\r\n");

    /* Single-shot read */
    uint16_t val = ADC_ReadChannel(ADC_CH0);
    UART_SendString("CH0: ");
    UART_PrintU16(val);
    UART_SendString("\r\n");

    /* 8-bit mode */
    uint8_t val8 = ADC_ReadChannel8(ADC_CH1);
    UART_SendString("CH1 (8-bit): ");
    UART_PrintU16(val8);
    UART_SendString("\r\n");

    /* Averaged read */
    uint16_t avg = ADC_ReadAverage(ADC_CH0, 16);
    UART_SendString("CH0 avg(16): ");
    UART_PrintU16(avg);
    UART_SendString("\r\n");

    /* Millivolt conversion */
    uint16_t mv = ADC_ToMillivolts(avg, 5000);
    UART_SendString("CH0 mV: ");
    UART_PrintU16(mv);
    UART_SendString("\r\n");

    /* Free-running mode */
    ADC_StartFreeRunning(ADC_CH0, adc_free_run_cb);

    while (1) {
        _delay_ms(500);
        uint16_t r = ADC_GetLastResult();
        UART_SendString("FR: ");
        UART_PrintU16(r);
        UART_SendString("\r\n");
    }
    return 0;
}
#endif

#endif /* AVR_ADC_H */
