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

SRC = $(shell find $(SRC_DIR) -name *.c)
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.c=.o)))

CFLAGS = -Wall -std=c99 -Os
LDFLAGS = 

all: $(OUT_DIR)/$(PROJ_NAME).hex
	@echo "done"

$(OUT_DIR)/$(PROJ_NAME).hex: $(OUT_DIR)/$(PROJ_NAME).elf
	$(OBJCOPY) -O $(FORMAT) $^ $@

$(OUT_DIR)/$(PROJ_NAME).elf: $(OBJ)
	$(LD) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/$(notdir %.c) $(shell find $(INC_DIR) -name *.h)
	$(CC) -c $< -o $@ $(CFLAGS) -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DDEBUG -I$(INC_DIR)

clean:
	rm -rf *.o
	rm $(OUT_DIR)/$(PROJ_NAME).elf
	rm $(OUT_DIR)/$(PROJ_NAME).hex
