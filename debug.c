#include "debug.h"

#ifdef DEBUG

#define BAUD 9600

#include "types.h"
#include <avr/io.h>
#include <util/setbaud.h>

void debug_init(void)
{
    //set baudrate
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;

    UCSRB = (1<<TXEN); // enable transmition mode
    UCSRC = (1<<URSEL) | (3<<UCSZ0); //set frame format: 8 data, 1 stopbit
}

void debug_putc(unsigned char c)
{
    //wait for empty transmition buffer
    while ( !(UCSRA & (1<<UDRE)) )
        ;

    UDR = c;
}

void debug_putData(const void* data, size_t size)
{
    const char* ptr = data;

    for (size_t i=0;i<size;i++)
    {
        debug_putc(*ptr)
        ptr++;
    }
}

void debug_puts(const char* str)
{
    const char* ptr = str;

    while (*ptr != '\0')
    {
        debug_putc(*ptr);
        ptr++;
    }

    //required to send for terminal
    debug_putc(10);
    debug_putc(13);
}

#endif //DEBUG
