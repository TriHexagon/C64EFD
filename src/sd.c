#include "sd.h"

#include <avr/io.h>
#include <stdlib.h>
#include "timer.h"
#include "debug.h"

#define SD_DDR DDRB
#define SD_PORT PORTB
#define SD_PIN PINB
#define SD_PIN_POWER 0
#define SD_PIN_CS 4
#define SD_PIN_DI 5
#define SD_PIN_CLK 7

//dummy byte (high level)
#define DUMMY_BYTE 0xFF

typedef union sd_Response1
{
    u8 data;
        
    struct
    {
        unsigned int inIdle : 1;
        unsigned int eraseReset : 1;
        unsigned int illegalCmd : 1;
        unsigned int crcError : 1;
        unsigned int eraseSequError : 1;
        unsigned int addressError : 1;
        unsigned int paramError : 1;
    };
} sd_Response1;

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

result_t sd_goIdle(sd_Response1* resp)
{
    u8 response[9];

    sd_select();
    
    //send CMD0
    for (u8 i=0; i<sizeof(CMD0); i++)
    {
        SPDR = CMD0[i];
        
        while ( !(SPSR & (1<<SPIF)) )
            ;
    }

    //send dummy bytes ( (1 up to 8) +1 * 8 clock cycles = NCR wait time + response byte)
    for (u8 i=0; i<9; i++)
    {
        SPDR = DUMMY_BYTE;
        
        while ( !(SPSR & (1<<SPIF)) )
                ;

        response[i] = SPDR; //save incoming data
    }

    sd_deselect();
    
    //try to get R1
    for (u8 i=0; i<sizeof(response); i++)
    {
        if (response[i] != 0xFF) //DO pin has pull-up, if one low bit -> start bit of response
        {
            //get startbit position in data byte
            u8 startPos = 0;
            for (u8 u=7; u!=255; u--) //overflow u8 -> 0-1=255
            {
                if ( (response[i] & (1<<u)) == 0x00 )
                {
                    startPos = u;
                    break;
                }
            }

            // if startbit = 7 then response[i] is the whole response byte
            if (startPos == 7)
            {
                resp->data = response[i];
                return SUCCESS;
            }

            //else: get last bits from next byte
            resp->data = (response[i]<<(7-startPos)) | (response[i+1]>>(startPos+1)); //TODO: check out of range (i+1) access
            return SUCCESS;
        }
    }
    

    return FAILED;
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
    
    sd_Response1 resp;
    if (sd_goIdle(&resp) == FAILED) //send CMD0 -> go idle state and enable SPI mode
    {
        sd_powerOff();
        return FAILED;
    }

    if (resp.inIdle)
        debug_puts("sd is in idle");
    char str[8];
    utoa(resp.data, str, 10);
    debug_puts(str);

    //sd voltage check
    
    return SUCCESS;
}
