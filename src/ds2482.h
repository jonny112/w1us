
#ifndef DS2482_H
#define DS2482_H

#include <stdint.h>

// ----- from linux/drivers/w1/masters/ds2482.c
/**
 * Configure Register bit definitions
 * The top 4 bits always read 0.
 * To write, the top nibble must be the 1's compl. of the low nibble.
 */
#define DS2482_REG_CFG_1WS      0x08    /* 1-wire speed */
#define DS2482_REG_CFG_SPU      0x04    /* strong pull-up */
#define DS2482_REG_CFG_PPM      0x02    /* presence pulse masking */
#define DS2482_REG_CFG_APU      0x01    /* active pull-up */

/**
 * Status Register bit definitions (read only)
 */
#define DS2482_REG_STS_DIR      0x80
#define DS2482_REG_STS_TSB      0x40
#define DS2482_REG_STS_SBR      0x20
#define DS2482_REG_STS_RST      0x10
#define DS2482_REG_STS_LL       0x08
#define DS2482_REG_STS_SD       0x04
#define DS2482_REG_STS_PPD      0x02
#define DS2482_REG_STS_1WB      0x01
// -----

// errors 
#define DS2482_ERR_WRITE -1
#define DS2482_ERR_READ -2
#define DS2482_ERR_BUS -3

int8_t ds2482_reset(int i2c);
int16_t ds2482_poll(int i2c);
int8_t ds2482_conf(int i2c, uint8_t conf);
int16_t ds2482_read_conf(int i2c);
int16_t ds2482_read_status(int i2c);
int8_t ds2482_w1_reset(int i2c);
int8_t ds2482_w1_write(int i2c, uint8_t data);
int8_t ds2482_w1_write_more(int i2c, uint8_t* buffer, int8_t cnt);
int16_t ds2482_w1_read(int i2c);
int8_t ds2482_w1_read_more(int i2c, uint8_t* buffer, int8_t cnt);
int8_t ds2482_w1_bit(int i2c, char out);
int16_t ds2482_w1_triplet(int i2c, char dir);
int8_t ds2482_w1_search(int i2c, int8_t branch, uint8_t *last_addr, uint8_t *addr);

#endif
