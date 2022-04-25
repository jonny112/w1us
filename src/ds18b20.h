 
#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>

#define DS18B20_FAMILY 0x28
#define DS18B20_CONVERT_T 0x44
#define DS18B20_WRITE_SCRATCHPAD 0x4E
#define DS18B20_READ_SCRATCHPAD 0xBE
#define DS18B20_COPY_SCRATCHPAD 0x48
#define DS18B20_RECALL_E2 0xB8
#define DS18B20_READ_POWER_SUPPLY 0xB4

float ds18b20_conv_temp(uint8_t* data);

#endif
