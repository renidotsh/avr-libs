/**
 * @file    i2c_example.c
 * @brief   ATmega328P example exercising every avr_i2c.h feature
 * @target  ATmega328P @ 16 MHz
 */
#define F_CPU 16000000UL

#define AVR_I2C_IMPLEMENTATION
#include "../avr_i2c.h"
#define AVR_UART_IMPLEMENTATION
#include "../avr_uart.h"
#include <util/delay.h>

#define MPU6050_ADDR  0x68
#define WHO_AM_I_REG  0x75
#define PWR_MGMT_REG  0x6B
#define ACCEL_REG     0x3B

int main(void)
{
    UART_Init(9600);
    sei();
    UART_SendString("I2C Demo\r\n");

    /* --- Init at 100 kHz --- */
    I2C_Init(I2C_SPEED_100K);

    /* --- Bus scan --- */
    uint8_t devices[16];
    uint8_t n = I2C_Scan(devices);
    UART_SendString("Scan found ");
    UART_PrintU16(n);
    UART_SendString(" device(s):\r\n");
    for (uint8_t i = 0; i < n; i++) {
        UART_SendString("  0x");
        UART_PrintHex8(devices[i]);
        UART_SendString("\r\n");
    }

    /* --- Low-level: Start, SLA+W, Write, Stop --- */
    i2c_status_e s;
    s = I2C_Start();
    if (s == I2C_OK) {
        s = I2C_SendAddress_W(MPU6050_ADDR);
        if (s == I2C_OK) {
            I2C_WriteByte(PWR_MGMT_REG);
            I2C_WriteByte(0x00);  /* wake up */
        }
    }
    I2C_Stop();

    /* --- High-level: WriteRegister --- */
    I2C_WriteRegister(MPU6050_ADDR, PWR_MGMT_REG, 0x00);

    /* --- High-level: ReadRegister --- */
    uint8_t whoami = 0;
    s = I2C_ReadRegister(MPU6050_ADDR, WHO_AM_I_REG, &whoami);
    UART_SendString("WHO_AM_I: 0x");
    UART_PrintHex8(whoami);
    UART_SendString(" (status=");
    UART_PrintU16(s);
    UART_SendString(")\r\n");

    /* --- ReadRegisters (burst 6 bytes – accel XYZ) --- */
    uint8_t accel_data[6];
    s = I2C_ReadRegisters(MPU6050_ADDR, ACCEL_REG, accel_data, 6);
    if (s == I2C_OK) {
        int16_t ax = (int16_t)((accel_data[0] << 8) | accel_data[1]);
        int16_t ay = (int16_t)((accel_data[2] << 8) | accel_data[3]);
        int16_t az = (int16_t)((accel_data[4] << 8) | accel_data[5]);
        UART_SendString("AX="); UART_PrintS16(ax);
        UART_SendString(" AY="); UART_PrintS16(ay);
        UART_SendString(" AZ="); UART_PrintS16(az);
        UART_SendString("\r\n");
    }

    /* --- WriteRegisters (burst write) --- */
    uint8_t config_data[] = {0x00, 0x08};  /* config + gyro config */
    I2C_WriteRegisters(MPU6050_ADDR, 0x1A, config_data, 2);

    /* --- Low-level: Start, SLA+R, Read with ACK/NACK --- */
    s = I2C_Start();
    if (s == I2C_OK) s = I2C_SendAddress_W(MPU6050_ADDR);
    if (s == I2C_OK) s = I2C_WriteByte(ACCEL_REG);
    if (s == I2C_OK) s = I2C_RepStart();
    if (s == I2C_OK) s = I2C_SendAddress_R(MPU6050_ADDR);
    if (s == I2C_OK) {
        uint8_t b0, b1;
        I2C_ReadByte_ACK(&b0);
        I2C_ReadByte_NACK(&b1);
        UART_SendString("Raw: 0x");
        UART_PrintHex8(b0);
        UART_PrintHex8(b1);
        UART_SendString("\r\n");
    }
    I2C_Stop();

    /* --- Disable --- */
    I2C_Disable();
    UART_SendString("I2C disabled\r\n");

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
