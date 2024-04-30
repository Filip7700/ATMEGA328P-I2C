#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/cpufunc.h>

#define ATMEGA328P_CPU_FREQUENCY_HZ 16000000UL

#define I2C_STANDARD_MODE_FREQUENCY_HZ       100000UL
#define I2C_FAST_MODE_FREQUENCY_HZ           400000UL
#define I2C_FAST_MODE_PLUS_FREQUENCY_HZ     1000000UL
#define I2C_HI_SPEED_MODE_FREQUENCY_HZ      1700000UL
#define I2C_HI_SPEED_MODE_PLUS_FREQUENCY_HZ 3400000UL
#define I2C_ULTRA_FAST_MODE_FREQUENCY_HZ    5000000UL

/* Counter number for 1 ms timeout for 16 MHz ATMega328P.
ATMega328P takes 1 ms to finish counting from 0 to I2C_TIMEOUT,
thus generating 1 ms timeout. */
#define I2C_TIMEOUT 1600U

// ATMEGA328P's I2C register bit masks
#define I2C_TWS_MASK        0xF8
#define I2C_TWCR_TWEN_MASK  0x04
#define I2C_TWCR_TWSTA_MASK 0x20
#define I2C_TWCR_TWINT_MASK 0x80

// ATMega328P I2C (TWI) Status Register (TWSR) status codes
#define I2C_TWS_START            0x08
#define I2C_TWS_REPEATED_START   0x10
#define I2C_TWS_ARBITRATION_LOST 0x38

/* ATMega328P I2C (TWI) Status Register (TWSR) status codes,
for data transmission to slave device. */
#define I2C_TWS_SLAVE_ADDRESS_WRITE_ACK  0x18
#define I2C_TWS_SLAVE_ADDRESS_WRITE_NACK 0x20
#define I2C_TWS_DATA_WRITE_ACK           0x28
#define I2C_TWS_DATA_WRITE_NACK          0x30

/* ATMega328P I2C (TWI) Status Register (TWSR) status codes,
for data reception from slave. */
#define I2C_TWS_SLAVE_ADDRESS_READ_ACK  0x40
#define I2C_TWS_SLAVE_ADDRESS_READ_NACK 0x48
#define I2C_TWS_DATA_READ_ACK           0x50
#define I2C_TWS_DATA_READ_NACK          0x58

// ATMEGA328P I2C driver error codes
#define I2C_RET_OK 0
#define I2C_RET_NOT_INITIALIZED -1



void i2c_initialize(const uint32_t);
int i2c_send(const uint8_t, uint8_t const * const, const unsigned);
int i2c_receive(const uint8_t, uint8_t *const, const unsigned);

#endif /* I2C_H */
