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
//wait time in bytes
#define NCR_MAX_WAIT_TIME 8

//SD Registers
typedef union sd_OCR
{
	u8 data[4]; //32 bit width
	
	struct
	{
		unsigned int reserved0 : 15;
		unsigned int v27_36 : 9; //2.7 - 3.6 Volt
		unsigned int reserved1 : 6;
		unsigned int capacityStatus : 1; //CCS (only valid when power up status bit is set)
		unsigned int powerUpStatus : 1; //bit is set to low if card has not finished power up routine (busy)
	};
} sd_OCR;

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
        unsigned int startBit : 1;
    };
} sd_Response1;

typedef union sd_Response1_DataToken
{
    u8 data[2];
        
    struct
    {
        unsigned int inIdle : 1;
        unsigned int eraseReset : 1;
        unsigned int illegalCmd : 1;
        unsigned int crcError : 1;
        unsigned int eraseSequError : 1;
        unsigned int addressError : 1;
        unsigned int paramError : 1;
        unsigned int startBit : 1;
        
        //DataToken
        unsigned int b0 : 1;
        unsigned int status : 3;
        unsigned int b4 : 1;
        unsigned int x : 3;
    };
} sd_Response1_DataToken;

typedef union sd_Response3
{
	u8 data[5]; //40 bit width
	
	struct
	{
		sd_OCR ocr;
		sd_Response1 r1;
	};
} sd_Response3;

const u8 CMD0[]   = { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
const u8 CMD55[]  = { 0x77, 0x00, 0x00, 0x00, 0x00, 0x01 };
const u8 ACMD41[] = { 0x69, 0x00, 0x00, 0x00, 0x00, 0x01 };
const u8 CMD16[]  = { 0x50, 0x00, 0x00, 0x02, 0x00, 0x01 };
const u8 CMD17_BEGIN[] = { 0x51 };
const u8 CMD17_END[] = { 0x01 };

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

u8 spi_receiveByte(void)
{
	SPDR = DUMMY_BYTE;
        
    while ( !(SPSR & (1<<SPIF)) )
        ;
    
    return SPDR;
}

void spi_sendData(const void* inData, size_t size)
{
	const u8* data = inData;
	for (size_t i=0; i<size; i++)
    {
        SPDR = data[i];
        
        while ( !(SPSR & (1<<SPIF)) )
            ;
    }
}

result_t sd_getResponse(void* outData, size_t size)
{
	u8* data = outData;
	
	u8 dataByte;
	u8 nextByte;
	for (u8 i=0; i<NCR_MAX_WAIT_TIME+1; i++)
	{
		SPDR = DUMMY_BYTE; //send dummy byte
		while ( !(SPSR & (1<<SPIF)) ) //wait for communication end
			;
		
		dataByte = SPDR;
		
		if (dataByte != 0xFF) //response begin
		{
			//get next data byte
			SPDR = DUMMY_BYTE; //send dummy byte
			
			//get start bit position
			u8 startPos = 0;
            for (u8 u=7; u!=255; u--) //overflow u8 -> 0-1=255
            {
                if ( (dataByte & (1<<u)) == 0x00 )
                {
                    startPos = u;
                    break;
                }
            }
			
			while ( !(SPSR & (1<<SPIF)) ) //wait for communication end
				;
		
			nextByte = SPDR;
			
			if (startPos == 7) //bit communication fits byte communication
			{
				data[size-1] = dataByte;
				if (size == 1)
					return SUCCESS;
					
				data[size-2] = nextByte;
				if (size == 2)
					return SUCCESS;
				
				//simple byte read in
				for (u8 u=size-3; u!=255; u--)
				{
					SPDR = DUMMY_BYTE; //send dummy byte
					while ( !(SPSR & (1<<SPIF)) ) //wait for communication end
						;
		
					data[u] = SPDR;
				}
				
				return SUCCESS;
			}
			
			//assemble both data bytes
			data[size-1] = (dataByte<<(7-startPos)) | (nextByte>>(startPos+1));
			//get last bytes and assemble
			for (u8 u=size-2; u!=255; u--)
			{
				dataByte = nextByte;
				
				//get next byte
				SPDR = DUMMY_BYTE; //send dummy byte
				while ( !(SPSR & (1<<SPIF)) ) //wait for communication end
					;
		
				nextByte = SPDR;
				
				//assemble
				data[u] = (dataByte<<(7-startPos)) | (nextByte>>(startPos+1));
			}
			
			return SUCCESS;
		}
	}
	
	return FAILED;
}

result_t sd_goIdle(sd_Response1* resp)
{
    sd_select();
    
    //send CMD0
    spi_sendData(CMD0, sizeof(CMD0));

    result_t res = sd_getResponse(&resp->data, sizeof(resp->data));
    sd_deselect();
    return res;
}

result_t sd_getVoltage(sd_Response3* resp)
{
	sd_Response1 response1;
	
	sd_select();
	
	//send CMD55
	spi_sendData(CMD55, sizeof(CMD55));
	
	result_t res = sd_getResponse(&response1.data, sizeof(response1.data));
	sd_deselect();
	
	if (res == FAILED)
		return FAILED;
		
	SPDR = DUMMY_BYTE; //send dummy byte
	while ( !(SPSR & (1<<SPIF)) ) //wait for communication end
		;
	
	sd_select();
	
	//send ACMD41
	spi_sendData(ACMD41, sizeof(ACMD41));
	res = sd_getResponse(resp->data, sizeof(resp->data));
	sd_deselect();
	
	return res;
}

result_t sd_setBlockSize(sd_Response1* resp)
{
	sd_select();
	
	//send CMD16 with block size of 512 bytes
	spi_sendData(CMD16, sizeof(CMD16));
	result_t res = sd_getResponse(resp, sizeof(resp));
	sd_deselect();
	
	return res;
}

result_t sd_readBlock(void* buffer, size_t size, u32 address)
{
	u8* buf = buffer;
	sd_Response1_DataToken resp;
	//TODO: check arguments
	
	sd_select();
	spi_sendData(CMD17_BEGIN, sizeof(CMD17_BEGIN)); //CMD17 begin
	spi_sendData(&address, sizeof(address)); //CMD17 address argument
	spi_sendData(CMD17_END, sizeof(CMD17_END)); //CMD17 end
	
	result_t res = sd_getResponse(resp.data, sizeof(resp));
	if (res == FAILED)
	{
		sd_deselect();
		return FAILED;
	}
		
	for (u8 i=0; i<10; i++)
	{
		if (spi_receiveByte() == 0xFE) //start block token
		{
			//get data block from sd
			for (size_t u=0; u<size; u++)
			{
				*buf = spi_receiveByte();
				buf++;
			}
			
			//crc16
			spi_receiveByte();
			spi_receiveByte();
			
			sd_deselect();
			return SUCCESS;
		}
	}
	
	sd_deselect();
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
    
    sd_Response1 resp1;
    if (sd_goIdle(&resp1) == FAILED) //send CMD0 -> go idle state and enable SPI mode
    {
        sd_powerOff();
        return FAILED;
    }

	//initialization wait loop
	while (1)
	{
		debug_puts("Voltage check");
		
		//voltage and busy check
		sd_Response3 resp3;
		if (sd_getVoltage(&resp3) == FAILED)
		{
			sd_powerOff();
			return FAILED;
		}
		
		if (resp3.ocr.v27_36) //supply voltage fits
			debug_puts("Supply voltage is in range");
		else
			debug_puts("Supply voltage is NOT in range");
			
		if (!resp3.r1.inIdle) //sd card is ready
			break;
		
		debug_puts("SD Card is busy");	
		timer_delayMs(250);
	}
	
	//set block size to 512 bytes
	return sd_setBlockSize(&resp1);
}
