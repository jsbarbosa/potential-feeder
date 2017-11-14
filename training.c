#include <avr/io.h>
#include <util/delay.h>
#include "OnLCDLib.h"

#define START_HOUR 21
#define START_MINUTE 00

#define SET_HOUR 12
#define SET_MINUTE 0

#define HOUR 60

#define MDELAY 2500

#define M0 _BV(PB5)
#define M1 _BV(PB4)
#define M2 _BV(PB3)
#define M3 _BV(PB2)

#define BUTTON PB0
#define PIN PINB

#define ROT 16
#define RIGHT 1
#define LEFT 0

void initMotor(void)
{
	DDRB = 0xFF; // all B as output
    PORTB = 0x00; // all low
}

void initButton(void)
{
	DDRB &= ~(1 << BUTTON);
} 

void rotate(uint8_t direction)
{	
	uint16_t i;
	for(i = 0; i < ROT; i++)
	{
		if(direction == RIGHT)
		{
			PORTB = M0;
			_delay_us(MDELAY);
			PORTB = M1;
			_delay_us(MDELAY);
			PORTB = M2;
			_delay_us(MDELAY);
			PORTB = M3;
			_delay_us(MDELAY);
		}
		else
		{
			PORTB = M3;
			_delay_us(MDELAY);
			PORTB = M2;
			_delay_us(MDELAY);
			PORTB = M1;
			_delay_us(MDELAY);
			PORTB = M0;
			_delay_us(MDELAY);
		}
	}
}

int main(void)
{	
	initMotor();
	
    LCDSetup(LCD_CURSOR_ULINE);
 
    while(1)
    {		
        LCDGotoXY(1, 1);
		LCDWriteString("TRAINING");
		
		if(PIN & (1 << BUTTON))
		{			
			rotate(LEFT);

			rotate(RIGHT);
			
			PORTB &= (1 << BUTTON);
			
			_delay_ms(500);
		}
		_delay_ms(10);
    }
}
