/**
 * @file     avr_i2c.h
 * @brief    I2C/TWI master communication library for AVR microcontrollers
 * @author   Reni
 * @date     2026
 * @version  1.0
 *
 * @description
 * Master-mode TWI driver for ATmega328P.  Generates START/STOP conditions,
 * handles ACK/NACK, and provides low-level and convenience functions for
 * reading/writing device registers.  Supports 100 kHz (standard) and
 * 400 kHz (fast) SCL frequencies.
 *
 * @features
 * - Master mode with auto TWBR calculation
 * - 100 kHz and 400 kHz clock speeds
 * - START / repeated-START / STOP generation
 * - Write and read with ACK / NACK
 * - High-level register read/write helpers
 * - Status code error returns
 * - Bus scan utility
 *
 * @example
 *   #define AVR_I2C_IMPLEMENTATION
 *   #include "avr_i2c.h"
 *
 *   int main(void) {
 *       i2c_init(I2C_SPEED_100K);
 *       uint8_t val;
 *       i2c_read_register(0x68, 0x75, &val); // WHO_AM_I
 *   }
 *
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */

#ifndef AVR_I2C_H
#define AVR_I2C_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== CONFIGURATION ===== */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/** I2C timeout (loop iterations).  Set to 0 to disable. */
#ifndef I2C_TIMEOUT
#define I2C_TIMEOUT 20000U
#endif

/* ===== SPEED ===== */
typedef enum {
    I2C_SPEED_100K = 100000UL,  /**< Standard mode  */
    I2C_SPEED_400K = 400000UL   /**< Fast mode      */
} i2c_speed_e;

/* ===== ERROR / STATUS CODES ===== */
typedef enum {
    I2C_OK           = 0,
    I2C_ERR_START    = 1,    /**< Failed to issue START        */
    I2C_ERR_SLA_ACK  = 2,    /**< Slave did not ACK address    */
    I2C_ERR_DATA_ACK = 3,    /**< Slave did not ACK data byte  */
    I2C_ERR_TIMEOUT  = 4,    /**< Bus operation timed out      */
    I2C_ERR_BUS      = 5     /**< Unexpected bus state         */
} i2c_status_e;

/* ===== TWI STATUS CODES (datasheet Table 22-2 ff.) ===== */
#define I2C_TW_START       0x08
#define I2C_TW_REP_START   0x10
#define I2C_TW_MT_SLA_ACK  0x18
#define I2C_TW_MT_SLA_NACK 0x20
#define I2C_TW_MT_DATA_ACK 0x28
#define I2C_TW_MR_SLA_ACK  0x40
#define I2C_TW_MR_SLA_NACK 0x48
#define I2C_TW_MR_DATA_ACK 0x50
#define I2C_TW_MR_DATA_NACK 0x58

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */

/**
 * @brief  Initialise TWI hardware.
 * @param  speed  SCL frequency (I2C_SPEED_100K or I2C_SPEED_400K)
 */
void i2c_init(i2c_speed_e speed);

/**
 * @brief  Issue a START condition.
 * @return I2C_OK on success
 */
i2c_status_e i2c_start(void);

/**
 * @brief  Issue a repeated START condition.
 * @return I2C_OK on success
 */
i2c_status_e i2c_rep_start(void);

/**
 * @brief  Issue a STOP condition.
 */
void i2c_stop(void);

/**
 * @brief  Send SLA+W (7-bit address + write bit).
 * @param  addr  7-bit slave address
 * @return I2C_OK if ACK received
 */
i2c_status_e i2c_send_address_w(uint8_t addr);

/**
 * @brief  Send SLA+R (7-bit address + read bit).
 * @param  addr  7-bit slave address
 * @return I2C_OK if ACK received
 */
i2c_status_e i2c_send_address_r(uint8_t addr);

/**
 * @brief  Write a data byte on the bus.
 * @param  data  Byte to send
 * @return I2C_OK if ACK received
 */
i2c_status_e i2c_write_byte(uint8_t data);

/**
 * @brief  Read a byte and send ACK (more bytes to follow).
 * @param  data  Pointer to store received byte
 * @return I2C_OK on success
 */
i2c_status_e i2c_read_byte_ack(uint8_t *data);

/**
 * @brief  Read a byte and send NACK (last byte).
 * @param  data  Pointer to store received byte
 * @return I2C_OK on success
 */
i2c_status_e i2c_read_byte_nack(uint8_t *data);

/* ---- High-level convenience ---- */

/**
 * @brief  Write a single byte to a device register.
 * @param  addr  7-bit slave address
 * @param  reg   Register address
 * @param  data  Byte to write
 * @return I2C status
 */
i2c_status_e i2c_write_register(uint8_t addr, uint8_t reg, uint8_t data);

/**
 * @brief  Read a single byte from a device register.
 * @param  addr  7-bit slave address
 * @param  reg   Register address
 * @param  data  Pointer to store the read byte
 * @return I2C status
 */
i2c_status_e i2c_read_register(uint8_t addr, uint8_t reg, uint8_t *data);

/**
 * @brief  Write multiple bytes to consecutive registers.
 * @param  addr  7-bit slave address
 * @param  reg   Starting register address
 * @param  buf   Buffer of data to write
 * @param  len   Number of bytes
 * @return I2C status
 */
i2c_status_e i2c_write_registers(uint8_t addr, uint8_t reg,
                                const uint8_t *buf, uint8_t len);

/**
 * @brief  Read multiple bytes from consecutive registers.
 * @param  addr  7-bit slave address
 * @param  reg   Starting register address
 * @param  buf   Buffer to store read data
 * @param  len   Number of bytes
 * @return I2C status
 */
i2c_status_e i2c_read_registers(uint8_t addr, uint8_t reg,
                               uint8_t *buf, uint8_t len);

/**
 * @brief  Scan the I2C bus for responsive devices.
 * @param  found  Array to store found addresses (at least 128 bytes)
 * @return Number of devices found (0-127)
 */
uint8_t i2c_scan(uint8_t *found);

/**
 * @brief  Disable the TWI peripheral.
 */
void i2c_disable(void);

/* ===== IMPLEMENTATION ===== */
#ifdef AVR_I2C_IMPLEMENTATION

/* wait for TWINT with optional timeout */
static i2c_status_e _i2c_wait(void)
{
#if I2C_TIMEOUT > 0
    uint16_t t = I2C_TIMEOUT;
    while (!(TWCR & (1 << TWINT))) {
        if (--t == 0) return I2C_ERR_TIMEOUT;
    }
#else
    while (!(TWCR & (1 << TWINT)))
        ;
#endif
    return I2C_OK;
}

static uint8_t _i2c_status(void)
{
    return TWSR & 0xF8;  /* mask prescaler bits */
}

void i2c_init(i2c_speed_e speed)
{
    /*
     * SCL frequency = F_CPU / (16 + 2 * TWBR * prescaler)
     * With prescaler = 1 (TWPS=0):
     *   TWBR = (F_CPU / SCL - 16) / 2
     */
    TWSR = 0;   /* prescaler = 1 */

    uint32_t scl = (uint32_t)speed;
    uint8_t twbr = (uint8_t)((F_CPU / scl - 16UL) / 2UL);
    TWBR = twbr;

    /* Enable TWI */
    TWCR = (1 << TWEN);
}

i2c_status_e i2c_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    uint8_t st = _i2c_status();
    if (st != I2C_TW_START && st != I2C_TW_REP_START)
        return I2C_ERR_START;
    return I2C_OK;
}

i2c_status_e i2c_rep_start(void)
{
    return i2c_start();   /* same HW sequence */
}

void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    /* TWINT is NOT set after STOP; wait for TWSTO to clear */
    while (TWCR & (1 << TWSTO))
        ;
}

i2c_status_e i2c_send_address_w(uint8_t addr)
{
    TWDR = (addr << 1) | 0;   /* SLA+W */
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    if (_i2c_status() != I2C_TW_MT_SLA_ACK)
        return I2C_ERR_SLA_ACK;
    return I2C_OK;
}

i2c_status_e i2c_send_address_r(uint8_t addr)
{
    TWDR = (addr << 1) | 1;   /* SLA+R */
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    if (_i2c_status() != I2C_TW_MR_SLA_ACK)
        return I2C_ERR_SLA_ACK;
    return I2C_OK;
}

i2c_status_e i2c_write_byte(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    if (_i2c_status() != I2C_TW_MT_DATA_ACK)
        return I2C_ERR_DATA_ACK;
    return I2C_OK;
}

i2c_status_e i2c_read_byte_ack(uint8_t *data)
{
    TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    *data = TWDR;
    return I2C_OK;
}

i2c_status_e i2c_read_byte_nack(uint8_t *data)
{
    TWCR = (1 << TWINT) | (1 << TWEN);   /* no TWEA */
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    *data = TWDR;
    return I2C_OK;
}

/* ---- high-level ---- */

i2c_status_e i2c_write_register(uint8_t addr, uint8_t reg, uint8_t data)
{
    i2c_status_e s;
    if ((s = i2c_start())          != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_send_address_w(addr)) != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_write_byte(reg))      != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_write_byte(data))     != I2C_OK) { i2c_stop(); return s; }
    i2c_stop();
    return I2C_OK;
}

i2c_status_e i2c_read_register(uint8_t addr, uint8_t reg, uint8_t *data)
{
    i2c_status_e s;
    /* Write register address */
    if ((s = i2c_start())            != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_send_address_w(addr)) != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_write_byte(reg))      != I2C_OK) { i2c_stop(); return s; }
    /* Repeated start + read */
    if ((s = i2c_rep_start())          != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_send_address_r(addr)) != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_read_byte_nack(data)) != I2C_OK) { i2c_stop(); return s; }
    i2c_stop();
    return I2C_OK;
}

i2c_status_e i2c_write_registers(uint8_t addr, uint8_t reg,
                                const uint8_t *buf, uint8_t len)
{
    i2c_status_e s;
    if ((s = i2c_start())            != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_send_address_w(addr)) != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_write_byte(reg))      != I2C_OK) { i2c_stop(); return s; }
    for (uint8_t i = 0; i < len; i++) {
        if ((s = i2c_write_byte(buf[i])) != I2C_OK) { i2c_stop(); return s; }
    }
    i2c_stop();
    return I2C_OK;
}

i2c_status_e i2c_read_registers(uint8_t addr, uint8_t reg,
                               uint8_t *buf, uint8_t len)
{
    i2c_status_e s;
    if ((s = i2c_start())            != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_send_address_w(addr)) != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_write_byte(reg))      != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_rep_start())          != I2C_OK) { i2c_stop(); return s; }
    if ((s = i2c_send_address_r(addr)) != I2C_OK) { i2c_stop(); return s; }

    for (uint8_t i = 0; i < len; i++) {
        if (i < len - 1)
            s = i2c_read_byte_ack(&buf[i]);
        else
            s = i2c_read_byte_nack(&buf[i]);
        if (s != I2C_OK) { i2c_stop(); return s; }
    }
    i2c_stop();
    return I2C_OK;
}

uint8_t i2c_scan(uint8_t *found)
{
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 128; addr++) {
        i2c_status_e s = i2c_start();
        if (s != I2C_OK) { i2c_stop(); continue; }

        s = i2c_send_address_w(addr);
        i2c_stop();

        if (s == I2C_OK) {
            if (found) found[count] = addr;
            count++;
        }
    }
    return count;
}

void i2c_disable(void)
{
    TWCR = 0;
}

#endif /* AVR_I2C_IMPLEMENTATION */

/* ===== EXAMPLE USAGE ===== */
#if 0
#define F_CPU 16000000UL
#define AVR_I2C_IMPLEMENTATION
#include "avr_i2c.h"

/* Also need UART for bus-scan output */
#define AVR_UART_IMPLEMENTATION
#include "avr_uart.h"
#include <util/delay.h>

int main(void)
{
    uart_init(9600);
    sei();
    uart_send_string("I2C Demo\r\n");

    i2c_init(I2C_SPEED_100K);

    /* Bus scan */
    uint8_t devices[16];
    uint8_t n = i2c_scan(devices);
    uart_send_string("Found ");
    uart_print_u16(n);
    uart_send_string(" device(s)\r\n");
    for (uint8_t i = 0; i < n; i++) {
        uart_send_string("  0x");
        uart_print_hex8(devices[i]);
        uart_send_string("\r\n");
    }

    /* Read WHO_AM_I from MPU6050 (addr 0x68, reg 0x75) */
    uint8_t whoami;
    if (i2c_read_register(0x68, 0x75, &whoami) == I2C_OK) {
        uart_send_string("WHO_AM_I: 0x");
        uart_print_hex8(whoami);
        uart_send_string("\r\n");
    }

    /* Write config register */
    i2c_write_register(0x68, 0x6B, 0x00);  /* wake up MPU6050 */

    /* Read 6 bytes (accel X/Y/Z) */
    uint8_t accel[6];
    i2c_read_registers(0x68, 0x3B, accel, 6);

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
#endif

#endif /* AVR_I2C_H */
