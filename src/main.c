#include <avr/io.h>

#include "timer.h"
#include "sd.h"
#include "debug.h"

int main(void)
{
    //init
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
        ;

    return 0;
}
