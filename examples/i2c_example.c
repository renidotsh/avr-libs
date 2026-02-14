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
    uart_init(9600);
    sei();
    uart_send_string("I2C Demo\r\n");

    /* --- Init at 100 kHz --- */
    i2c_init(I2C_SPEED_100K);

    /* --- Bus scan --- */
    uint8_t devices[16];
    uint8_t n = i2c_scan(devices);
    uart_send_string("Scan found ");
    uart_print_u16(n);
    uart_send_string(" device(s):\r\n");
    for (uint8_t i = 0; i < n; i++) {
        uart_send_string("  0x");
        uart_print_hex8(devices[i]);
        uart_send_string("\r\n");
    }

    /* --- Low-level: Start, SLA+W, Write, Stop --- */
    i2c_status_e s;
    s = i2c_start();
    if (s == I2C_OK) {
        s = i2c_send_address_w(MPU6050_ADDR);
        if (s == I2C_OK) {
            i2c_write_byte(PWR_MGMT_REG);
            i2c_write_byte(0x00);  /* wake up */
        }
    }
    i2c_stop();

    /* --- High-level: WriteRegister --- */
    i2c_write_register(MPU6050_ADDR, PWR_MGMT_REG, 0x00);

    /* --- High-level: ReadRegister --- */
    uint8_t whoami = 0;
    s = i2c_read_register(MPU6050_ADDR, WHO_AM_I_REG, &whoami);
    uart_send_string("WHO_AM_I: 0x");
    uart_print_hex8(whoami);
    uart_send_string(" (status=");
    uart_print_u16(s);
    uart_send_string(")\r\n");

    /* --- ReadRegisters (burst 6 bytes – accel XYZ) --- */
    uint8_t accel_data[6];
    s = i2c_read_registers(MPU6050_ADDR, ACCEL_REG, accel_data, 6);
    if (s == I2C_OK) {
        int16_t ax = (int16_t)((accel_data[0] << 8) | accel_data[1]);
        int16_t ay = (int16_t)((accel_data[2] << 8) | accel_data[3]);
        int16_t az = (int16_t)((accel_data[4] << 8) | accel_data[5]);
        uart_send_string("AX="); uart_print_s16(ax);
        uart_send_string(" AY="); uart_print_s16(ay);
        uart_send_string(" AZ="); uart_print_s16(az);
        uart_send_string("\r\n");
    }

    /* --- WriteRegisters (burst write) --- */
    uint8_t config_data[] = {0x00, 0x08};  /* config + gyro config */
    i2c_write_registers(MPU6050_ADDR, 0x1A, config_data, 2);

    /* --- Low-level: Start, SLA+R, Read with ACK/NACK --- */
    s = i2c_start();
    if (s == I2C_OK) s = i2c_send_address_w(MPU6050_ADDR);
    if (s == I2C_OK) s = i2c_write_byte(ACCEL_REG);
    if (s == I2C_OK) s = i2c_rep_start();
    if (s == I2C_OK) s = i2c_send_address_r(MPU6050_ADDR);
    if (s == I2C_OK) {
        uint8_t b0, b1;
        i2c_read_byte_ack(&b0);
        i2c_read_byte_nack(&b1);
        uart_send_string("Raw: 0x");
        uart_print_hex8(b0);
        uart_print_hex8(b1);
        uart_send_string("\r\n");
    }
    i2c_stop();

    /* --- Disable --- */
    i2c_disable();
    uart_send_string("I2C disabled\r\n");

    while (1) {
        _delay_ms(1000);
    }
    return 0;
}
