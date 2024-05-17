#include "serialprint.h"

SerialPrint::SerialPrint() {
    // Number of samples per bit for asynchronous operation in normal mode.
    static const uint32_t SAMPLES_PER_BIT = 16UL;

    uint32_t baudrate_value = F_CPU / (SAMPLES_PER_BIT * SERIALPRINT_BAUD_RATE_DEFAULT) - 1UL;

    // Set the baudrate register value
    UBRR0H = (baudrate_value & 0x0F00) >> 8U;
    UBRR0L = baudrate_value & 0x00FF;

    /* Set Frame Format so the USART uses:
        - Asynchronous mode
        - Parity disabled
        - One stop bit
        - 8 bit data selection */
    UCSR0C = 3U << UCSZ00;

    /* Only enable transmitter, receiver is not needed
    because only printing functionality is being utilized. */
    UCSR0B |= 1U << TXEN0;
}



void SerialPrint::serialprint_print_char(const char c) {
    // Do nothing until UDR is ready
    while ((UCSR0A & (1U << UDRE0)) == 0U) {
        _NOP();
    }

    UDR0 = c;
}



void SerialPrint::serialprint_print_int(const int d) {
    static const int ASCII_DIGIT_OFFSET = 48;

    int tmp = d;

    char print_buffer[10];
    uint8_t print_buffer_size = 0U;

    if(d == 0) {
        this->serialprint_print_char('0');
    }
    else {
        if(d < 0) {
            this->serialprint_print_char('-');
            tmp = -d;
        }

        while(tmp > 0 && print_buffer_size < 10) {
            print_buffer[print_buffer_size++] = (tmp % 10) + ASCII_DIGIT_OFFSET;
            tmp /= 10;
        }

        for(int i = print_buffer_size - 1; i >= 0; i--) {
            this->serialprint_print_char(print_buffer[i]);
        }
    }
}



void SerialPrint::serialprint_print_uint(const unsigned u) {
    static const unsigned ASCII_DIGIT_OFFSET = 48U;

    unsigned tmp = u;

    char print_buffer[10];
    uint8_t print_buffer_size = 0U;

    if(u == 0U) {
        this->serialprint_print_char('0');
    }
    else {
        while(tmp > 0 && print_buffer_size < 10) {
            print_buffer[print_buffer_size++] = (tmp % 10U) + ASCII_DIGIT_OFFSET;
            tmp /= 10U;
        }

        for(int i = print_buffer_size - 1; i >= 0; i--) {
            this->serialprint_print_char(print_buffer[i]);
        }
    }
}



void SerialPrint::serialprint_print_float(const float f) {
    int64_t integer_part = (int64_t)f;
    float decimal_part = f - integer_part;

    if(f < 0.0F) {
        decimal_part = -decimal_part;
    }

    this->serialprint_print_int(integer_part);
    this->serialprint_print_char('.');

    for(unsigned i = 0; i < 6U; i++) {
        decimal_part *= 10.0F;
        unsigned decimal_digit = (unsigned)decimal_part;
        this->serialprint_print_uint(decimal_digit);
        decimal_part -= (float)decimal_digit;
    }
}



void SerialPrint::serialprint_print_string(char const * const str) {
    for(unsigned i = 0U; str[i] != '\0'; i++) {
        this->serialprint_print_char(str[i]);
    }
}



void SerialPrint::serialprint_print_string(char const * const str, unsigned str_size) {
    for(unsigned i = 0U; i < str_size; i++) {
        this->serialprint_print_char(str[i]);
    }
}



void SerialPrint::serialprint_println(void) {
    this->serialprint_print_char('\n');
    this->serialprint_print_char('\r');
}
