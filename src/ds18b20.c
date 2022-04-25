
#include <stdint.h>
#include "ds18b20.h"

float ds18b20_conv_temp(uint8_t* data) {
    return (float)(int16_t)(data[0] | (data[1] << 8)) / 16;
}
