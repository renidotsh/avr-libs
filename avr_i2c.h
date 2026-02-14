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
 *       I2C_Init(I2C_SPEED_100K);
 *       uint8_t val;
 *       I2C_ReadRegister(0x68, 0x75, &val); // WHO_AM_I
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
void I2C_Init(i2c_speed_e speed);

/**
 * @brief  Issue a START condition.
 * @return I2C_OK on success
 */
i2c_status_e I2C_Start(void);

/**
 * @brief  Issue a repeated START condition.
 * @return I2C_OK on success
 */
i2c_status_e I2C_RepStart(void);

/**
 * @brief  Issue a STOP condition.
 */
void I2C_Stop(void);

/**
 * @brief  Send SLA+W (7-bit address + write bit).
 * @param  addr  7-bit slave address
 * @return I2C_OK if ACK received
 */
i2c_status_e I2C_SendAddress_W(uint8_t addr);

/**
 * @brief  Send SLA+R (7-bit address + read bit).
 * @param  addr  7-bit slave address
 * @return I2C_OK if ACK received
 */
i2c_status_e I2C_SendAddress_R(uint8_t addr);

/**
 * @brief  Write a data byte on the bus.
 * @param  data  Byte to send
 * @return I2C_OK if ACK received
 */
i2c_status_e I2C_WriteByte(uint8_t data);

/**
 * @brief  Read a byte and send ACK (more bytes to follow).
 * @param  data  Pointer to store received byte
 * @return I2C_OK on success
 */
i2c_status_e I2C_ReadByte_ACK(uint8_t *data);

/**
 * @brief  Read a byte and send NACK (last byte).
 * @param  data  Pointer to store received byte
 * @return I2C_OK on success
 */
i2c_status_e I2C_ReadByte_NACK(uint8_t *data);

/* ---- High-level convenience ---- */

/**
 * @brief  Write a single byte to a device register.
 * @param  addr  7-bit slave address
 * @param  reg   Register address
 * @param  data  Byte to write
 * @return I2C status
 */
i2c_status_e I2C_WriteRegister(uint8_t addr, uint8_t reg, uint8_t data);

/**
 * @brief  Read a single byte from a device register.
 * @param  addr  7-bit slave address
 * @param  reg   Register address
 * @param  data  Pointer to store the read byte
 * @return I2C status
 */
i2c_status_e I2C_ReadRegister(uint8_t addr, uint8_t reg, uint8_t *data);

/**
 * @brief  Write multiple bytes to consecutive registers.
 * @param  addr  7-bit slave address
 * @param  reg   Starting register address
 * @param  buf   Buffer of data to write
 * @param  len   Number of bytes
 * @return I2C status
 */
i2c_status_e I2C_WriteRegisters(uint8_t addr, uint8_t reg,
                                const uint8_t *buf, uint8_t len);

/**
 * @brief  Read multiple bytes from consecutive registers.
 * @param  addr  7-bit slave address
 * @param  reg   Starting register address
 * @param  buf   Buffer to store read data
 * @param  len   Number of bytes
 * @return I2C status
 */
i2c_status_e I2C_ReadRegisters(uint8_t addr, uint8_t reg,
                               uint8_t *buf, uint8_t len);

/**
 * @brief  Scan the I2C bus for responsive devices.
 * @param  found  Array to store found addresses (at least 128 bytes)
 * @return Number of devices found (0-127)
 */
uint8_t I2C_Scan(uint8_t *found);

/**
 * @brief  Disable the TWI peripheral.
 */
void I2C_Disable(void);

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

void I2C_Init(i2c_speed_e speed)
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

i2c_status_e I2C_Start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    uint8_t st = _i2c_status();
    if (st != I2C_TW_START && st != I2C_TW_REP_START)
        return I2C_ERR_START;
    return I2C_OK;
}

i2c_status_e I2C_RepStart(void)
{
    return I2C_Start();   /* same HW sequence */
}

void I2C_Stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    /* TWINT is NOT set after STOP; wait for TWSTO to clear */
    while (TWCR & (1 << TWSTO))
        ;
}

i2c_status_e I2C_SendAddress_W(uint8_t addr)
{
    TWDR = (addr << 1) | 0;   /* SLA+W */
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    if (_i2c_status() != I2C_TW_MT_SLA_ACK)
        return I2C_ERR_SLA_ACK;
    return I2C_OK;
}

i2c_status_e I2C_SendAddress_R(uint8_t addr)
{
    TWDR = (addr << 1) | 1;   /* SLA+R */
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    if (_i2c_status() != I2C_TW_MR_SLA_ACK)
        return I2C_ERR_SLA_ACK;
    return I2C_OK;
}

i2c_status_e I2C_WriteByte(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    if (_i2c_status() != I2C_TW_MT_DATA_ACK)
        return I2C_ERR_DATA_ACK;
    return I2C_OK;
}

i2c_status_e I2C_ReadByte_ACK(uint8_t *data)
{
    TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    *data = TWDR;
    return I2C_OK;
}

i2c_status_e I2C_ReadByte_NACK(uint8_t *data)
{
    TWCR = (1 << TWINT) | (1 << TWEN);   /* no TWEA */
    i2c_status_e s = _i2c_wait();
    if (s != I2C_OK) return s;

    *data = TWDR;
    return I2C_OK;
}

/* ---- high-level ---- */

i2c_status_e I2C_WriteRegister(uint8_t addr, uint8_t reg, uint8_t data)
{
    i2c_status_e s;
    if ((s = I2C_Start())          != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_SendAddress_W(addr)) != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_WriteByte(reg))      != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_WriteByte(data))     != I2C_OK) { I2C_Stop(); return s; }
    I2C_Stop();
    return I2C_OK;
}

i2c_status_e I2C_ReadRegister(uint8_t addr, uint8_t reg, uint8_t *data)
{
    i2c_status_e s;
    /* Write register address */
    if ((s = I2C_Start())            != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_SendAddress_W(addr)) != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_WriteByte(reg))      != I2C_OK) { I2C_Stop(); return s; }
    /* Repeated start + read */
    if ((s = I2C_RepStart())          != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_SendAddress_R(addr)) != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_ReadByte_NACK(data)) != I2C_OK) { I2C_Stop(); return s; }
    I2C_Stop();
    return I2C_OK;
}

i2c_status_e I2C_WriteRegisters(uint8_t addr, uint8_t reg,
                                const uint8_t *buf, uint8_t len)
{
    i2c_status_e s;
    if ((s = I2C_Start())            != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_SendAddress_W(addr)) != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_WriteByte(reg))      != I2C_OK) { I2C_Stop(); return s; }
    for (uint8_t i = 0; i < len; i++) {
        if ((s = I2C_WriteByte(buf[i])) != I2C_OK) { I2C_Stop(); return s; }
    }
    I2C_Stop();
    return I2C_OK;
}

i2c_status_e I2C_ReadRegisters(uint8_t addr, uint8_t reg,
                               uint8_t *buf, uint8_t len)
{
    i2c_status_e s;
    if ((s = I2C_Start())            != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_SendAddress_W(addr)) != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_WriteByte(reg))      != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_RepStart())          != I2C_OK) { I2C_Stop(); return s; }
    if ((s = I2C_SendAddress_R(addr)) != I2C_OK) { I2C_Stop(); return s; }

    for (uint8_t i = 0; i < len; i++) {
        if (i < len - 1)
            s = I2C_ReadByte_ACK(&buf[i]);
        else
            s = I2C_ReadByte_NACK(&buf[i]);
        if (s != I2C_OK) { I2C_Stop(); return s; }
    }
    I2C_Stop();
    return I2C_OK;
}

uint8_t I2C_Scan(uint8_t *found)
{
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 128; addr++) {
        i2c_status_e s = I2C_Start();
        if (s != I2C_OK) { I2C_Stop(); continue; }

        s = I2C_SendAddress_W(addr);
        I2C_Stop();

        if (s == I2C_OK) {
            if (found) found[count] = addr;
            count++;
        }
    }
    return count;
}

void I2C_Disable(void)
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
    UART_Init(9600);
    sei();
    UART_SendString("I2C Demo\r\n");

    I2C_Init(I2C_SPEED_100K);

    /* Bus scan */
    uint8_t devices[16];
    uint8_t n = I2C_Scan(devices);
    UART_SendString("Found ");
    UART_PrintU16(n);
    UART_SendString(" device(s)\r\n");
    for (uint8_t i = 0; i < n; i++) {
        UART_SendString("  0x");
        UART_PrintHex8(devices[i]);
        UART_SendString("\r\n");
    }

    /* Read WHO_AM_I from MPU6050 (addr 0x68, reg 0x75) */
    uint8_t whoami;
    if (I2C_ReadRegister(0x68, 0x75, &whoami) == I2C_OK) {
        UART_SendString("WHO_AM_I: 0x");
        UART_PrintHex8(whoami);
        UART_SendString("\r\n");
    }

    /* Write config register */
    I2C_WriteRegister(0x68, 0x6B, 0x00);  /* wake up MPU6050 */

    /* Read 6 bytes (accel X/Y/Z) */
    uint8_t accel[6];
    I2C_ReadRegisters(0x68, 0x3B, accel, 6);

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
#endif

#endif /* AVR_I2C_H */
