#include <avr/cpufunc.h>
#include "74hc165.h"

void load_data_74hc165() {
    // Toggle parallel load pin
    // Active low, so pull down first, then high again
    PORTC &= ~(1 << PC0);
    _NOP();
    _NOP();
    PORTC |= (1 << PC0);
}

uint8_t read_byte_74hc165() {
    // Clear CE, it is active low
    PORTC &= ~(1 << PC2);

    uint8_t result = 0;
    for(uint8_t i = 0; i < 8; i++) {

        uint8_t value = (PINC & (1 << PC3)) != 0 ? 1 : 0;
        result |= value << i;

        // Pulse clock
        PORTC |= (1 << PC1);
        PORTC &= ~(1 << PC1);
    }

    PORTC |= (1 << PC2);

    return result;
}

