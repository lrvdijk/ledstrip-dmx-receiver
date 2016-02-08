#include <stdint.h>

#ifndef DMX_RECEIVER_DMX_H
#define DMX_RECEIVER_DMX_H

enum DmxState {
    IDLE,
    BREAK,
    CHANNELS
};

void init_uart(uint16_t brr_value);

#endif //DMX_RECEIVER_DMX_H
