#include <stdint.h>
#include <avr/io.h>

#include "74hc165.h"


int main() {

    // Configure PWM ports as output
    DDRB = 0xFF;
    DDRD = 0xFF;

    // 74HC165 control lines as output
    DDRC = (1 << PC0) | (1 << PC1) | (1 << PC2);

    PORTB = 0;
    PORTD = 0;

    // We use the two timers for PWM outputs
    // Setup timer 0
    // Phase correct PWM, inverting output at OC0A
    // Clock divided by 8
    TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00);
    TCCR0B = (1 << CS01);

    // Setup timer 1 (16 bit timer)
    // 8 bit phase correct PWM, inverting output at OC1A
    // Clock divided by 8
    TCCR1A = (1 << COM1A1) | (1 << WGM10);
    TCCR1B = (1 << CS11);

    while(true) {
        load_data_74hc165();
        uint8_t switch1 = (PINC & (1 << PC4)) == 0 ? 1 : 0;
        uint8_t switches = read_byte_74hc165();

        uint8_t red_factor = switch1 | ((switches & 0b11) << 1);
        uint8_t red = (red_factor * 255) / 7;
        uint8_t green = (((switches & 0b11100) >> 2) * 255) / 7;
        uint8_t blue = (((switches & 0b11100000) >> 5) * 255) / 7;

        OCR0A = red;
        OCR0B = blue;
        OCR1A = green;
    }

    return 0;
}
