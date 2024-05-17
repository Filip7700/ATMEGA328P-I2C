#pragma once

#ifndef F_CPU
#define F_CPU 16000000UL // ATMega328P CPU frequency in Hz (16 MHz)
#endif

#include <stdint.h>
#include <avr/io.h>
#include <avr/cpufunc.h>

#define SERIALPRINT_BAUD_RATE_DEFAULT 9600UL

class SerialPrint {
public:
    SerialPrint();

    void serialprint_print_char(const char);
    void serialprint_print_int(const int);
    void serialprint_print_uint(const unsigned);
    void serialprint_print_float(const float);
    void serialprint_print_string(char const * const);
    void serialprint_print_string(char const * const, unsigned);
    void serialprint_println(void);
};
