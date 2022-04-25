
#include <unistd.h>
#include <stdint.h>
#include "w1.h"
#include "ds2482.h"

// ----- from linux/drivers/w1/masters/ds2482.c
/**
 * The DS2482 registers - there are 3 registers that are addressed by a read
 * pointer. The read pointer is set by the last command executed.
 *
 * To read the data, issue a register read for any address
 */
#define DS2482_CMD_RESET            0xF0    /* No param */
#define DS2482_CMD_SET_READ_PTR     0xE1    /* Param: DS2482_PTR_CODE_xxx */
#define DS2482_CMD_CHANNEL_SELECT   0xC3    /* Param: Channel byte - DS2482-800 only */
#define DS2482_CMD_WRITE_CONFIG     0xD2    /* Param: Config byte */
#define DS2482_CMD_1WIRE_RESET      0xB4    /* Param: None */
#define DS2482_CMD_1WIRE_SINGLE_BIT 0x87    /* Param: Bit byte (bit7) */
#define DS2482_CMD_1WIRE_WRITE_BYTE 0xA5    /* Param: Data byte */
#define DS2482_CMD_1WIRE_READ_BYTE  0x96    /* Param: None */
/* Note to read the byte, Set the ReadPtr to Data then read (any addr) */
#define DS2482_CMD_1WIRE_TRIPLET    0x78    /* Param: Dir byte (bit7) */

/* Values for DS2482_CMD_SET_READ_PTR */
#define DS2482_PTR_CODE_STATUS      0xF0
#define DS2482_PTR_CODE_DATA        0xE1
#define DS2482_PTR_CODE_CHANNEL     0xD2    /* DS2482-800 only */
#define DS2482_PTR_CODE_CONFIG      0xC3
// -----

// timing definitions (usec)
#define DS2482_T_DELAY 1
#define DS2482_T_RSTL 630
#define DS2482_T_RSTH 614
#define DS2482_T_SLOT 73


int8_t ds2482_reset(int i2c) {
    uint8_t buffer[] = { DS2482_CMD_RESET };
    if (write(i2c, buffer, 1) != 1) return DS2482_ERR_WRITE;
    usleep(DS2482_T_DELAY);
    return 0;
}

int16_t ds2482_poll(int i2c) {
    uint8_t data;
    if (read(i2c, &data, 1) != 1) return DS2482_ERR_READ;
    return data;
}

int8_t ds2482_conf(int i2c, uint8_t conf) {
    uint8_t buffer[] = { DS2482_CMD_WRITE_CONFIG, (~conf << 4) | (conf & 0xF) };
    if (write(i2c, buffer, 2) != 2) return DS2482_ERR_WRITE;
    return 0;
}

int16_t ds2482_read_reg(int i2c, uint8_t reg) {
    uint8_t buffer[] = { DS2482_CMD_SET_READ_PTR, reg };
    if (write(i2c, buffer, 2) != 2) return DS2482_ERR_WRITE;
    if (read(i2c, buffer, 1) != 1) return DS2482_ERR_READ;
    return buffer[0];
}

int16_t ds2482_read_conf(int i2c) {
    return ds2482_read_reg(i2c, DS2482_PTR_CODE_CONFIG);
}

int16_t ds2482_read_status(int i2c) {
    return ds2482_read_reg(i2c, DS2482_PTR_CODE_STATUS);
}

int8_t ds2482_w1_reset(int i2c) {
    uint8_t buffer[] = { DS2482_CMD_1WIRE_RESET };
    if (write(i2c, buffer, 1) != 1) return DS2482_ERR_WRITE;
    usleep(DS2482_T_RSTL + DS2482_T_RSTH + DS2482_T_DELAY);
    return 0;
}

int8_t ds2482_w1_write(int i2c, uint8_t data) {
    uint8_t buffer[] = { DS2482_CMD_1WIRE_WRITE_BYTE, data };
    if (write(i2c, buffer, 2) != 2) return DS2482_ERR_WRITE;
    usleep(8 * DS2482_T_SLOT + DS2482_T_DELAY);
    return 0;
}

int8_t ds2482_w1_write_more(int i2c, uint8_t *buffer, int8_t cnt) {
    uint8_t rslt;
    while (cnt--) {
        rslt = ds2482_w1_write(i2c, *buffer++);
        if (rslt < 0) return rslt;
    }
    return 0;
}

int16_t ds2482_w1_read(int i2c) {
    uint8_t buffer[] = { DS2482_CMD_1WIRE_READ_BYTE };
    if (write(i2c, buffer, 1) != 1) return DS2482_ERR_WRITE;
    usleep(8 * DS2482_T_SLOT + DS2482_T_DELAY);
    return ds2482_read_reg(i2c, DS2482_PTR_CODE_DATA);
}

int8_t ds2482_w1_read_more(int i2c, uint8_t* buffer, int8_t cnt) {
    uint16_t data;
    while (cnt--) {
        data = ds2482_w1_read(i2c);
        if (data < 0) return data;
        *buffer++ = data;
    }
    return 0;
}

int8_t ds2482_w1_bit(int i2c, char out) {
    uint8_t buffer[] = { DS2482_CMD_1WIRE_SINGLE_BIT, out > 0 ? 0x80 : 0 };
    if (write(i2c, buffer, 2) != 2) return DS2482_ERR_WRITE;
    usleep(DS2482_T_SLOT + DS2482_T_DELAY);
    return ds2482_poll(i2c) & DS2482_REG_STS_SBR;
}

int16_t ds2482_w1_triplet(int i2c, char dir) {
    uint8_t buffer[] = { DS2482_CMD_1WIRE_TRIPLET, dir > 0 ? 0x80 : 0 };
    if (write(i2c, buffer, 2) != 2) return DS2482_ERR_WRITE;
    usleep(3 * DS2482_T_SLOT + DS2482_T_DELAY);
    return ds2482_poll(i2c);
}

int8_t ds2482_w1_search(int i2c, int8_t branch, uint8_t *last_addr, uint8_t *addr) {
    int8_t pos, discr = 0;
    int16_t stat;
    
    ds2482_w1_reset(i2c);
    // if no devices where detected during the reset/presence-pulse cycle we won't find anything
    if ((ds2482_poll(i2c) & DS2482_REG_STS_PPD) == 0) return DS2482_ERR_BUS;
    
    // put slaves in search mode and probe every bit in the ROM-code
    ds2482_w1_write(i2c, W1_CMD_SEARCH_ROM);
    for (pos = 0; pos < 64; pos++) {
        // before the last discrepancy take the same path as last time, at the discrepancy take the 1-edge, then default to 0
        stat = ds2482_w1_triplet(i2c, branch == (pos + 1) ? 1 : pos < branch && last_addr != 0 ? last_addr[pos / 8] & (1 << (pos % 8)) : 0);
        if (stat < 0) return stat;  // in case the triplet command failed
        
        if ((stat & DS2482_REG_STS_SBR) != 0) {
            // the current conjunction is 1 so the complement has to be 0 unless no slave responded at all
            if ((stat & DS2482_REG_STS_TSB) == 0) {
                addr[pos / 8] |= (1 << (pos % 8));
            } else {
                return DS2482_ERR_BUS;
            }
        } else {
            // the conjunction is 0 so if the complementary bit is 1 there is no discrepancy
            if ((stat & DS2482_REG_STS_TSB) != 0) {
                addr[pos / 8] &= ~(1 << (pos % 8));
            } else {
                // if we deliberately took the 1-edge the discrepancy is resolved otherwise we need to branch off at this point during a subsequent pass
                if ((stat & DS2482_REG_STS_DIR) != 0) {
                    addr[pos / 8] |= (1 << (pos % 8));
                } else {
                    addr[pos / 8] &= ~(1 << (pos % 8));
                    discr = pos + 1;
                }
            }
        }
    }
    
    return discr;
}
