/**
 * @file     avr_wdt.h
 * @brief    Watchdog timer library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Configures the AVR watchdog timer for system-reset, interrupt, or
 * interrupt-then-reset modes.  Implements the timed-sequence enable /
 * disable procedures required by the WDTCSR register, and provides
 * reset-cause detection via MCUSR.
 *
 * @features
 * - Timeout periods from 15 ms to 8 s
 * - System reset mode
 * - Interrupt mode (periodic wake / processing)
 * - Combined interrupt + reset mode
 * - Safe enable / disable timed sequences
 * - Reset cause detection (POR, EXT, BOD, WDT)
 *
 * @example
 *   #define AVR_WDT_IMPLEMENTATION
 *   #include "avr_wdt.h"
 *
 *   int main(void) {
 *       WDT_Enable(WDT_TIMEOUT_1S, WDT_MODE_RESET);
 *       while (1) { WDT_Reset(); }
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_WDT_H
#define AVR_WDT_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== TIMEOUT PERIODS ===== */
typedef enum {
    WDT_TIMEOUT_15MS  = 0,
    WDT_TIMEOUT_30MS  = 1,
    WDT_TIMEOUT_60MS  = 2,
    WDT_TIMEOUT_120MS = 3,
    WDT_TIMEOUT_250MS = 4,
    WDT_TIMEOUT_500MS = 5,
    WDT_TIMEOUT_1S    = 6,
    WDT_TIMEOUT_2S    = 7,
    WDT_TIMEOUT_4S    = 8,
    WDT_TIMEOUT_8S    = 9
} wdt_timeout_e;

/* ===== WDT MODES ===== */
typedef enum {
    WDT_MODE_RESET     = 0,   /**< System reset on timeout             */
    WDT_MODE_INTERRUPT = 1,   /**< ISR fires on timeout (no reset)     */
    WDT_MODE_INT_RESET = 2    /**< ISR fires first, then reset on next */
} wdt_mode_e;

/* ===== RESET CAUSE FLAGS ===== */
#define WDT_RESET_POWER_ON   (1 << PORF)    /**< Power-on reset      */
#define WDT_RESET_EXTERNAL   (1 << EXTRF)   /**< External reset pin  */
#define WDT_RESET_BROWN_OUT  (1 << BORF)    /**< Brown-out reset     */
#define WDT_RESET_WATCHDOG   (1 << WDRF)    /**< Watchdog reset      */

/* ===== CALLBACK TYPE ===== */
typedef void (*wdt_callback_t)(void);

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Enable the watchdog timer.
 * @param  timeout  Timeout period
 * @param  mode     Operating mode (reset / interrupt / both)
 */
void WDT_Enable(wdt_timeout_e timeout, wdt_mode_e mode);

/**
 * @brief  Disable the watchdog timer (timed sequence).
 * @note   Must be called within 4 clock cycles of setting WDCE.
 */
void WDT_Disable(void);

/**
 * @brief  Reset (kick) the watchdog counter.
 *         Call this periodically to prevent watchdog timeout.
 */
static inline void WDT_Reset(void)
{
    wdt_reset();
}

/**
 * @brief  Register a callback for WDT interrupt mode.
 * @param  cb  Function to call from WDT ISR (or NULL)
 */
void WDT_SetCallback(wdt_callback_t cb);

/**
 * @brief  Read the reset cause from MCUSR.
 * @return Bitmask of WDT_RESET_* flags
 * @note   Clears MCUSR after reading.  Call once at startup.
 */
uint8_t WDT_GetResetCause(void);

/**
 * @brief  Force an immediate system reset via the watchdog.
 *         Enables WDT with shortest timeout and enters infinite loop.
 */
void WDT_ForceReset(void) __attribute__((noreturn));

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_WDT_IMPLEMENTATION

static volatile wdt_callback_t _wdt_cb = (void*)0;

void WDT_Enable(wdt_timeout_e timeout, wdt_mode_e mode)
{
    uint8_t wdp  = (uint8_t)timeout;
    uint8_t sreg = SREG;
    cli();

    WDT_Reset();

    /*
     * Timed sequence (datasheet §11.8.2):
     * 1. Write WDCE + WDE = 1 in same operation
     * 2. Within 4 cycles, write desired WDP/WDE/WDIE bits
     */

    /* Build desired WDTCSR value */
    uint8_t val = 0;

    /* Prescaler bits: WDP3 is bit 5, WDP[2:0] are bits [2:0] */
    val |= (wdp & 0x07);          /* WDP[2:0] */
    if (wdp & 0x08)
        val |= (1 << WDP3);       /* WDP3     */

    switch (mode) {
        case WDT_MODE_RESET:
            val |= (1 << WDE);
            break;
        case WDT_MODE_INTERRUPT:
            val |= (1 << WDIE);
            break;
        case WDT_MODE_INT_RESET:
            val |= (1 << WDE) | (1 << WDIE);
            break;
    }

    /* Step 1: set WDCE + WDE */
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    /* Step 2: write final config (must happen within 4 cycles) */
    WDTCSR = val;

    SREG = sreg;
}

void WDT_Disable(void)
{
    uint8_t sreg = SREG;
    cli();

    WDT_Reset();

    /* Clear WDRF in MCUSR – required to disable WDT */
    MCUSR &= ~(1 << WDRF);

    /* Timed sequence */
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = 0x00;

    SREG = sreg;
}

void WDT_SetCallback(wdt_callback_t cb)
{
    _wdt_cb = cb;
}

uint8_t WDT_GetResetCause(void)
{
    uint8_t cause = MCUSR;
    MCUSR = 0;      /* clear for next detection */
    return cause;
}

void WDT_ForceReset(void)
{
    cli();
    WDT_Enable(WDT_TIMEOUT_15MS, WDT_MODE_RESET);
    while (1)
        ;  /* wait for reset */
}

ISR(WDT_vect)
{
    if (_wdt_cb) _wdt_cb();
}

#endif /* AVR_WDT_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_WDT_IMPLEMENTATION
#include "avr_wdt.h"

#define AVR_UART_IMPLEMENTATION
#include "avr_uart.h"

static volatile uint8_t wdt_fired = 0;

void wdt_handler(void)
{
    wdt_fired = 1;
}

int main(void)
{
    UART_Init(9600);
    sei();

    /* Check reset cause */
    uint8_t cause = WDT_GetResetCause();
    UART_SendString("Reset cause: 0x");
    UART_PrintHex8(cause);
    UART_SendString("\r\n");

    if (cause & WDT_RESET_WATCHDOG)
        UART_SendString("  -> Watchdog reset!\r\n");

    /* --- Mode 1: Reset mode --- */
    UART_SendString("WDT reset mode (1s)\r\n");
    WDT_Enable(WDT_TIMEOUT_1S, WDT_MODE_RESET);
    for (uint8_t i = 0; i < 5; i++) {
        WDT_Reset();            /* kick the dog */
        UART_SendString("Kick\r\n");
        /* _delay_ms(500) equivalent busy loop */
        for (volatile uint32_t d = 0; d < 800000UL; d++);
    }
    WDT_Disable();
    UART_SendString("WDT disabled\r\n");

    /* --- Mode 2: Interrupt mode --- */
    UART_SendString("WDT interrupt mode (500ms)\r\n");
    WDT_SetCallback(wdt_handler);
    WDT_Enable(WDT_TIMEOUT_500MS, WDT_MODE_INTERRUPT);

    while (1) {
        if (wdt_fired) {
            wdt_fired = 0;
            UART_SendString("WDT ISR\r\n");
            /* Re-enable WDIE (cleared after ISR fires) */
            WDT_Enable(WDT_TIMEOUT_500MS, WDT_MODE_INTERRUPT);
        }
    }
    return 0;
}
#endif

#endif /* AVR_WDT_H */
