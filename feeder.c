#include <avr/io.h>
#include <util/delay.h>
#include "OnLCDLib.h"

#define START_HOUR 7 + 12
#define START_MINUTE 00

#define SET_HOUR 12
#define SET_MINUTE 0

#define TIME_WINDOW 20

#define HOUR 60

#define MDELAY 2500

#define M0 _BV(PB5)
#define M1 _BV(PB4)
#define M2 _BV(PB3)
#define M3 _BV(PB2)

#define ROT 128
#define RIGHT 1
#define LEFT 0

#define BUTTON PB0
#define PIN PINB

uint8_t USED = 0;

void initMotor(void)
{
	DDRB = 0xFF; // all B as output
    PORTB = 0x00; // all low
}

void initButton(void)
{
	DDRB &= ~(1 << BUTTON);
}

uint16_t globalTime(uint8_t hours, uint8_t minutes)
{
	return hours*HOUR + minutes;
}

void hoursMinutes(uint16_t current, uint8_t *hours, uint8_t *minutes)
{
	*hours = current / HOUR;
	*minutes = current % HOUR;
}

void rotate(uint8_t direction)
{	
	uint16_t i;
	for(i = 0; i < ROT; i++)
	{
		if(direction == RIGHT)
		{
			PORTB = M0;	_delay_us(MDELAY);
			PORTB = M1; _delay_us(MDELAY);
			PORTB = M2; _delay_us(MDELAY);
			PORTB = M3;	_delay_us(MDELAY);
		}
		else
		{
			PORTB = M3;	_delay_us(MDELAY);
			PORTB = M2;	_delay_us(MDELAY);
			PORTB = M1;	_delay_us(MDELAY);
			PORTB = M0;	_delay_us(MDELAY);
		}
	}
}

void toScreen(uint8_t hours, uint8_t minutes, uint8_t seconds,
	uint8_t hours_left, uint8_t minutes_left, uint8_t seconds_left)
{
	LCDGotoXY(1, 1);
	LCDWriteString("Current:");
	LCDWriteInt(hours, 2);
	LCDWriteString(":");
	LCDWriteInt(minutes, 2);
	LCDWriteString(":");
	LCDWriteInt(seconds, 2);
	
	LCDGotoXY(2, 2);
	LCDWriteString("Left: ");
	LCDWriteInt(hours_left, 2);
	LCDWriteString(":");
	LCDWriteInt(minutes_left, 2);
	LCDWriteString(":");
	LCDWriteInt(seconds_left, 2);
}

int main(void)
{	
	uint8_t i;
	initMotor();
	initButton();
	
	uint8_t hours = START_HOUR, minutes = START_MINUTE, seconds = 0;
    uint8_t hours_left, minutes_left, seconds_left = 60;
    
    uint16_t current_time, set_time, left_time;
    
    current_time = globalTime(hours, minutes);
    set_time = globalTime(SET_HOUR, SET_MINUTE);
    
    if(set_time >= current_time){left_time = set_time - current_time;}
	else{left_time = (set_time + 24*HOUR) - current_time;}
	hoursMinutes(left_time, &hours_left, &minutes_left);

    LCDSetup(LCD_CURSOR_ULINE);
 
    while(1)
    {
		for(i = 0; i < 100; i++)
		{
			if((current_time > set_time - TIME_WINDOW) & (current_time < set_time + TIME_WINDOW))
			{
				if(PIN & (1 << BUTTON) & (USED == 0))
				{
					rotate(LEFT);
					rotate(RIGHT);
					
					PORTB &= (1 << BUTTON);
					seconds += 2;
					USED = 1;
					break;
				}
			}
			else if((current_time == set_time + TIME_WINDOW) & (seconds == 0) & (USED == 0))
			{
				rotate(LEFT);
				rotate(RIGHT);
				
				PORTB &= (1 << BUTTON);
				seconds += 2;
				break;
			}
			else if(USED == 1)
			{
				USED = 0;
			}
			_delay_ms(10);
		}
		
		if(seconds < 60)
		{				
			seconds += 1;
			seconds_left -= 1;
		}
		else
		{
			seconds = 0;
			seconds_left = 60;
			
			minutes += 1;
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
			else{left_time = (set_time + 24*HOUR) - current_time;}
			
			hoursMinutes(left_time, &hours_left, &minutes_left);
		}
		
		toScreen(hours, minutes, seconds, hours_left, minutes_left, seconds_left);
    }
    return 0;
}
