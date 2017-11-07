#include <avr/io.h>
#include <util/delay.h>
#include "OnLCDLib.h"

#define START_HOUR 11
#define START_MINUTE 59

#define SET_HOUR 12
#define SET_MINUTE 0

#define HOUR 60

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

uint16_t globalTime(uint8_t hours, uint8_t minutes)
{
	return hours*HOUR + minutes;
}

void hoursMinutes(uint16_t current, uint8_t *hours, uint8_t *minutes)
{
	
	uint16_t hour = HOUR;
	*hours = current / hour;
	*minutes = current % hour;
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
	
	uint8_t hours = START_HOUR;
    uint8_t minutes = START_MINUTE;
     
    uint8_t hours_left, minutes_left;
    
    uint16_t current_time, set_time, left_time;
    
    current_time = globalTime(hours, minutes);
    set_time = globalTime(SET_HOUR, SET_MINUTE);

    LCDSetup(LCD_CURSOR_ULINE);
 
    while(1)
    {
        if(minutes > 59)
		{
			hours += 1;
			minutes -= 60;
		}
		if(hours > 24)
		{
			hours -= 24;
		}
		
		current_time = globalTime(hours, minutes);
		if(set_time >= current_time){left_time = set_time - current_time;}
		else{left_time = (set_time + 24*HOUR) - current_time ;}
		
		hours_left = left_time / HOUR;
		minutes_left = left_time % HOUR;
		
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
		
		if(left_time != 0)
		{
			minutes += 1;
			_delay_ms(60000);
		}
		else
		{			
			rotate(RIGHT);

			rotate(LEFT);
			PORTB = 0x00;
			
			_delay_ms(57000);
			minutes += 1;
		}        
    }
}
