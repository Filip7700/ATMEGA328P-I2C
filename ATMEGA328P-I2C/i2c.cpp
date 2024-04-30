#include "i2c.h"



static bool is_initialized = false;



static void i2c_start(void) {
    static const uint8_t TWCR_START_CONDITION_VALUE = 164U;

    /* In order to successfully start data transmission,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWSTA bit to send start condition (5th bit)
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is:
    0b10100100, i.e. 164 (TWCR_START_CONDITION_VALUE) */
    TWCR = TWCR_START_CONDITION_VALUE;

    /* Wait until start condition is transmitted,
    i.e. TWINT bit from TWCR register is set to 1. */
    while(((TWCR & I2C_TWCR_TWINT_MASK) >> TWINT) == 0U) {
        _NOP();
    }

    // Wait for the acknowledgment
    while((TWSR & I2C_TWS_MASK) != I2C_TWS_START) {
        _NOP();
    }
}



static void i2c_stop(void) {
    static const uint8_t TWCR_STOP_CONDITION_VALUE = 148U;
    static const uint8_t TWCR_TWEN_CLEAR_MASK = 251U;

    /* In order to successfully stop data transmission,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit),
    - Set TWSTO bit to send STOP condition on SDA and SCL line (4th bit),
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is:
    0b10010100, i.e. 148 (TWCR_STOP_CONDITION_VALUE) */
    TWCR = TWCR_STOP_CONDITION_VALUE;

    /* Clear TWEN bit from control register (TWCR),
    to stop using I2C.
    This bit is set by clearing 2th bit, and leaving the rest of bits as is.
    This can be done by using a following bit mask:
    0b11111011, i.e. 251 (TWCR_TWEN_CLEAR_MASK) */
    TWCR &= TWCR_TWEN_CLEAR_MASK;
}



static void i2c_read_from_address(const uint8_t address) {
    static const uint8_t TWCR_INITIAL_VALUE = 132U;

    // Put 7bit address and read bit to TWI data register (TWDR)
    TWDR = (address << 1U) | 1U;

    /* Before any I2C operation,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is:
    0b10000100, i.e. 132 (TWCR_INITIAL_VALUE) */
    TWCR = TWCR_INITIAL_VALUE;

    /* Wait until finished transmitting data from TWDR,
    i.e. TWINT bit from TWCR register is set to 1. */
    while(((TWCR & I2C_TWCR_TWINT_MASK) >> TWINT) == 0U) {
        _NOP();
    }

    // Wait for the acknowledgment
    while((TWSR & I2C_TWS_MASK) != I2C_TWS_SLAVE_ADDRESS_READ_ACK) {
        _NOP();
    }
}



static void i2c_write_to_address(const uint8_t address) {
    static const uint8_t TWCR_INITIAL_VALUE = 132U;
    static const uint8_t TWDR_WRITE_BIT_MASK = 254U;

    /* Put 7 bit address and write bit to TWI data register (TWDR)
    Write bit is set by clearing 0th bit, and leaving the rest of bits as is.
    This can be done by using a following bit mask:
    0b11111110, i.e. 254 (TWDR_WRITE_BIT_MASK) */
    uint8_t twdr_bits = (address << 1U) & TWDR_WRITE_BIT_MASK;
    TWDR = twdr_bits;

    /* Before any I2C operation,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is:
    0b10000100, i.e. 132 (TWCR_INITIAL_VALUE) */
    TWCR = TWCR_INITIAL_VALUE;

    /* Wait until finished transmitting data from TWDR,
    i.e. TWINT bit from TWCR register is set to 1. */
    while(((TWCR & I2C_TWCR_TWINT_MASK) >> TWINT) == 0U) {
        _NOP();
    }

    // Wait for the acknowledgment
    while((TWSR & I2C_TWS_MASK) != I2C_TWS_SLAVE_ADDRESS_WRITE_ACK) {
        _NOP();
    }
}



static uint8_t i2c_read_data(const bool is_last_byte) {
    static const uint8_t TWCR_INITIAL_VALUE = 132U;
    static const uint8_t TWCR_MULTI_READ_INITIAL_VALUE = 196U;

    uint8_t read_data = 0U;

    if(is_last_byte) {
        /* Before any I2C operation,
        TWI Control Register (TWCR) should have the following bits set:
        - Clear TWINT flag by writing 1 (7th bit)
        - Set TWEN bit to initialize the I2C/TWI (2nd bit).
        Therefore, the register value is: 0b10000100,
        i.e. 132 (TWCR_INITIAL_VALUE) */
        TWCR = TWCR_INITIAL_VALUE;
    }
    else {
        /* In case master wants to read more than 1 byte (is_last_byte is false)
        additional bi in TWCR should be set.
        - Set TWEA bit for reading multiple bytes (6th bit).
        Therefore, the final TWCR register value is:
        0b11000100, i.e. 196 (TWCR_MULTI_READ_INITIAL_VALUE) */
        TWCR = TWCR_MULTI_READ_INITIAL_VALUE;
    }

    /* Wait until finished transmitting data from TWDR,
    i.e. TWINT bit from TWCR register is set to 1. */
    while(((TWCR & I2C_TWCR_TWINT_MASK) >> TWINT) == 0U) {
        _NOP();
    }

    if(is_last_byte) {
        /* In case when master reads the last byte, after receiving last bit,
        it sends NACK bit to slave device.
        Therefore, the program should wait for NACK code. */
        while((TWSR & I2C_TWS_MASK) != I2C_TWS_DATA_READ_NACK) {
            _NOP();
        }
    }
    else {
        /* Otherwise, wait for the acknowledgment, as usual.
        In this case master expects more incoming bytes from slave. */
        while((TWSR & I2C_TWS_MASK) != I2C_TWS_DATA_READ_ACK) {
            _NOP();
        }
    }

    /* After the reading is complete,
    new received data is in the I2C data register (TWDR). */
    read_data = TWDR;

    return read_data;
}



static void i2c_write_data(const uint8_t data) {
    static const uint8_t TWCR_INITIAL_VALUE = 132U;

    // Put 8 bit data to TWI data register (TWDR) for writing to slave device
    TWDR = data;

    /* Before any I2C operation,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is:
    0b10000100, i.e. 132 (TWCR_INITIAL_VALUE) */
    TWCR = TWCR_INITIAL_VALUE;

    /* Wait until finished transmitting data from TWDR,
    i.e. TWINT bit from TWCR register is set to 1. */
    while(((TWCR & I2C_TWCR_TWINT_MASK) >> TWINT) == 0U) {
        _NOP();
    }

    // Wait for the acknowledgment
    while((TWSR & I2C_TWS_MASK) != I2C_TWS_DATA_WRITE_ACK) {
        _NOP();
    }
}



void i2c_initialize(const uint32_t i2c_clock_frequency) {
    /* i2c_clock_frequency = ATMEGA328P_CPU_FREQUENCY_HZ / (16 + 2 * TWBR * TWPS_VALUE)
    TWPS: I2C (TWI) Prescaler bits
    TWPS -> TWPS_VALUE
    ------------------
    00   -> 1
    01   -> 2
    10   -> 16
    11   -> 64
    Here, prescaler value is chosen to be 1, therefore bits should be set to 00.
    Since desired I2C clock frequency (i2c_clock_frequency) is known,
    value TWBR register should be calculated.
    Here is the following formula:
    TWBR = ((ATMEGA328P_CPU_FREQUENCY_HZ / i2c_clock_frequency) - 16) / (2 * TWPS_VALUE) */

    static const uint8_t TWPS_VALUE = 1U;

    /* Clear 0th and 1st bit from I2C status register.
    Those 2 bits are TWPS bits, and they both should be 0,
    so the TWPS_VALUE is 1.
    The clear mask is 0b11111100, i.e. 252 */
    static const uint8_t TWSR_TWPS_CLEAR_MASK = 252U;

    uint8_t twbr_value = ((ATMEGA328P_CPU_FREQUENCY_HZ / i2c_clock_frequency) - 16U) / (2U * TWPS_VALUE);
    TWBR = twbr_value;

    TWSR &= TWSR_TWPS_CLEAR_MASK;

    is_initialized = true;
}



int i2c_send(const uint8_t target_address, uint8_t const * const buffer, const unsigned buffer_size) {
    int ret = I2C_RET_OK;

    if(is_initialized) {
        i2c_start();
        i2c_write_to_address(target_address);

        unsigned i;
        for(i = 0U; i < buffer_size; i++) {
            i2c_write_data(buffer[i]);
        }

        i2c_stop();
    }
    else {
        ret = I2C_RET_NOT_INITIALIZED;
    }

    return ret;
}



int i2c_receive(const uint8_t target_address, uint8_t *const buffer, const unsigned buffer_size) {
    int ret = I2C_RET_OK;

    if(is_initialized) {
        i2c_start();
        i2c_read_from_address(target_address);

        unsigned i;
        for(i = 0U; i < buffer_size; i++) {
            bool is_last_byte = false;

            if(i == buffer_size - 1U) {
                is_last_byte = true;
            }

            buffer[i] = i2c_read_data(is_last_byte);
        }

        i2c_stop();
    }
    else {
        ret = I2C_RET_NOT_INITIALIZED;
    }

    return ret;
}
