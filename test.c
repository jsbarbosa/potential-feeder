#include <avr/io.h>
#include <util/delay.h>
#include "OnLCDLib.h"

#define START_HOUR 11
#define START_MINUTE 59
#define START_SECOND 45

#define SET_HOUR 12
#define SET_MINUTE 0
#define SET_SECOND 0

uint8_t

#define HOUR 3600
#define MINUTE 60

#define M0 _BV(PB5)
#define M1 _BV(PB4)
#define M2 _BV(PB3)
#define M3 _BV(PB2)

#define ROT 128
#define RIGHT 1
#define LEFT 0

void initMotor(void)
{
	DDRB = 0xFF; // all B as output
    PORTB = 0x00; // all low
}

void timeLeft(uint8_t hours, uint8_t minutes, uint8_t seconds, 
		uint8_t *hours_left, uint8_t *minutes_left, uint8_t *seconds_left)
{
	if(SET_SECOND < seconds)
	{
		if(minutes_left > 0){minutes_left -= 1;}
		else{minutes_left += 60; hours_left -= 1;}
		seconds_left += 60;
	}
	seconds_left -= seconds;
	if(minutes_left < minutes)
	{
		minutes_left += 60;
		hours_left -= 1;
	}
	minutes_left -= minutes;
	hours_left -= hours;
	
}

void rotate(uint8_t direction)
{	
	uint16_t i;
	for(i = 0; i < ROT; i++)
	{
		if(direction == RIGHT)
		{
			PORTB = M0;
			_delay_us(2500);
			PORTB = M1;
			_delay_us(2500);
			PORTB = M2;
			_delay_us(2500);
			PORTB = M3;
			_delay_us(2500);
		}
		else
		{
			PORTB = M3;
			_delay_us(2500);
			PORTB = M2;
			_delay_us(2500);
			PORTB = M1;
			_delay_us(2500);
			PORTB = M0;
			_delay_us(2500);
		}
	}
}

int main(void)
{	
	initMotor();
	
    uint8_t seconds = 45;
    uint8_t minutes = 59;
    uint8_t hours = 11; 
    
    uint8_t seconds_left = 0;
    uint8_t minutes_left = 0;
    uint8_t hours_left = 0; 
    
    uint8_t seconds_set = 0;
    uint8_t minutes_set = 0;
    uint8_t hours_set = 12;
 
    LCDSetup(LCD_CURSOR_ULINE);
 
    while(1)
    {
        if(seconds > 59)
        {
            minutes += 1;
            seconds -= 60;
        }
        if(minutes > 59)
		{
			hours += 1;
			minutes -= 60;
		}
		if(hours > 24)
		{
			hours -= 24;
		}
		
        LCDGotoXY(1, 1);
		LCDWriteString("Current: ");
		LCDWriteInt(hours, 2);
		LCDWriteString(":");
		LCDWriteInt(minutes, 2);
		
		LCDGotoXY(2, 2);
		LCDWriteString("Left: ");
		
		LCDWriteInt(hours_left, 2);
		
		LCDWriteString(":");
		LCDWriteInt(minutes_left, 2);
		
		if((seconds_set != seconds) | (minutes_set != minutes) | (hours_set != hours))
		{
			seconds += 1;
			_delay_ms(1000);
		}
		else
		{
			seconds_left = 0;
			minutes_left = 0;
			hours_left = 24;
			
			rotate(RIGHT);
			PORTB = 0x00;

			_delay_ms(1000);

			rotate(LEFT);
			PORTB = 0x00;
			
			seconds += 4;
		}        
    }
}
