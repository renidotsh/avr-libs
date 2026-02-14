/**
 * @file     avr_spi.h
 * @brief    SPI master/slave communication library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Full SPI master and slave driver for ATmega328P.  Supports all four SPI
 * modes (CPOL/CPHA), configurable clock speeds (fosc/4 – fosc/128 plus
 * 2x modes), byte and buffer transfers, and CS pin management helpers.
 *
 * @features
 * - Master and slave mode initialisation
 * - Clock polarity / phase selection (SPI Mode 0-3)
 * - Speed dividers fosc/4 … fosc/128, plus SPI2X for /2, /8, /32
 * - Single-byte and multi-byte transfers
 * - CS (chip-select) assert / deassert helpers
 * - Interrupt-driven slave receive with callback
 *
 * @example
 *   #define AVR_SPI_IMPLEMENTATION
 *   #include "avr_spi.h"
 *
 *   int main(void) {
 *       spi_master_init(SPI_MODE0, SPI_SPEED_DIV16);
 *       SPI_CS_LOW(&PORTB, PB2);
 *       uint8_t rx = spi_transfer_byte(0xAA);
 *       SPI_CS_HIGH(&PORTB, PB2);
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_SPI_H
#define AVR_SPI_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== PIN DEFINITIONS (ATmega328P) ===== */
/* Override these if targeting a different chip */
#ifndef SPI_DDR
#define SPI_DDR   DDRB
#define SPI_PORT  PORTB
#define SPI_MOSI  PB3    /* Master-Out / Slave-In  */
#define SPI_MISO  PB4    /* Master-In  / Slave-Out */
#define SPI_SCK   PB5    /* Serial Clock           */
#define SPI_SS    PB2    /* Slave Select           */
#endif

/* ===== SPI MODE (CPOL | CPHA) ===== */
typedef enum {
    SPI_MODE0 = 0x00,   /**< CPOL=0, CPHA=0 – idle low,  sample leading  */
    SPI_MODE1 = 0x04,   /**< CPOL=0, CPHA=1 – idle low,  sample trailing */
    SPI_MODE2 = 0x08,   /**< CPOL=1, CPHA=0 – idle high, sample leading  */
    SPI_MODE3 = 0x0C    /**< CPOL=1, CPHA=1 – idle high, sample trailing */
} spi_mode_e;

/* ===== SPI CLOCK SPEED ===== */
typedef enum {
    SPI_SPEED_DIV4   = 0x00,   /**< fosc / 4   */
    SPI_SPEED_DIV16  = 0x01,   /**< fosc / 16  */
    SPI_SPEED_DIV64  = 0x02,   /**< fosc / 64  */
    SPI_SPEED_DIV128 = 0x03,   /**< fosc / 128 */
    SPI_SPEED_DIV2   = 0x80,   /**< fosc / 2   (SPI2X) */
    SPI_SPEED_DIV8   = 0x81,   /**< fosc / 8   (SPI2X) */
    SPI_SPEED_DIV32  = 0x82    /**< fosc / 32  (SPI2X) */
} spi_speed_e;

/* ===== DATA ORDER ===== */
#define SPI_MSB_FIRST  0
#define SPI_LSB_FIRST  1

/* ===== CALLBACK TYPE ===== */
typedef void (*spi_callback_t)(uint8_t received);

/* ===== CS MACROS ===== */
#define SPI_CS_LOW(port, pin)   do { *(port) &= ~(1U << (pin)); } while(0)
#define SPI_CS_HIGH(port, pin)  do { *(port) |=  (1U << (pin)); } while(0)

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Initialise SPI as master.
 * @param  mode   SPI clock polarity / phase
 * @param  speed  Clock speed divider
 */
void spi_master_init(spi_mode_e mode, spi_speed_e speed);

/**
 * @brief  Initialise SPI as slave with an optional receive callback.
 * @param  mode   SPI clock polarity / phase
 * @param  cb     Callback invoked from SPI_STC ISR on byte received (or NULL)
 */
void spi_slave_init(spi_mode_e mode, spi_callback_t cb);

/**
 * @brief  Set data order.
 * @param  order  SPI_MSB_FIRST or SPI_LSB_FIRST
 */
void spi_set_data_order(uint8_t order);

/**
 * @brief  Transfer a single byte (full-duplex).
 * @param  data  Byte to send
 * @return Byte received simultaneously
 */
uint8_t spi_transfer_byte(uint8_t data);

/**
 * @brief  Transfer a buffer (send tx_buf, received data stored in rx_buf).
 * @param  tx_buf  Send buffer (NULL to send 0x00 bytes)
 * @param  rx_buf  Receive buffer (NULL to discard received data)
 * @param  len     Number of bytes
 */
void spi_transfer_buffer(const uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len);

/**
 * @brief  Send a byte without reading the response.
 * @param  data  Byte to send
 */
void spi_send_byte(uint8_t data);

/**
 * @brief  Receive a byte (sends 0x00 as dummy).
 * @return Received byte
 */
uint8_t spi_receive_byte(void);

/**
 * @brief  Disable the SPI peripheral.
 */
void spi_disable(void);

/**
 * @brief  Configure a GPIO pin as chip-select output (set HIGH initially).
 * @param  ddr   Pointer to DDRx
 * @param  port  Pointer to PORTx
 * @param  pin   Pin number
 */
void spi_cs_init(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t pin);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_SPI_IMPLEMENTATION

static volatile spi_callback_t _spi_slave_cb = (void*)0;

void spi_master_init(spi_mode_e mode, spi_speed_e speed)
{
    /* MOSI, SCK, SS → output; MISO → input */
    SPI_DDR |= (1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS);
    SPI_DDR &= ~(1 << SPI_MISO);

    /* SS pin HIGH (deselected) */
    SPI_PORT |= (1 << SPI_SS);

    /* SPCR: Enable | Master | mode | speed[1:0] */
    uint8_t spcr = (1 << SPE) | (1 << MSTR) | (uint8_t)mode
                   | ((uint8_t)speed & 0x03);
    SPCR = spcr;

    /* SPI2X flag in SPSR */
    if ((uint8_t)speed & 0x80)
        SPSR |= (1 << SPI2X);
    else
        SPSR &= ~(1 << SPI2X);
}

void spi_slave_init(spi_mode_e mode, spi_callback_t cb)
{
    /* MISO → output; MOSI, SCK, SS → input */
    SPI_DDR |=  (1 << SPI_MISO);
    SPI_DDR &= ~((1 << SPI_MOSI) | (1 << SPI_SCK) | (1 << SPI_SS));

    _spi_slave_cb = cb;

    /* SPCR: Enable | Slave | mode */
    SPCR = (1 << SPE) | (uint8_t)mode;

    if (cb)
        SPCR |= (1 << SPIE);  /* enable SPI interrupt */
}

void spi_set_data_order(uint8_t order)
{
    if (order == SPI_LSB_FIRST)
        SPCR |= (1 << DORD);
    else
        SPCR &= ~(1 << DORD);
}

uint8_t spi_transfer_byte(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & (1 << SPIF)))
        ;
    return SPDR;
}

void spi_transfer_buffer(const uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        uint8_t tx = tx_buf ? tx_buf[i] : 0x00;
        uint8_t rx = spi_transfer_byte(tx);
        if (rx_buf) rx_buf[i] = rx;
    }
}

void spi_send_byte(uint8_t data)
{
    (void)spi_transfer_byte(data);
}

uint8_t spi_receive_byte(void)
{
    return spi_transfer_byte(0x00);
}

void spi_disable(void)
{
    SPCR &= ~(1 << SPE);
}

void spi_cs_init(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t pin)
{
    *ddr  |= (1 << pin);    /* output   */
    *port |= (1 << pin);    /* HIGH (inactive) */
}

ISR(SPI_STC_vect)
{
    uint8_t data = SPDR;
    if (_spi_slave_cb) _spi_slave_cb(data);
}

#endif /* AVR_SPI_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_SPI_IMPLEMENTATION
#include "avr_spi.h"
#include <util/delay.h>

/* Example: Send & receive bytes to a peripheral on PB2 as CS */
int main(void)
{
    spi_master_init(SPI_MODE0, SPI_SPEED_DIV16);

    /* Additional CS pin (external device) */
    spi_cs_init(&DDRB, &PORTB, PB2);

    while (1) {
        SPI_CS_LOW(&PORTB, PB2);

        /* Write command byte, read status */
        spi_send_byte(0x9F);                    /* e.g. JEDEC ID cmd */
        uint8_t mfr  = spi_receive_byte();      /* Manufacturer */
        uint8_t type = spi_receive_byte();      /* Memory type  */
        uint8_t cap  = spi_receive_byte();      /* Capacity     */

        SPI_CS_HIGH(&PORTB, PB2);

        (void)mfr; (void)type; (void)cap;
        _delay_ms(1000);
    }
    return 0;
}
#endif

#endif /* AVR_SPI_H */
