
#ifndef W1_H
#define W1_H

#include <stdint.h>

#define W1_CMD_SEARCH_ROM 0xF0
#define W1_CMD_READ_ROM 0x33
#define W1_CMD_MATCH_ROM 0x55
#define W1_CMD_SKIP_ROM 0xCC
#define W1_CMD_ALARM_SEARCH 0xEC

void w1_format_romcode(uint8_t* id, char* str);
void w1_format_romcode_crc(uint8_t* id, char* str);
uint8_t w1_crc8(uint8_t crc, uint8_t *data, int cnt);

#endif
