# Makefile for C64 emulated floppy drive

PROJ_NAME = C64EFD
MCU = atmega32
F_CPU = 16000000UL
FORMAT = ihex
OUT_DIR = bin
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj

CC = avr-gcc
OBJCOPY = avr-objcopy
LD = avr-ld
AVRDUDE = avrdude
SIMULAVR = simulavr
GDB = avr-gdb

SRC = $(shell find $(SRC_DIR) -name *.c)
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.c=.o)))

CFLAGS = -Wall -pedantic -pedantic-errors -std=c99 -Os -g
LDFLAGS = -lm

all: $(OUT_DIR)/$(PROJ_NAME).hex
	@echo "done"

$(OUT_DIR)/$(PROJ_NAME).hex: $(OUT_DIR)/$(PROJ_NAME).elf
	$(OBJCOPY) -O $(FORMAT) $^ $@

$(OUT_DIR)/$(PROJ_NAME).elf: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS) -mmcu=$(MCU)

$(OBJ_DIR)/%.o: $(SRC_DIR)/$(notdir %.c) $(shell find $(INC_DIR) -name *.h)
	$(CC) -c $< -o $@ $(CFLAGS) -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DDEBUG -I$(INC_DIR)

clean:
	rm -rf $(OBJ)
	rm -rf $(OUT_DIR)/$(PROJ_NAME).elf
	rm -rf $(OUT_DIR)/$(PROJ_NAME).hex

program:
	$(AVRDUDE) -p $(MCU) -c avrispmkII -P usb -U flash:w:$(OUT_DIR)/$(PROJ_NAME).hex

sim:
	$(SIMULAVR) -g -d $(MCU) -p 1212 -c 16000000

gdb:
	$(GDB) $(OUT_DIR)/$(PROJ_NAME).elf
