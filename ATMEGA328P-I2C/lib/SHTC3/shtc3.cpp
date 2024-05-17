#include "shtc3.h"



SHTC3::SHTC3(const ATMega328PI2C &atmega328p_i2c): temperature_celsius(0.0F), humidity(0.0F), shtc3_i2c(atmega328p_i2c) {}



int SHTC3::write_sleep_command(void) {
    uint8_t write_buffer[SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT] = {
        SHTC3_SLEEP_COMMAND_MSB,
        SHTC3_SLEEP_COMMAND_LSB};

    int ret = this->shtc3_i2c.i2c_send(SHTC3_I2C_ADDRESS, write_buffer, SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT);

    return ret;
}



int SHTC3::write_wakeup_command(void) {
    uint8_t write_buffer[SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT] = {
        SHTC3_WAKEUP_COMMAND_MSB,
        SHTC3_WAKEUP_COMMAND_LSB};

    int ret = this->shtc3_i2c.i2c_send(SHTC3_I2C_ADDRESS, write_buffer, SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT);

    return ret;
}



int SHTC3::write_measure_command(void) {
    uint8_t write_buffer[SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT] = {
        SHTC3_MEASUREMENT_COMMAND_HIGH_RES_TEMPERATURE_FIRST_MSB,
        SHTC3_MEASUREMENT_COMMAND_HIGH_RES_TEMPERATURE_FIRST_LSB};

    int ret = this->shtc3_i2c.i2c_send(SHTC3_I2C_ADDRESS, write_buffer, SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT);

    return ret;
}



int SHTC3::read_data(uint8_t *buffer, unsigned buffer_size) {
    int ret = this->shtc3_i2c.i2c_receive(SHTC3_I2C_ADDRESS, buffer, buffer_size);

    return ret;
}



int SHTC3::shtc3_read_off_data(void) {
    /* Each measurement cycle contains a set of 4 commands,
    initiated by the I2C START condition and ended by the I2C STOP condition.
    I2C START and STOP conditions are handled by the I2C driver.
    Therefore measurement procedure consists of these 4 steps:
        1) Send wakeup command to SHTC3 slave (see SHTC3_WAKEUP_COMMAND macro)
        2) Send measurement command to SHTC3 slave
           (SHTC3 sensor can work up to 8 possible modes,
           however this basic SHTC3 driver is programmed to work in only 1
           mode for now.
           The mode works by reading temperature first, humidity second,
           in high resolution for better precision,
           i.e. 16 bit data for both temperature and humidity.
           Furthermore, the mode works with clock stretching disabled,
           and relies of waiting for predefined time,
           before requesting the read.
           See SHTC3_MEASUREMENT_COMMAND_HIGH_RES_TEMPERATURE_FIRST)
        3) Read temperature and humidity from SHTC3 slave
           (Since SHTC3 driver is programmed to work in high resolution mode,
           total of 6 bytes is required for buffer to store all the data about
           temperature and humidity.
           Bytes 0 and 1 are temperature bytes, byte 2 is temperature
           data 8-bit CRC, computed from SHTC3 sensor.
           Furthermore, bytes 3 and 4 are humidity bytes, and byte 5 is
           humidity data 8-bit CRC, also computed from SHTC3 sensor.
           More paranoid SHTC3 drivers may compute their own 8-bit CRCs
           from temperature and humidity data bytes, and compare them against
           the CRC bytes computed by SHTC3 sensor.
           However, for the sake of simplicity, this driver ignores CRC
           bytes.)
        4) Send sleep command to SHTC3 slave (see SHTC3_SLEEP_COMMAND) */

    int ret = SHTC3_RET_OK;

    // STEP 1: Send wakeup command to SHTC3 slave
    ret = this->write_wakeup_command();

    if(ret == SHTC3_RET_OK) {
        /* Give some time for SHTC3 sensor to finish powering up,
        and to enter the idle state.
        Typically, it takes 230 us for SHTC3 to power up.
        Here the time is rounded up to 1 ms.
        See SHTC3_POWERUP_TIME_MILLISECONDS
        and SHTC3_POWERUP_TIME_MICROSECONDS macros. */
        _delay_ms(SHTC3_POWERUP_TIME_MILLISECONDS);

        // STEP 2: Write measurement command to SHTC3 slave
        ret = this->write_measure_command();

        if(ret == SHTC3_RET_OK) {
            /* Give some time for SHTC3 sensor to finish measuring temperature
            and humidity (since clock stretching is disabled).
            Typically, it takes 10800us (10.8 ms) for SHTC3 to finish measuring.
            Here the time is rounded up to 11 ms.
            See SHTC3_MEASUREMENT_DURATION_MILLISECONDS
            and SHTC3_MEASUREMENT_DURATION_MICROSECONDS macros. */
            _delay_ms(SHTC3_MEASUREMENT_DURATION_MILLISECONDS);

            /* STEP 3: Read temperature and humidity from SHTC3 slave,
            and store the data in 6 byte buffer
            (see SHTC3_MEASUREMENT_BYTES_COUNT macro). */
            uint8_t measurements[SHTC3_MEASUREMENT_BYTES_COUNT] = {0U};
            ret = this->read_data(measurements, SHTC3_MEASUREMENT_BYTES_COUNT);

            if(ret == SHTC3_RET_OK) {
                /* Separate the temperature and humidity bytes in their own
                variables for easier further computation. */
                uint16_t temperature_data = (measurements[0] << 8U) | measurements[1];
                //uint8_t temperature_crc = measurements[2];
                uint16_t humidity_data = (measurements[3] << 8U) | measurements[4];
                //uint8_t humidity_crc = measurements[5];

                /* Measurement data is always transferred as 16-bit values.
                These values are already linearized and temperature
                compensated by the SHTC3.
                Humidity and temperature values can be calculated with the
                formulas given below:
                - Relative humidity conversion formula (result in percentage, %)
                    RH = 100 * Srh / 65536
                - Temperature conversion formula (result in degrees celsius):
                    T = -45 + 175 * St / 65536
                Where:
                    RH  -> humidity in percentage
                    T   -> temperature in degrees celsius
                    Srh -> raw 16-bit humidity data, read from SHTC3 sensor
                           (see humidity_data local variable)
                    St  -> raw 16-bit temperature data, read from SHTC3 sensor
                           (see temperature_data local variable) */
                this->temperature_celsius = -45.0F + (175.0F * (float)temperature_data) / 65536.0F;
                this->humidity = (100.0F * (float)humidity_data) / 65536.0F;

                // STEP 4: Put SHTC3 slave to sleep mode
                ret = this->write_sleep_command();
            }
        }
    }

    return ret;
}



float SHTC3::get_temperature_celsius(void) {
    return this->temperature_celsius;
}



float SHTC3::get_humidity(void) {
    return this->humidity;
}
