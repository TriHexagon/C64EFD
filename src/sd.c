#include "sd.h"

#include <avr/io.h>
#include "timer.h"

#define SD_DDR DDRB
#define SD_PORT PORTB
#define SD_PIN PINB
#define SD_PIN_POWER 0
#define SD_PIN_CS 4
#define SD_PIN_DI 5
#define SD_PIN_CLK 7

//dummy byte (high level)
#define DUMMY_BYTE 0xFF

const u8 CMD0[] = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };

static void sd_powerOn(void)
{
    //pin -> high (set bit)
    SD_PORT |= (1<<SD_PIN_POWER);
}

static void sd_powerOff(void)
{
    //pin -> low (clear bit)
    SD_PORT &= ~(1<<SD_PIN_POWER);
}

static void sd_select(void)
{
    //CS low (clear bit)
    SD_PORT &= ~(1<<SD_PIN_CS);
}

static void sd_deselect(void)
{
    //CS high (set bit)
    SD_PORT |= (1<<SD_PIN_CS);
}

void sd_sendByte(u8 data)
{
    sd_select();
    SPDR = data;
    
    //wait for complete transmition
    while ( !(SPSR & (1<<SPIF)) )
        ;

    sd_deselect();
}

u8 sd_goIdle(void)
{
    sd_select();
    
    //send CMD0
    for (u8 i=0; i<sizeof(CMD0); i++)
    {
        SPDR = CMD0[i];
        
        while ( !(SPSR & (1<<SPIF)) )
            ;
    }

    //send dummy byte (NCR wait time)
    SPDR = DUMMY_BYTE;
        
    while ( !(SPSR & (1<<SPIF)) )
            ;

    //read response; it is reqired to send dummy byte (high) to generate clk signal
    SPDR = DUMMY_BYTE;
        
    while ( !(SPSR & (1<<SPIF)) )
            ;

    sd_deselect();

    return SPDR;
}

result_t sd_init(void)
{
    //init io
    SD_DDR |= (1<<SD_PIN_POWER) | (1<<SD_PIN_CS) | (1<<SD_PIN_CLK) | (1<<SD_PIN_DI); //set pins to output
    sd_powerOff();
    sd_deselect();

    //init SPI
    //enable SPI, master mode, clk polarity CPOL=0 (rising edge -> falling edge), 64 prescaler (250 kHz at 16 MHz), clk phase CPHA=0 (sample on leading edge), data order (MSB first)
    SPCR = (1<<SPE) | (1<<MSTR) | (0<<CPOL) | (1<<SPR1);

    //power on sd card and enable SPI mode
    sd_powerOn();
    timer_delayMs(250);
    //if ( (SD_PIN & (1<<SD_PIN_DI)) == 0x00) return FAILED;
    sd_goIdle();
    
    return SUCCESS;
}
