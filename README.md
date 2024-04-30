# ATMEGA328P-I2C

This is a simple (Arduino) library to simplify sending multi-byte data over
I2C (or TWI in Atmel's terminology).
This library is written specifically for AVR architecture, and it is
primarily meant to be used for Arduino boards, which use Atmel's ATMega328P
microcontroller.
The library features basic I2C operations for microcontroller as master
device, such as initializing I2C, writing and rading multiple bytes to/from
slave devices.



## Installation

This project can be used as library for Arduino.
In order to install library for Arduino IDE, please go to one of the
following branches and download the project as a .zip file:
- `i2c-arduino-lib`
- `i2c-example-shtc3-arduino-lib`

For best ease of use, the branch `i2c-example-shtc3-arduino-lib` is
recomended, because except the I2C driveer itself, it includes working SHTC3
driver, written to test I2C driver functionality.
It includes an example source codes as well.
The branch `i2c-arduino-lib` only includes I2C driver itself.
However it can be used for writing drivers for other devices and sensors,
which use I2C protocol (e.g. LSM6DSO 6-DOF module or RTC PCF85063A).

### Installing the downloaded .zip library

1. Open the Arduino IDE
2. On the menu bar click on the "Sketch" dropdown menu
3. Include Library > Add .ZIP Library
4. Navigate to the location where the project .zip file is downloaded
5. Select and open the .zip file



## Using the ATMEGA328P-I2C library

The library can be included into Arduino sketch, using the following line:
`#include <atmega328p_i2c.h>`

After including the library, the following functions can be called to utilize
functionalities of I2C communication:
1. `void i2c_initialize(const uint32_t)`
2. `int i2c_send(const uint8_t, uint8_t const * const, const unsigned)`
3. `int i2c_receive(const uint8_t, uint8_t *const, const unsigned)`

### Function i2c_initialize

Initializes the I2C clock and registers, i.e. I2C bit rate register (TWBR)
and I2C status register (TWSR).
The funciton takes one input parameter, which is desired I2C clock frequency.
One of the following frequencies must be passed to function:
- `I2C_STANDARD_MODE_FREQUENCY_HZ` (or 100000)
- `I2C_FAST_MODE_FREQUENCY_HZ` (or 400000)
- `I2C_FAST_MODE_PLUS_FREQUENCY_HZ` (or 1000000)
- `I2C_HI_SPEED_MODE_FREQUENCY_HZ` (or 1700000)
- `I2C_HI_SPEED_MODE_PLUS_FREQUENCY_HZ` (or 3400000)
- `I2C_ULTRA_FAST_MODE_FREQUENCY_HZ` (or 5000000)

This function must be called first, before doing anything else with I2C.
Therefore, it is reasonable to call this function within `void setup()`
function.

### Function i2c_send

Writes array of bytes of arbitrary size to slave device.
The function accepts three parameters:
- slave's 7-bit I2C address
- byte buffer, which will be written to target slave device
- buffer size, i.e. number of bytes in buffer, which will be written to target slave device

### Function i2c_receive

Reads bytes of slave device, and stores it in byte buffer.
The function accepts three parameters:
- slave's 7-bit I2C address
- byte buffer, which will store read bytes from the slave
- buffer size, i.e. expected number of bytes in buffer, which will store data from slave
