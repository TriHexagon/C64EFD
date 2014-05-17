all:
  avr-gcc main.c debug.c sd.c timer.c -DF_CPU=16000000UL -DDEBUG -mmcu=atmega32 -o out.bin -O0 -g -std=c99
