# w1us
User-space utility for interacting with 1-Wire devices through a DS2482 1-Wire bridge via i2c-dev.

This is intended to be used with SOCs like Raspberry Pi or ODROID with I2C ports accessible via i2c-dev device files.

A DS2482 will be at I2C address `0x18`-`0x1B` depending on the AD0/AD1 pin setting, see datasheet.
For detecting connected devices you may use `i2cdetect -y <port>`. Where `port` is the number of the I2C interface corresponding to `/dev/i2c-<port>`.

### w1-term
Interface with temperature sensors of the DS18B20 and DS18S20 family.

#### Usage
```
w1-therm <i2c device file> <i2c slave address> <(i)nit master|(c)ycle[(d)etect slaves|convert (t)emperature[(s)ample|(r)ead]]> [(r)rd|(j)son]
```
The device file will be locked for exclusive access. Another instance ran at the same time will block until the first releases the device.

#### Process options
 i | c | d | t | s | r | Step
---|---|---|---|---|---|-
(+)|(-)|(-)|(-)|(-)|(-)| Initialization of the 1-Wire bridge/master. Should be performed once after power-on.
(-)|(+)|(-)|(+)|(+)|(-)| Request all sensors on the bus to sample temperature and wait (polling the bus status) for this to complete. This is a broadcast (skip ROM) operation that does not require individual devices to be known.
(-)|(+)|(+)|(-)|(-)|(-)| Detect individual 1-Wire devices/slaves and retrieve their addresses. If this is the only operation performed, the detected slave addresses are written to `stdout` as 8-byte blocks.
(-)|(+)|(-)|(+)|(-)|(+)| Read the last sampled temperature value from each sensors. If no slave detection is performed in the same run, addresses are read from `stdin` as 8-byte blocks.

By default temperature values are output to `stdout` in RRD format, with values in the order slave addresses were specified/detected, or alternatively as JSON object mapping address-strings to numbers.
A `*` or `!` in the output means good or bad checksum respectively, for addresses and data.
Values for addresses from which could not be successfully read are output as `U` for RRD and `null` for JSON.

#### Examples
Detect connected sensors and save addresses to file:
```sh
w1-therm /dev/i2c-1 0x18 d > sensors.w1
```
```
/dev/i2c-1:0x18
Detecting slaves (32 max)...
[01] 10.00xxxxxxxx29 ( 4)
[02] 28.03xxxxxxxx64 (10)
[03] 28.03xxxxxxxxfe ( 9)
[04] 28.03xxxxxxxxeb (11)
[05] 28.03xxxxxxxxff ( 0)
```
Sample temperature, read values from previously detected addresses and output as JSON:
```
w1-therm /dev/i2c-1 0x18 t j < sensors.w1
```
```
/dev/i2c-1:0x18
Sampling temperature........................................................
Reading slave data...
[01] 10.00xxxxxxxx29+86*  2c 00 4b 46 ff ff 09 10 79 *  22.0000°C
[02] 28.03xxxxxxxx64+ba*  0c 02 55 05 7f a5 a5 66 0a *  32.7500°C
[03] 28.03xxxxxxxxfe+4a*  3d 02 55 05 7f a5 81 66 4f *  35.8125°C
[04] 28.03xxxxxxxxeb+bc*  b8 01 55 05 7f a5 a5 66 36 *  27.5000°C
[05] 28.03xxxxxxxxff+cb*  bd 03 55 05 7f a5 a5 66 1a *  59.8125°C
{"10.00xxxxxxxx29":22.000000,"28.03xxxxxxxx64":32.750000,"28.03xxxxxxxxfe":35.812500,"28.03xxxxxxxxeb":27.500000,"28.03xxxxxxxxff":59.812500}
```
Only the last line is `stdout`, other output is `stderr`.
