#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "74hc165.h"
#include "dmx.h"

#define BAUD 250000
#define DMX_UBBR F_CPU/16/BAUD-1

// Four channels: master, R, G, B
#define DMX_CHANNELS 4

volatile uint16_t dmxAddress = 0;
volatile DmxState dmxState = IDLE;
volatile uint8_t dmxData[DMX_CHANNELS] = {0};


int main() {

    // Temporarily disable interrupts
    cli();

    // Configure PWM ports as output
    DDRB = (1 << PB1);
    DDRD = (1 << PD5) | (1 << PD6);

    // 74HC165 control lines as output
    DDRC = (1 << PC0) | (1 << PC1) | (1 << PC2);

    // We use the two timers for PWM outputs
    // Setup timer 0
    // Phase correct PWM, inverting output at OC0A
    // Clock divided by 256 (PWM frequency ~120 MHz)
    TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00);
    TCCR0B = (1 << CS02);

    // Setup timer 1 (16 bit timer)
    // 8 bit phase correct PWM, inverting output at OC1A
    // Clock divided by 256 (PWM frequency ~120 MHz)
    TCCR1A = (1 << COM1A1) | (1 << WGM10);
    TCCR1B = (1 << CS12);

    // Initialize UART for DMX receiver
    init_uart(DMX_UBBR);

    // Enable interrupts again
    sei();
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    uint8_t switch1 = 0;
    uint8_t switches = 0;

    load_data_74hc165();
    switch1 = (PINC & (1 << PC4)) == 0 ? 1 : 0;
    switches = read_byte_74hc165();

    dmxAddress = (switches << 1) | switch1;

    while(true) {
        // Calculate the pulse widths
        volatile uint8_t& master = dmxData[0];
        if(master != 255) {
            red = (dmxData[1] * master) / 255;
            green = (dmxData[2] * master) / 255;
            blue = (dmxData[3] * master) / 255;
        } else {
            red = dmxData[1];
            green = dmxData[2];
            blue = dmxData[3];
        }

        OCR0A = red;
        OCR0B = blue;
        OCR1A = green;
    }

    return 0;
}

ISR(USART_RX_vect) {
    // 8 bits int, use extra bool to denote first 256 or last 256 of
    // 512 possible DMX channels
    static uint16_t channelCounter = 0;

    uint8_t status = UCSR0A;
    uint8_t data = UDR0;
    uint8_t state = (uint8_t) dmxState; // Read once from SRAM

    if(status & (1 << FE0)) {
        channelCounter = 0;
        dmxState = BREAK;
        return;
    }

    // Invalid state, wait for next break
    if(status & (1 << DOR0)) {
        channelCounter = 0;
        dmxState = IDLE;
        return;
    }

    if(state == BREAK) {
        // data is now the start code, we only accept a zero start code
        if(data == 0) {
            dmxState = CHANNELS;
        } else {
            dmxState = IDLE;
        }
    } else if(state == CHANNELS) {
        if(channelCounter >= dmxAddress && channelCounter < dmxAddress+DMX_CHANNELS) {
            uint8_t index = channelCounter - dmxAddress;
            dmxData[index] = data;
        }

        if(channelCounter >= dmxAddress+DMX_CHANNELS) {
            dmxState = IDLE;
        } else {
            channelCounter++;
        }
    }
}
