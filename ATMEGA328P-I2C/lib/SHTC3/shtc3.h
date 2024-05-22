#pragma once

#ifndef F_CPU
#define F_CPU 16000000UL // ATMega328P CPU frequency in Hz (16 MHz)
#endif

#include <stdint.h>
#include <util/delay.h>

#include "atmega328p_i2c.h"

#define SHTC3_I2C_ADDRESS 0x70

/* SHTC3 sleep command
(in one piece and broken down into single bytes
for more convinient use with I2C). */
#define SHTC3_SLEEP_COMMAND     0xB098
#define SHTC3_SLEEP_COMMAND_MSB 0xB0
#define SHTC3_SLEEP_COMMAND_LSB 0x98

/* SHTC3 wakeup command
(in one piece and broken down into single bytes
for more convinient use with I2C). */
#define SHTC3_WAKEUP_COMMAND     0x3517
#define SHTC3_WAKEUP_COMMAND_MSB 0x35
#define SHTC3_WAKEUP_COMMAND_LSB 0x17



/* SHTC3 Measurement Commands
   ========================== */

/* There are 8 possible modes, wich SHTC3 sensor can work with.
Every mode has it unique 16-bit command code,
and every command code is defined with 3 macros.
One macro for whole 16-bit command code,
while 2 macros define command code broken down to single bytes,
for more convinient use with I2C.

Following table describes all possible working modes and their respective command codes:
+-------------------------+----------------------------------------------+----------------------------------------------+
|                         | Clock stretching enabled                     | Clock stretching disabled                    |
|                         |----------------------------------------------+----------------------------------------------+
|                         | Read temperature first | Read humidity first | Read temperature first | Read humidity first |
+-------------------------+------------------------+---------------------+------------------------+---------------------+
| High resolution         |                        |                     |                        |                     |
| (16-bit temperature and |         0x7CA2         |       0x5C24        |         0x7866         |       0x58E0        |
| humidity data)          |                        |                     |                        |                     |
+-------------------------+------------------------+---------------------+------------------------+---------------------+
| Low resolution          |                        |                     |                        |                     |
| (8-bit temperature and  |         0x6458         |       0x44DE        |         0x609C         |       0x401A        |
| humidity data)          |                        |                     |                        |                     |
+-------------------------+------------------------+---------------------+------------------------+---------------------+

NOTE:
For now the SHTC3 driver ONLY works in the mode described with 0x7CA2 command code, i.e.:
 - Uses high resolution
 - I2C clock stretching is enabled
 - Reads temperature data first, and humidity data second
(See SHTC3_MEASUREMENT_COMMAND_HIGH_RES_CLK_STRETCHING_TEMPERATURE_FIRST macro) */

#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_CLK_STRETCHING_TEMPERATURE_FIRST     0x7CA2
#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_CLK_STRETCHING_TEMPERATURE_FIRST_MSB 0x7C
#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_CLK_STRETCHING_TEMPERATURE_FIRST_LSB 0xA2

#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_CLK_STRETCHING_HUMIDITY_FIRST     0x5C24
#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_CLK_STRETCHING_HUMIDITY_FIRST_MSB 0x5C
#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_CLK_STRETCHING_HUMIDITY_FIRST_LSB 0x24

#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_TEMPERATURE_FIRST     0x7866
#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_TEMPERATURE_FIRST_MSB 0x78
#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_TEMPERATURE_FIRST_LSB 0x66

#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_HUMIDITY_FIRST     0x58E0
#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_HUMIDITY_FIRST_MSB 0x58
#define SHTC3_MEASUREMENT_COMMAND_HIGH_RES_HUMIDITY_FIRST_LSB 0xE0

#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_CLK_STRETCHING_TEMPERATURE_FIRST     0x6458
#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_CLK_STRETCHING_TEMPERATURE_FIRST_MSB 0x64
#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_CLK_STRETCHING_TEMPERATURE_FIRST_LSB 0x58

#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_CLK_STRETCHING_HUMIDITY_FIRST     0x44DE
#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_CLK_STRETCHING_HUMIDITY_FIRST_MSB 0x44
#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_CLK_STRETCHING_HUMIDITY_FIRST_LSB 0xDE

#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_TEMPERATURE_FIRST     0x609C
#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_TEMPERATURE_FIRST_MSB 0x60
#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_TEMPERATURE_FIRST_LSB 0x9C

#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_HUMIDITY_FIRST     0x401A
#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_HUMIDITY_FIRST_MSB 0x40
#define SHTC3_MEASUREMENT_COMMAND_LOW_RES_HUMIDITY_FIRST_LSB 0x1A

/* SHTC3 Measurement Commands END
   ============================== */



/* Time read from SHTC3 datasheet.
Describes typical time in microseconds wich is needed for SHTC3 sensor to
to power up and enter the idle mode.
For better use with _delay_ms() function time is also rounded up to
milliseconds. */
#define SHTC3_POWERUP_TIME_MICROSECONDS           230U
#define SHTC3_POWERUP_TIME_MILLISECONDS             2U

/* Time read from SHTC3 datasheet.
Describes typical time in microseconds wich is needed for SHTC3 sensor to
to measure temperature and humidity.
For better use with _delay_ms() function, time is also rounded up to
milliseconds. */
#define SHTC3_MEASUREMENT_DURATION_MICROSECONDS 10800U
#define SHTC3_MEASUREMENT_DURATION_MILLISECONDS    11U

// Predefined buffer sizes for reading and writing to SHTC3 sensor
#define SHTC3_MEASUREMENT_BYTES_COUNT 6U
#define SHTC3_WRITE_BUFFER_COMMAND_BYTES_COUNT 2U

// SHTC3 return values
#define SHTC3_RET_OK                   0
#define SHTC3_RET_I2C_NOT_INITIALIZED -1
#define SHTC3_RET_SENSOR_IRRESPONSIVE -2



class SHTC3 {
public:
    SHTC3(const ATMega328PI2C&);

    int shtc3_read_off_data(void);

    // Getters
    float get_temperature_celsius(void);
    float get_humidity(void);

private:
    float temperature_celsius;
    float humidity;

    ATMega328PI2C shtc3_i2c;

    int write_sleep_command(void);
    int write_wakeup_command(void);
    int write_measure_command(void);
    int read_data(uint8_t*, unsigned);
};
