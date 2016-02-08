#include <avr/io.h>
#include "dmx.h"

void init_uart(uint16_t brr_value) {
    UBRR0H = (uint8_t) (brr_value >> 8);
    UBRR0L = (uint8_t) brr_value;

    // Enable receiver and receive complete interrupt
    UCSR0B = (1 << RXEN0) | (1 << RXCIE0);

    // 8 bit data, 2 stop bits, no parity bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (1 << USBS0);
}

