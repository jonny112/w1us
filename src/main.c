 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <linux/i2c-dev.h>
#include "w1.h"
#include "ds2482.h"
#include "ds1820.h"

#define MAX_SLAVE_COUNT 32


int main(int argc, char** argv) {
    int i2c, addr, slave_cnt, slave_id, n;
    uint8_t buffer[10], slave_addrs[8 * MAX_SLAVE_COUNT], cksum, family;
    int16_t branch;
    char op, str_addr[20], output_mode;
    float temp;
    
    if (argc < 4) {
        fprintf(stderr, "Usage: <i2c device file> <i2c slave address> <(i)nit master|(c)ycle[(d)etect slaves|convert (t)emperature[(s)ample|(r)ead]]> [(r)rd|(j)son]\n");
        return 1;
    }
    
    op = argv[3][0];    
    addr = strtol(argv[2], NULL, 0);
    output_mode = (argc >= 5) ? argv[4][0] : 'r';
    fprintf(stderr, "%s:0x%02X\n", argv[1], addr);
    
    if ((i2c = open(argv[1], O_RDWR)) == -1) {
        perror("Could not open device");
        return 2;
    }
    
    if (flock(i2c, LOCK_EX) != 0) {
        perror("Failed to obtain lock on device");
        return 4;
    }
    
    if (ioctl(i2c, I2C_SLAVE, addr) < 0) {
        perror("Failed to set slave address");
        return 3;
    }
    
    if (op == 'i') {
        fprintf(stderr, "Resetting 1-wire master...\n");
        ds2482_reset(i2c);
        fprintf(stderr, " -> CONF: 0x%02X, STATUS: 0x%02X\n", ds2482_read_conf(i2c), ds2482_read_status(i2c));
        
        fprintf(stderr, "Configuring 1-wire master...\n");
        ds2482_conf(i2c, DS2482_REG_CFG_APU);
        fprintf(stderr, " -> CONF: 0x%02X, STATUS: 0x%02X\n", ds2482_read_conf(i2c), ds2482_read_status(i2c));
    } 
    
    if (op == 'd' || op == 'c') {
        branch = 0;
        slave_cnt = 0;
        fprintf(stderr, "Detecting slaves (%d max)...\n", MAX_SLAVE_COUNT);
        do {
            branch = ds2482_w1_search(i2c, branch, slave_cnt > 0 ? slave_addrs + (slave_cnt - 1) * 8 : 0, slave_addrs + slave_cnt * 8);
            if (branch >= 0) {
                w1_format_romcode(slave_addrs + slave_cnt * 8, str_addr);
                fprintf(stderr, "[%02d] %s (%2d)\n", slave_cnt + 1, str_addr, branch);
                slave_cnt++;
            }
        } while (branch > 0 && slave_cnt < MAX_SLAVE_COUNT);
        if (op != 'c') fwrite(slave_addrs, 8, slave_cnt, stdout);
    } 
    
    if (op == 's' || op == 't' || op == 'c') {
        fprintf(stderr, "Sampling temperature...");
        ds2482_w1_reset(i2c);
        ds2482_w1_write(i2c, W1_CMD_SKIP_ROM);
        ds2482_w1_write(i2c, DS1820_CONVERT_T);
        do {
            fprintf(stderr, ".");
            fflush(stderr);
            usleep(10000);
        } while (! ds2482_w1_bit(i2c, 1));
        fprintf(stderr, "\n");
    } 
    
    if (op == 'r' || op == 't' || op == 'c') {
        fprintf(stderr, "Reading slave data...\n");
        
        if (op != 'c') {
            slave_cnt = 0;
            while (fread(slave_addrs + slave_cnt * 8, 8, 1, stdin) == 1 && slave_cnt < MAX_SLAVE_COUNT) slave_cnt++;
        }
        
        printf(output_mode == 'r' ? "N" : "{");
        for (slave_id = 0; slave_id < slave_cnt; slave_id++) {
            w1_format_romcode_crc(slave_addrs + slave_id * 8, str_addr);
            fprintf(stderr, "[%02d] %s  ", slave_id + 1, str_addr);
            
            family = *(slave_addrs + slave_id * 8);
            if (family == DS18B20_FAMILY || family == DS18S20_FAMILY) {
                ds2482_w1_reset(i2c);
                ds2482_w1_write(i2c, W1_CMD_MATCH_ROM);
                ds2482_w1_write_more(i2c, slave_addrs + slave_id * 8, 8);
                ds2482_w1_write(i2c, DS1820_READ_SCRATCHPAD);
                ds2482_w1_read_more(i2c, buffer, 9);
                
                for (n = 0; n < 9; n++) fprintf(stderr, "%02x ", buffer[n]);
                cksum = w1_crc8(0, buffer, 9);
                temp = (family == DS18B20_FAMILY ? ds18b20_conv_temp(buffer) : ds18s20_conv_temp(buffer));
                
                fprintf(stderr, "%c  %.4f°C\n", cksum ? '!' : '*', temp);
                if (output_mode == 'r') {
                    if (cksum) printf(":U"); else printf(":%f", temp);
                } else {
                    w1_format_romcode(slave_addrs + slave_id * 8, str_addr);
                    if (cksum) printf("\"%s\":null", str_addr); else printf("\"%s\":%f", str_addr, temp);
                    if (slave_id < slave_cnt - 1) printf(",");
                }
            } else {
                fprintf(stderr, "(not supported)\n");
            }
        }
        printf(output_mode == 'r' ? "\n" : "}\n");
    }
    
    close(i2c);
    return 0;
}
