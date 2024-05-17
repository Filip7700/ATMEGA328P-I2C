#include <avr/io.h>

#include "atmega328p_i2c.h"
#include "shtc3.h"
#include "serialprint.h"



int main(void) {
    bool is_running = true;

    ATMega328PI2C i2c_obj;
    i2c_obj.i2c_initialize(I2C_STANDARD_MODE_FREQUENCY_HZ);

    SHTC3 shtc3_obj(i2c_obj);
    SerialPrint serialprint_obj;

    while(is_running) {
        int ret = shtc3_obj.shtc3_read_off_data();

        if(ret == SHTC3_RET_OK) {
            float temperature_celsius = shtc3_obj.get_temperature_celsius();
            float humidity = shtc3_obj.get_humidity();

            //serialprint_obj.serialprint_print_char('A');
            serialprint_obj.serialprint_print_string("Temperature: ");
            serialprint_obj.serialprint_print_float(temperature_celsius);
            serialprint_obj.serialprint_println();

            serialprint_obj.serialprint_print_string("Humidity: ");
            serialprint_obj.serialprint_print_float(humidity);
            serialprint_obj.serialprint_println();
        }
        else {
            serialprint_obj.serialprint_print_string("SHTC3 returned error value: ");
            serialprint_obj.serialprint_print_int(ret);

            if(ret == SHTC3_RET_I2C_NOT_INITIALIZED) {
                serialprint_obj.serialprint_print_string(" (I2C not initialized)");
            }
            else if(ret == SHTC3_RET_SENSOR_IRRESPONSIVE) {
                serialprint_obj.serialprint_print_string(" (SHTC3 not responding)");
            }

            serialprint_obj.serialprint_println();
        }

        _delay_ms(600U);
    }

    return 0;
}

