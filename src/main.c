#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "timer.h"
#include "sd.h"
#include "debug.h"

u8 buffer[512];

int main(void)
{
    //init
    sei();
    timer_init();
    debug_init();
    timer_delayMs(100); //waiting for safety

    while(1)
    {
        if (sd_init() == SUCCESS)
            break;
        timer_delayMs(1000);
    }
    
    debug_puts("sd is detected!");
    
    if (sd_readBlock(buffer, 512, 0) == SUCCESS)
    {
		debug_puts("read block success!");
		for (size_t i=0; i<sizeof(buffer); i++)
			debug_putc(buffer[i]);
			
		debug_puts("\0\0");
	}

    while (1)
	{
	}

    return 0;
}
