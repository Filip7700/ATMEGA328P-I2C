#include "shtc3.h"



void shtc3_initialize(void) {
    i2c_initialize(I2C_STANDARD_MODE_FREQUENCY_HZ);
}



static int shtc3_write_sleep_command(void) {
    uint8_t write_buffer[SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT] = {
        SHTC3_SLEEP_COMMAND_MSB,
        SHTC3_SLEEP_COMMAND_LSB};

    int ret = i2c_send(SHTC3_I2C_ADDRESS, write_buffer, SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT);

    return ret;
}



static int shtc3_write_wakeup_command(void) {
    uint8_t write_buffer[SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT] = {
        SHTC3_WAKEUP_COMMAND_MSB,
        SHTC3_WAKEUP_COMMAND_LSB};

    int ret = i2c_send(SHTC3_I2C_ADDRESS, write_buffer, SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT);

    return ret;
}



static int shtc3_write_measure_command(void) {
    uint8_t write_buffer[SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT] = {
        SHTC3_MEASUREMENT_COMMAND_HIGH_RES_TEMPERATURE_FIRST_MSB,
        SHTC3_MEASUREMENT_COMMAND_HIGH_RES_TEMPERATURE_FIRST_LSB};

    int ret = i2c_send(SHTC3_I2C_ADDRESS, write_buffer, SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT);

    return ret;
}



static int shtc3_read_data(uint8_t *buffer, unsigned buffer_size) {
    int ret = i2c_receive(SHTC3_I2C_ADDRESS, buffer, buffer_size);

    return ret;
}



int shtc3_get_temperature_and_humidity(float *temperature_celsius, float *humidity) {
    int ret;

    ret = shtc3_write_wakeup_command();

    if(ret == SHTC3_RET_OK) {
        _delay_ms(SHTC3_POWERUP_TIME_MILLISECONDS);

        ret = shtc3_write_measure_command();

        if(ret == SHTC3_RET_OK) {
            _delay_ms(SHTC3_MEASUREMENT_DURATION_MILLISECONDS);

            uint8_t measurements[SHTC3_MEASUREMENT_BYTES_COUNT] = {0U};
            ret = shtc3_read_data(measurements, SHTC3_MEASUREMENT_BYTES_COUNT);

            if(ret == SHTC3_RET_OK) {
                uint16_t temperature_data = (measurements[0] << 8U) | measurements[1];
                //uint8_t temperature_crc = measurements[2];
                uint16_t humidity_data = (measurements[3] << 8U) | measurements[4];
                //uint8_t humidity_crc = measurements[5];

                *temperature_celsius = -45.0F + (175.0F * (float)temperature_data) / 65536.0F;
                *humidity = (100.0F * (float)humidity_data) / 65536.0F;

                ret = shtc3_write_sleep_command();
            }
        }
    }

    return ret;
}
