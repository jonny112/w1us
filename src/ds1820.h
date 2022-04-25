 
#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>

#define DS18B20_FAMILY 0x28
#define DS18S20_FAMILY 0x10
#define DS1820_CONVERT_T 0x44
#define DS1820_WRITE_SCRATCHPAD 0x4E
#define DS1820_READ_SCRATCHPAD 0xBE
#define DS1820_COPY_SCRATCHPAD 0x48
#define DS1820_RECALL_E2 0xB8
#define DS1820_READ_POWER_SUPPLY 0xB4

float ds18b20_conv_temp(uint8_t* data);
float ds18s20_conv_temp(uint8_t* data);

#endif
