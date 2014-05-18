#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"
#include "sd.h"
#include "debug.h"

int main(void)
{
	DDRB = 0xFF;
	PORTB = 0x00;
	
    //init
    sei();
    timer_init();
    debug_init();
    timer_delayMs(100); //waiting for safety

    debug_puts("Ready!");

    while(1)
    {
        if (sd_init() == SUCCESS)
            break;
    }

    while (1)
	{
		PORTB = ~PORTB;
		timer_delayMs(2000);
		debug_puts("2 seconds");
	}

    return 0;
}
