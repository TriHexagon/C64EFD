#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile bool msIsElapsed;

static void timer_start(void)
{
    TCNT0 = 0; //reset counter
    TCCR0 |= (1<<CS01) | (1<<CS00); //64 prescaler, start timer
}

static void timer_stop(void)
{
    TCCR0 &= ~((1<<CS02) | (1<<CS01) | (1<<CS00)); //clear prescaler -> timer stop
}

void timer_init(void)
{
    msIsElapsed = false;

    //init timer0
    TCCR0 = (1<<WGM01); //ctc-mode
    OCR0 = 250-1; // (16 MHz / 64) / 250 = 1 kHz
    TIMSK |= (1<<OCIE0); //enable timer0 output compare match interrupt
}

//timer 0 compare match interrupt handler
ISR(TIMER0_COMP_vect)
{
    msIsElapsed = true;
}

void timer_delayMs(u16 ms)
{
    timer_start();

    for (u16 i=0;i<ms;i++)
    {
        msIsElapsed = false;

        while (msIsElapsed == false)
            ;
    }

    timer_stop();
}
