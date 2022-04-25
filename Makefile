
CC = gcc
CC_OPTS = -std=gnu99 -Wall
BIN = bin
OUT = w1-therm

all: $(BIN) $(BIN)/$(OUT)

$(BIN):
	mkdir $(BIN)

$(BIN)/$(OUT): src/main.c src/w1.c src/w1.h src/ds2482.c src/ds2482.h src/ds1820.c src/ds1820.h
	$(CC) $(CC_OPTS) -o bin/w1-therm src/main.c src/w1.c src/ds2482.c src/ds1820.c

.PHONY: clean

clean:
	rm -f $(BIN)/*	
