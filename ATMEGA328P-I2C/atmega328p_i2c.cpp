#include "atmega328p_i2c.h"



ATMega328PI2C::ATMega328PI2C(): is_initialized(false), i2c_timeout_count(I2C_TIMEOUT_COUNTER_DEFAULT) {}



ATMega328PI2C::ATMega328PI2C(const uint32_t timeout_counter): is_initialized(false), i2c_timeout_count(timeout_counter) {}



ATMega328PI2C::ATMega328PI2C(const ATMega328PI2C &other) {
    this->is_initialized = other.is_initialized;
    this->i2c_timeout_count = other.i2c_timeout_count;
}



int ATMega328PI2C::wait_for_transmission_completion(void) {
    int ret = I2C_RET_TIMEOUT;

    uint8_t twint_value = 0U;

    /* Wait until finished transmitting data,
    i.e. TWINT bit from TWCR register is set to 1. */
    volatile unsigned i;
    for(i = 0U; i < this->i2c_timeout_count && twint_value == 0U; i++) {
        twint_value = (TWCR & I2C_TWCR_TWINT_MASK) >> TWINT;
    }

    if(twint_value == 1U) {
        ret = I2C_RET_OK;
    }

    return ret;
}



int ATMega328PI2C::wait_for_acknowledgment(const uint8_t acknowledgment_status_code) {
    int ret = I2C_RET_TIMEOUT;
    uint8_t i2c_status = TWSR & I2C_TWS_MASK;

    /* Wait for the expected acknowledgment,
    defined by acknowledgment_status_code input parameter. */
    volatile unsigned i;
    for(i = 0U; i < this->i2c_timeout_count && i2c_status != acknowledgment_status_code; i++) {
        i2c_status = TWSR & I2C_TWS_MASK;
    }

    if(i2c_status == acknowledgment_status_code) {
        ret = I2C_RET_OK;
    }

    return ret;
}



int ATMega328PI2C::start_communication(void) {
    static const uint8_t TWCR_START_CONDITION_VALUE = 164U;

    int ret = I2C_RET_TIMEOUT;

    /* In order to successfully start data transmission,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWSTA bit to send start condition (5th bit)
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is:
    0b10100100, i.e. 164 (TWCR_START_CONDITION_VALUE) */
    TWCR = TWCR_START_CONDITION_VALUE;

    ret = this->wait_for_transmission_completion();

    if(ret == I2C_RET_OK) {
        this->wait_for_acknowledgment(I2C_TWS_START);
    }

    return ret;
}



void ATMega328PI2C::stop_communication(void) {
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



int ATMega328PI2C::read_from_address(const uint8_t address) {
    static const uint8_t TWCR_INITIAL_VALUE = 132U;

    int ret = I2C_RET_TIMEOUT;

    // Put 7bit address and read bit to TWI data register (TWDR)
    TWDR = (address << 1U) | 1U;

    /* Before any I2C operation,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is:
    0b10000100, i.e. 132 (TWCR_INITIAL_VALUE) */
    TWCR = TWCR_INITIAL_VALUE;

    ret = this->wait_for_transmission_completion();

    if(ret == I2C_RET_OK) {
        ret = this->wait_for_acknowledgment(I2C_TWS_SLAVE_ADDRESS_READ_ACK);
    }

    return ret;
}



int ATMega328PI2C::write_to_address(const uint8_t address) {
    static const uint8_t TWCR_INITIAL_VALUE = 132U;
    static const uint8_t TWDR_WRITE_BIT_MASK = 254U;

    int ret = I2C_RET_TIMEOUT;

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

    ret = this->wait_for_transmission_completion();

    if(ret == I2C_RET_OK) {
        ret = this->wait_for_acknowledgment(I2C_TWS_SLAVE_ADDRESS_WRITE_ACK);
    }

    return ret;
}



int ATMega328PI2C::read_byte(uint8_t* const data) {
    static const uint8_t TWCR_MULTI_READ_INITIAL_VALUE = 196U;

    int ret = I2C_RET_TIMEOUT;

    /* In case master wants to read more than 1 byte
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWEA bit for reading multiple bytes (6th bit).
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the final TWCR register value is:
    0b11000100, i.e. 196 (TWCR_MULTI_READ_INITIAL_VALUE) */
    TWCR = TWCR_MULTI_READ_INITIAL_VALUE;

    ret = this->wait_for_transmission_completion();

    if(ret == I2C_RET_OK) {
        /* In case master wants to read multiple bytes,
        it sends ACK bit to slave device.
        In this case master expects more incoming bytes from slave. */
        ret = this->wait_for_acknowledgment(I2C_TWS_DATA_READ_ACK);

        if(ret == I2C_RET_OK) {
            /* After the reading is complete,
            new received data is in the I2C data register (TWDR). */
            *data = TWDR;
        }
    }

    return ret;
}



int ATMega328PI2C::read_last_byte(uint8_t* const data) {
    static const uint8_t TWCR_INITIAL_VALUE = 132U;

    int ret = I2C_RET_TIMEOUT;

    /* Before any I2C operation,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is: 0b10000100,
    i.e. 132 (TWCR_INITIAL_VALUE) */
    TWCR = TWCR_INITIAL_VALUE;

    ret = this->wait_for_transmission_completion();

    if(ret == I2C_RET_OK) {
        /* In case when master reads the last byte, after receiving last bit,
        it sends NACK bit to slave device.
        Therefore, the program should wait for NACK code. */
        ret = this->wait_for_acknowledgment(I2C_TWS_DATA_READ_NACK);

        if(ret == I2C_RET_OK) {
            /* After the reading is complete,
            new received data is in the I2C data register (TWDR). */
            *data = TWDR;
        }
    }

    return ret;
}



int ATMega328PI2C::write_byte(const uint8_t data) {
    static const uint8_t TWCR_INITIAL_VALUE = 132U;

    int ret = I2C_RET_TIMEOUT;

    // Put 8 bit data to TWI data register (TWDR) for writing to slave device
    TWDR = data;

    /* Before any I2C operation,
    TWI Control Register (TWCR) should have the following bits set:
    - Clear TWINT flag by writing 1 (7th bit)
    - Set TWEN bit to initialize the I2C/TWI (2nd bit).
    Therefore, the register value is:
    0b10000100, i.e. 132 (TWCR_INITIAL_VALUE) */
    TWCR = TWCR_INITIAL_VALUE;

    ret = this->wait_for_transmission_completion();

    if(ret == I2C_RET_OK) {
        ret = this->wait_for_acknowledgment(I2C_TWS_DATA_WRITE_ACK);
    }

    return ret;
}



int ATMega328PI2C::i2c_initialize(const uint32_t i2c_clock_frequency) {
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

    int ret = I2C_RET_NOT_INITIALIZED;

    if(i2c_clock_frequency == I2C_STANDARD_MODE_FREQUENCY_HZ
        || i2c_clock_frequency == I2C_FAST_MODE_FREQUENCY_HZ
        || i2c_clock_frequency == I2C_FAST_MODE_PLUS_FREQUENCY_HZ
        || i2c_clock_frequency == I2C_HI_SPEED_MODE_FREQUENCY_HZ
        || i2c_clock_frequency == I2C_HI_SPEED_MODE_PLUS_FREQUENCY_HZ
        || i2c_clock_frequency == I2C_ULTRA_FAST_MODE_FREQUENCY_HZ) {

        uint8_t twbr_value = ((ATMEGA328P_CPU_FREQUENCY_HZ / i2c_clock_frequency) - 16U) / (2U * TWPS_VALUE);
        TWBR = twbr_value;

        TWSR &= TWSR_TWPS_CLEAR_MASK;

        this->is_initialized = true;

        ret = I2C_RET_OK;
    }

    return ret;
}



int ATMega328PI2C::i2c_send(const uint8_t target_address, uint8_t const * const buffer, const unsigned buffer_size) {
    int ret = I2C_RET_NOT_INITIALIZED;

    if(this->is_initialized) {
        if(buffer == NULL || buffer_size == 0U) {
            ret = I2C_RET_INVALID_BUFFER;
        }
        else {
            ret = this->start_communication();

            if(ret == I2C_RET_OK) {
                ret = this->write_to_address(target_address);

                for(unsigned i = 0U; i < buffer_size && ret == I2C_RET_OK; i++) {
                    ret = this->write_byte(buffer[i]);
                }
            }

            this->stop_communication();
        }


    }

    return ret;
}



int ATMega328PI2C::i2c_receive(const uint8_t target_address, uint8_t* const buffer, const unsigned buffer_size) {
    int ret = I2C_RET_NOT_INITIALIZED;

    if(this->is_initialized) {
        if(buffer == NULL || buffer_size == 0U) {
            ret = I2C_RET_INVALID_BUFFER;
        }
        else {
            ret = this->start_communication();

            if(ret == I2C_RET_OK) {
                ret = this->read_from_address(target_address);

                for(unsigned i = 0U; i < buffer_size && ret == I2C_RET_OK; i++) {
                    if(i == buffer_size - 1U) {
                        ret = this->read_last_byte(&buffer[i]);
                    }
                    else {
                        ret = this->read_byte(&buffer[i]);
                    }
                }
            }

            this->stop_communication();
        }
    }

    return ret;
}
