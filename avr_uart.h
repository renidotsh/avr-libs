/**
 * @file     avr_uart.h
 * @brief    UART serial communication library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Interrupt-driven USART driver with configurable ring buffers for TX and
 * RX.  Supports blocking and non-blocking send/receive, baud-rate auto-
 * calculation, frame/parity/overrun error detection, and optional printf
 * integration via FDEV_SETUP_STREAM.
 *
 * @features
 * - Automatic UBRR calculation from F_CPU
 * - Interrupt-driven TX/RX with ring buffers
 * - Blocking and non-blocking APIs
 * - Error detection (frame, parity, overrun)
 * - printf redirect support
 * - String and buffer send helpers
 *
 * @example
 *   #define AVR_UART_IMPLEMENTATION
 *   #include "avr_uart.h"
 *
 *   int main(void) {
 *       uart_init(9600);
 *       sei();
 *       uart_send_string("Hello, world!\r\n");
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_UART_H
#define AVR_UART_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>         /* for FDEV_SETUP_STREAM / FILE */

/* ===== CONFIGURATION ===== */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/** TX ring-buffer size – must be power of 2 */
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif

/** RX ring-buffer size – must be power of 2 */
#ifndef UART_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE 64
#endif

/* ===== ERROR FLAGS ===== */
#define UART_ERR_NONE       0x00
#define UART_ERR_FRAME      0x01
#define UART_ERR_PARITY     0x02
#define UART_ERR_OVERRUN    0x04
#define UART_ERR_BUFOVF     0x08   /**< Software buffer overflow */

/* ===== REGISTER PORTABILITY ===== */
/* Map generic names to USART0 registers for ATmega328P.
   Override for other chips by defining before including this header. */
#ifndef UART_UDR
  #if defined(UDR0)
    #define UART_UDR      UDR0
    #define UART_UCSRA    UCSR0A
    #define UART_UCSRB    UCSR0B
    #define UART_UCSRC    UCSR0C
    #define UART_UBRRH    UBRR0H
    #define UART_UBRRL    UBRR0L
    #define UART_RXC_vect USART_RX_vect
    #define UART_TXC_vect USART_TX_vect
    #define UART_UDRE_vect USART_UDRE_vect
    #define UART_RXEN     RXEN0
    #define UART_TXEN     TXEN0
    #define UART_RXCIE    RXCIE0
    #define UART_UDRIE    UDRIE0
    #define UART_U2X      U2X0
    #define UART_FE       FE0
    #define UART_DOR      DOR0
    #define UART_UPE      UPE0
    #define UART_UCSZ0    UCSZ00
    #define UART_UCSZ1    UCSZ01
  #elif defined(UDR)
    /* Older chips (ATmega8, ATmega16) */
    #define UART_UDR      UDR
    #define UART_UCSRA    UCSRA
    #define UART_UCSRB    UCSRB
    #define UART_UCSRC    UCSRC
    #define UART_UBRRH    UBRRH
    #define UART_UBRRL    UBRRL
    #define UART_RXC_vect USART_RXC_vect
    #define UART_UDRE_vect USART_UDRE_vect
    #define UART_RXEN     RXEN
    #define UART_TXEN     TXEN
    #define UART_RXCIE    RXCIE
    #define UART_UDRIE    UDRIE
    #define UART_U2X      U2X
    #define UART_FE       FE
    #define UART_DOR      DOR
    #define UART_UPE      PE
    #define UART_UCSZ0    UCSZ0
    #define UART_UCSZ1    UCSZ1
  #endif
#endif

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Initialise USART with given baud rate (8N1, interrupt-driven).
 * @param  baud  Desired baud rate (e.g. 9600, 115200)
 */
void uart_init(uint32_t baud);

/**
 * @brief  Send a single byte (interrupt-driven, returns immediately if room).
 * @param  data  Byte to send
 */
void uart_send_byte(uint8_t data);

/**
 * @brief  Send a null-terminated string.
 * @param  str  Pointer to C string
 */
void uart_send_string(const char *str);

/**
 * @brief  Send a buffer of arbitrary bytes.
 * @param  buf  Byte buffer
 * @param  len  Number of bytes
 */
void uart_send_buffer(const uint8_t *buf, uint8_t len);

/**
 * @brief  Send byte, blocking until TX buffer has room.
 * @param  data  Byte to send
 */
void uart_send_byte_blocking(uint8_t data);

/**
 * @brief  Check how many bytes are available in the RX buffer.
 * @return Number of unread bytes
 */
uint8_t uart_available(void);

/**
 * @brief  Read a byte from the RX buffer (non-blocking).
 * @return Received byte, or 0 if buffer empty (check uart_available first)
 */
uint8_t uart_read_byte(void);

/**
 * @brief  Read a byte, blocking until one arrives.
 * @return Received byte
 */
uint8_t uart_read_byte_blocking(void);

/**
 * @brief  Get and clear accumulated error flags.
 * @return Bitmask of UART_ERR_* flags
 */
uint8_t uart_get_errors(void);

/**
 * @brief  Flush the TX buffer (block until all bytes sent).
 */
void uart_flush_tx(void);

/**
 * @brief  Discard all data in the RX buffer.
 */
void uart_flush_rx(void);

/**
 * @brief  Get a FILE* suitable for use with fprintf / printf.
 *         Redirects stdout (and optionally stdin) to UART.
 * @return Pointer to static FILE structure (valid for lifetime of program).
 *
 * @usage
 *   stdout = uart_get_stream();
 *   printf("Value = %d\n", val);
 */
FILE *uart_get_stream(void);

/**
 * @brief  Send an unsigned 16-bit integer as ASCII decimal.
 * @param  val  Value to print
 */
void uart_print_u16(uint16_t val);

/**
 * @brief  Send a signed 16-bit integer as ASCII decimal.
 * @param  val  Value to print
 */
void uart_print_s16(int16_t val);

/**
 * @brief  Send an 8-bit value as two ASCII hex digits.
 * @param  val  Value to print
 */
void uart_print_hex8(uint8_t val);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_UART_IMPLEMENTATION

/* ---- ring buffer helpers ---- */
#define _UART_TX_MASK (UART_TX_BUFFER_SIZE - 1)
#define _UART_RX_MASK (UART_RX_BUFFER_SIZE - 1)

static volatile uint8_t _tx_buf[UART_TX_BUFFER_SIZE];
static volatile uint8_t _tx_head = 0;
static volatile uint8_t _tx_tail = 0;

static volatile uint8_t _rx_buf[UART_RX_BUFFER_SIZE];
static volatile uint8_t _rx_head = 0;
static volatile uint8_t _rx_tail = 0;

static volatile uint8_t _uart_errors = UART_ERR_NONE;

/* ---- init ---- */

void uart_init(uint32_t baud)
{
    /* Calculate UBRR with rounding for normal speed */
    uint16_t ubrr = (uint16_t)((F_CPU + 8UL * baud) / (16UL * baud) - 1);

    /* Check if double-speed would give better accuracy */
    uint32_t actual_normal = F_CPU / (16UL * ((uint32_t)ubrr + 1));
    uint16_t ubrr2x = (uint16_t)((F_CPU + 4UL * baud) / (8UL * baud) - 1);
    uint32_t actual_2x = F_CPU / (8UL * ((uint32_t)ubrr2x + 1));

    int32_t err_normal = (int32_t)actual_normal - (int32_t)baud;
    int32_t err_2x     = (int32_t)actual_2x     - (int32_t)baud;
    if (err_normal < 0) err_normal = -err_normal;
    if (err_2x     < 0) err_2x     = -err_2x;

    if (err_2x < err_normal) {
        UART_UCSRA = (1 << UART_U2X);      /* double speed         */
        UART_UBRRH = (uint8_t)(ubrr2x >> 8);
        UART_UBRRL = (uint8_t)(ubrr2x);
    } else {
        UART_UCSRA = 0;
        UART_UBRRH = (uint8_t)(ubrr >> 8);
        UART_UBRRL = (uint8_t)(ubrr);
    }

    /* 8N1 frame format */
    UART_UCSRC = (1 << UART_UCSZ1) | (1 << UART_UCSZ0);

    /* Enable TX, RX, and RX-complete interrupt */
    UART_UCSRB = (1 << UART_RXEN) | (1 << UART_TXEN) | (1 << UART_RXCIE);

    /* Clear buffers */
    _tx_head = _tx_tail = 0;
    _rx_head = _rx_tail = 0;
    _uart_errors = UART_ERR_NONE;
}

/* ---- transmit ---- */

void uart_send_byte(uint8_t data)
{
    uint8_t next = (_tx_head + 1) & _UART_TX_MASK;

    /* Wait if buffer full */
    while (next == _tx_tail)
        ;

    _tx_buf[_tx_head] = data;
    _tx_head = next;

    /* Enable UDRE interrupt to start draining */
    UART_UCSRB |= (1 << UART_UDRIE);
}

void uart_send_byte_blocking(uint8_t data)
{
    uart_send_byte(data);  /* already blocks if full */
}

void uart_send_string(const char *str)
{
    while (*str)
        uart_send_byte((uint8_t)*str++);
}

void uart_send_buffer(const uint8_t *buf, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
        uart_send_byte(buf[i]);
}

void uart_flush_tx(void)
{
    while (_tx_head != _tx_tail)
        ;
}

/* ---- receive ---- */

uint8_t uart_available(void)
{
    return (_rx_head - _rx_tail) & _UART_RX_MASK;
}

uint8_t uart_read_byte(void)
{
    if (_rx_head == _rx_tail)
        return 0;

    uint8_t data = _rx_buf[_rx_tail];
    _rx_tail = (_rx_tail + 1) & _UART_RX_MASK;
    return data;
}

uint8_t uart_read_byte_blocking(void)
{
    while (_rx_head == _rx_tail)
        ;
    return uart_read_byte();
}

void uart_flush_rx(void)
{
    _rx_tail = _rx_head;
}

/* ---- errors ---- */

uint8_t uart_get_errors(void)
{
    uint8_t e = _uart_errors;
    _uart_errors = UART_ERR_NONE;
    return e;
}

/* ---- printf stream ---- */

static int _uart_putchar(char c, FILE *stream)
{
    (void)stream;
    if (c == '\n')
        uart_send_byte('\r');
    uart_send_byte((uint8_t)c);
    return 0;
}

static int _uart_getchar(FILE *stream)
{
    (void)stream;
    return (int)uart_read_byte_blocking();
}

static FILE _uart_file = FDEV_SETUP_STREAM(_uart_putchar, _uart_getchar,
                                           _FDEV_SETUP_RW);

FILE *uart_get_stream(void)
{
    return &_uart_file;
}

/* ---- convenience print ---- */

void uart_print_u16(uint16_t val)
{
    char buf[6];
    int8_t i = 0;

    if (val == 0) {
        uart_send_byte('0');
        return;
    }

    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (--i >= 0)
        uart_send_byte((uint8_t)buf[i]);
}

void uart_print_s16(int16_t val)
{
    if (val < 0) {
        uart_send_byte('-');
        val = -val;
    }
    uart_print_u16((uint16_t)val);
}

void uart_print_hex8(uint8_t val)
{
    static const char hex[] = "0123456789ABCDEF";
    uart_send_byte((uint8_t)hex[val >> 4]);
    uart_send_byte((uint8_t)hex[val & 0x0F]);
}

/* ---- ISRs ---- */

ISR(UART_RXC_vect)
{
    /* Read error flags BEFORE reading UDR (reading UDR clears them) */
    uint8_t status = UART_UCSRA;

    if (status & (1 << UART_FE))
        _uart_errors |= UART_ERR_FRAME;
    if (status & (1 << UART_DOR))
        _uart_errors |= UART_ERR_OVERRUN;
    if (status & (1 << UART_UPE))
        _uart_errors |= UART_ERR_PARITY;

    uint8_t data = UART_UDR;
    uint8_t next = (_rx_head + 1) & _UART_RX_MASK;

    if (next != _rx_tail) {
        _rx_buf[_rx_head] = data;
        _rx_head = next;
    } else {
        _uart_errors |= UART_ERR_BUFOVF;   /* software overrun */
    }
}

ISR(UART_UDRE_vect)
{
    if (_tx_head != _tx_tail) {
        UART_UDR  = _tx_buf[_tx_tail];
        _tx_tail = (_tx_tail + 1) & _UART_TX_MASK;
    } else {
        /* Nothing left to send – disable UDRE interrupt */
        UART_UCSRB &= ~(1 << UART_UDRIE);
    }
}

#endif /* AVR_UART_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_UART_IMPLEMENTATION
#include "avr_uart.h"

int main(void)
{
    uart_init(9600);
    sei();

    /* Redirect stdout */
    stdout = uart_get_stream();
    printf("UART ready @ 9600 baud\n");

    /* Send string and hex */
    uart_send_string("Byte: 0x");
    uart_print_hex8(0xAB);
    uart_send_string("\r\n");

    /* Echo loop */
    while (1) {
        if (uart_available()) {
            uint8_t c = uart_read_byte();
            uart_send_byte(c);           /* echo */

            /* Check for errors */
            uint8_t err = uart_get_errors();
            if (err & UART_ERR_FRAME)
                uart_send_string("[FE]");
            if (err & UART_ERR_OVERRUN)
                uart_send_string("[OVR]");
        }
    }
    return 0;
}
#endif

#endif /* AVR_UART_H */
