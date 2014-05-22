#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "timer.h"
#include "sd.h"
#include "debug.h"

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

    while (1)
	{
		debug_puts("sd is detected!");
        timer_delayMs(2000);
	}

    return 0;
}
