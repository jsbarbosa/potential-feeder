/*_______________________________________________________________________________
Copyright 2015 Istrate Liviu

OnLCDLib - LCD Library written in C for AVR Microcontrollers v1.4

This is a library used for interfacing with Standard Alpha Numeric LCD Modules using 4 or 8 bit mode.
Supports 16x1, 16x2, 16x4, 20x4, 20x2, 32x2, 40x2 LCD displays.

FEATURES
--------
* 	Converts integers to strings (positive or negative) and displays them on screen.
	Optionally the numbers can be padded with zeros.
* 	It does not use external libraries to convert integers to strings thus reducing size on MCU.
*	Data can be displayed at a certain position using only one function.
*	Support for 8 and 4 bit mode.
*	Support for different LCD screens (still in development)
*	LCD backlight dimming or turn off using PWM.
*	Display double height digits and separator for digital clocks using custom fonts.


HOW TO USE
----------
1. Setting things up
- Bellow in the setup section modify the setup as needed
- In your main function, use  "LCDSetup(cursorStyle)":
	"cursorStyle" can be: LCD_CURSOR_BLINK, LCD_CURSOR_ULINE, LCD_CURSOR_NONE
	
2. Interfacing with LCD
STRINGS
- Send a string to the LCD:
	"LCDWriteString(aString)"
- Send a string to a specific location. x is character position, y is line/row number.
  x and y can start from 0 or 1 depending how user prefers:
	"LCDWriteStringXY(x, y, aString)"

INTEGERS
- Send a integer number:
	"LCDWriteInt(number, nr_of_digits)"
	"nr_of_digits" - number of digits. If the number to be displayed has less digits than "nr_of_digits"
	then it will be padded with zeros. E.g:
	uint16_t ADC_results = 120;
	LCDWriteInt(ADC_results, 5);
	will display "00120"
	LCDWriteInt(ADC_results, 3);
	will display "120".
	You can also display negative numbers.
- Send a integer number to a specific location:
	"LCDWriteIntXY(x, y, number, nr_of_digits)"
	
BIG DIGITS
- Double height digits:
	- Custom double height digits 1 character wide font can be found in "double_height_sharp_digits.h".
	Copy data from there in this file in the variable "LCD_custom_chars". Custom char must be 
	uncommented in the setup section. "nrOfDigits" has the same function as in "LCDWriteInt" function.
		"LCDWriteIntBig(int16_t number, int8_t nrOfDigits)"
	
	- Custom double height digits 3 characters wide font can be found in "double_height_3_characters_round_digits.h".
	Same as above but needs to be used with "LCDWriteIntBig3Chars" function.
	"LCDWriteIntBig3Chars(int16_t number, int8_t nrOfDigits)"
	
BIG SEPARATOR
- Big 2 lines height separator. Can be used in conjunction with big digits to make a digital clock.
The function displays ":" but bigger, on two lines.
	"LCDWriteBigSeparator(void)"
	
LCD COMMANDS
- Move cursor to a specific location:
	"LCDGotoXY(character_position, row_number)"
- Clear display and DDRAM:
	"LCDClear()"
- Go to character 1, line 1
	"LCDHome()"
	
BACKLIGHT PWM
- Dim LCD backlight using Fast PWM Timer0, channel B (OC0B pin), OCR0A as TOP.
Frequency is set to 400 to prevent flickering. "brightness" can be between 0 - 100.
"0" will turn off the backlight and the LCD without clearing DDRAM thus saving power.
"100" will turn LED backlight fully on and stop PWM.
Circuit: a small signal transistor can be used with emitter connected to ground.
Connect OC0B pin to the base of transistor. Connect LCD backlight anode to Vcc and
cathode to collector.
	"LCDBacklightPWM(uint8_t brightness)"

3. Animations
- Scroll a string from right to left. Needs to be uncommented in setup section:
	"LCDScrollText(aString)"
	
4. Utils
- Find characters positions where lines start and end. Needs to be uncommented in setup section:
  Puts an x and increments cursor position showing the current position.
  LCD_X_POS_DELAY in how fast to increment cursor.
	"LCDFindCharPositions()"

Tips:
- It is faster to replace something with spaces than to use "LCDClear" function.
Use clear function only to clear entire screen.

NOTICE
--------
NO PART OF THIS WORK CAN BE COPIED, DISTRIBUTED OR PUBLISHED WITHOUT A
WRITTEN PERMISSION FROM THE AUTHOR. THE LIBRARY, NOR ANY PART
OF IT CAN BE USED IN COMMERCIAL APPLICATIONS. IT IS INTENDED TO BE USED FOR
HOBBY, LEARNING AND EDUCATIONAL PURPOSE ONLY. IF YOU WANT TO USE THEM IN 
COMMERCIAL APPLICATION PLEASE WRITE TO THE AUTHOR.


AUTHOR: Istrate Liviu
CONTACT: istrateliviu24@yahoo.com
I don't mind if you send me a beer through Paypal.
__________________________________________________________________________________*/

#ifndef OnLCDLib
#define OnLCDLib

/*************************************************************
	INCLUDES
**************************************************************/
#include <avr/io.h>
#include <util/delay.h>

/*************************************************************
	DEFINE SETUP
**************************************************************/
/*--------------- SETUP HERE --------------------------------------------*/
// MCU IO Setup															 |
#define LCD_DATA_DDR 		DDRC 	// Data bus (DB0 to DB7 on LCD pins) |	
#define LCD_DATA_PORT 		PORTC 	//									 |
#define LCD_DATA_PIN 		PINC 	// Used to check busy flag			 |
#define LCD_DATA_START_PIN	2		// In 8-bit mode pins 0-7 will be used, in 4-bit mode 0-3 if 0 is first
#define LCD_RS_CONTROL_DDR 	DDRD 	//									 |
#define LCD_RW_CONTROL_DDR 	DDRD 	//									 |
#define LCD_E_CONTROL_DDR 	DDRD 	//									 |
#define LCD_RS_CONTROL_PORT PORTD 	// Port where RS, RW, E pins are	 |
#define LCD_RW_CONTROL_PORT PORTD 	// Port where RS, RW, E pins are	 |
#define LCD_E_CONTROL_PORT 	PORTD 	// Port where RS, RW, E pins are	 |
#define LCD_RS_PIN			PD0 	// Register selection signal		 |
#define LCD_RW_PIN			PD1 	// Read/write signal 				 |
#define LCD_E_PIN 			PD2 	// Enable signal					 |
//																		 |
// LCD type																 |
#define LCD_NR_OF_CHARACTERS 	16 	// e.g 16 if LCD is 16x2 type	     |
#define LCD_NR_OF_ROWS 		 	2 	// e.g 2 if LCD is 16x2 type		 |
//																		 |
// MCU bits																 |
#define PORT_SIZE			 	8 	// 8 bit microcontroller			 |
//																		 |
// Backlight brightness control using PWM								 |
#define LCD_BACKLIGHT				// Comment out to deactivate and save space if not needed
#define LCD_PWM_DDR			DDRD 	// OC0B	DDR for backlight brightness control
#define LCD_PWM_PORT		PORTD	//									 |
#define LCD_PWM_PIN			PD5 	// OC0B	pin for backlight brightness control
//									 									 |
// Select 4 or 8 bit mode (uncomment just one)							 |
#define BIT_MODE_4					// 									 |
// #define BIT_MODE_8				// 									 |
//																		 |
// Text wrap - If defined and the text length is greater than the numbers of characters
// per line on LCD, the cursor will be set on the beginning of the next line
#define LCD_WRAP					// 									 |
//																		 |
// Use of custom characters (if not used comment out to save space)		 |
//#define CUSTOM_CHARS				// 									 |
//																		 |
// Use of big double height digits (if not used comment out to save space)
//#define BIG_DIGITS  			    // Uncomment to use custom big digits|
#define BIG_DIGITS_3_CHARACTERS	    // 3 characters wide digits 		 |
//																		 |
// * Animations * (Uncomment if needed)									 |
// This types of LCD aren't meant for animations.                        |
// The crystals have slow rise and fall times. Use TFT LCDs for animations
// But if you want you can try this function.							 |
// Leave commented if you don't use to save space on MCU				 |
//#define LCD_ANIMATIONS			    //	     			                 |
#define LCD_SCROLL_SPEED		200 // In milliseconds					 |
//																		 |
// * Utils * (Uncomment if needed)									 	 |
// A function used to find positions on LCD                              |
// #define LCD_UTILS				//						  			 |
// #define LCD_X_POS_DELAY		200 // In milliseconds					 |
/*-----------------------------------------------------------------------*/

// LCD Commands
#define LCD_SHIFT_RIGHT 	0b00011100
#define LCD_SHIFT_LEFT	 	0b00011000
#define LCD_DISPLAY_ON	 	0b00001100
#define LCD_DISPLAY_OFF	 	0b00001000
#define LCD_CURSOR_BLINK 	0b00000011
#define LCD_CURSOR_ULINE 	0b00000010
#define LCD_CURSOR_NONE	 	0b00000000

/*************************************************************
	FUNCTION PROTOTYPES
**************************************************************/
void LCDSetup(uint8_t cursorStyle);
void LCDWriteString(const char *msg);
void LCDWriteInt(int16_t number, int8_t nrOfDigits);
void LCDWriteIntBig(int16_t number, int8_t nrOfDigits);
void LCDWriteIntBig3Chars(int16_t number, int8_t nrOfDigits);
void LCDWriteBigSeparator(void);
void LCDGotoXY(uint8_t x, uint8_t y);
void LCDBacklightPWM(uint8_t brightness);
void LCDByte(uint8_t, uint8_t);
void LCDBusyLoop(void);
void FlashEnable(void);
// Animations
void LCDScrollText(const char *text);
// Utils
void LCDFindCharPositions(void);

#ifdef LCD_ANIMATIONS
#include <string.h>
#endif

#ifdef CUSTOM_CHARS
static const uint8_t LCD_custom_chars[] = {
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, //Char0
	0x1E, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, //Char1
	0x1E, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1E, //Char2
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x1E, //Char3
	0x1E, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x1E, //Char4
	0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x1E, //Char5
	0x1E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x1E, //Char6
	0x1E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, //Char7
};
#endif


/*************************************************************
	MACROS
**************************************************************/
#define LCDCmd(c) LCDByte(c, 0)  // send a command to LCD
#define LCDData(d) LCDByte(d, 1) // send data to LCD

#define LCDClear() LCDCmd(0b00000001)
#define LCDHome() LCDCmd(0b10000000)

#define E_ON() (LCD_E_CONTROL_PORT |= (1 << LCD_E_PIN))
#define RS_ON() (LCD_RS_CONTROL_PORT |= (1 << LCD_RS_PIN))
#define RW_ON() (LCD_RW_CONTROL_PORT |= (1 << LCD_RW_PIN))
#define E_OFF() (LCD_E_CONTROL_PORT &= (~(1 << LCD_E_PIN)))
#define RS_OFF() (LCD_RS_CONTROL_PORT &= (~(1 << LCD_RS_PIN)))
#define RW_OFF() (LCD_RW_CONTROL_PORT &= (~(1 << LCD_RW_PIN)))

#define LCDWriteStringXY(x, y, msg){\
	 LCDGotoXY(x, y);\
	 LCDWriteString(msg);\
}

#define LCDWriteIntXY(x, y, nr, nrOfDigits){\
	 LCDGotoXY(x, y);\
	 LCDWriteInt(nr, nrOfDigits);\
}




/*************************************************************
	FUNCTIONS
**************************************************************/
#ifdef LCD_BACKLIGHT
uint8_t cursorType = 0b00001100; // Display on, cursor off by default
#endif

void LCDSetup(uint8_t cursorStyle){
	// After power on wait for LCD to initialize. On 3.3v LCD clock will be slower so add more delay
	_delay_ms(100);
	
	// Save cursor style - used by LCDBacklightPWM function
	#ifdef LCD_BACKLIGHT
	cursorType = cursorStyle;
	#endif
	
	// Set MCU IO Ports
	LCD_DATA_DDR |= (0x0F << LCD_DATA_START_PIN);
	LCD_DATA_PORT &= (~(0x0F << LCD_DATA_START_PIN));
	LCD_RS_CONTROL_DDR |= (1 << LCD_RS_PIN);
	LCD_RW_CONTROL_DDR |= (1 << LCD_RW_PIN);
	LCD_E_CONTROL_DDR  |= (1 << LCD_E_PIN) | (1 << LCD_RW_PIN) | (1 << LCD_RS_PIN);
	E_OFF();
	RW_OFF();
	RS_OFF();
	
	#ifdef BIT_MODE_8
		LCDCmd(0b00001100 | cursorStyle); // Turn on display, set cursor type
		LCDCmd(0x38); // 8 bit mode. Function Set: 8-bit, 2 Line, 5x7 Dots
	#elif defined BIT_MODE_4
		// LCD reset instructions according to datasheet's flowchart
		LCD_DATA_PORT |= 0b00000011 << LCD_DATA_START_PIN;
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN); // Clear pins
		_delay_ms(10);
	
		LCD_DATA_PORT |= 0b00000011 << LCD_DATA_START_PIN;
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		_delay_ms(1);
		
		LCD_DATA_PORT |= 0b00000011 << LCD_DATA_START_PIN;
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		_delay_ms(1);
		// End reset
		
		LCD_DATA_PORT |= 0b00000010 << LCD_DATA_START_PIN; // Set 4 bit mode
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		_delay_ms(1);
		
		// 4 bit mode. Function Set: 4-bit, 2 Line, 5x7 Dots. Lines are number of memory lines
		// not rows on LCD. There are LCDs with 1 line/row that have 2 memory lines and other
		// LCDs with 1 row with 1 memory line. Please read this article for a better understanding
		// http://web.alfredstate.edu/weimandn/lcd/lcd_addressing/lcd_addressing_index.html
		LCD_DATA_PORT |= (0b00000010 | 0b00000000) << LCD_DATA_START_PIN;
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		_delay_ms(1);
		
		LCD_DATA_PORT |= 0b00001000 << LCD_DATA_START_PIN; // Display off
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		_delay_ms(1);
		
		LCD_DATA_PORT |= 0b00000001 << LCD_DATA_START_PIN; // Clear Display
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		_delay_ms(5);
		
		LCD_DATA_PORT |= 0b00000110 << LCD_DATA_START_PIN; // Entry Mode Set
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		_delay_ms(1);
		
		LCD_DATA_PORT |= 0b00001100 << LCD_DATA_START_PIN; // Turn on display
		_delay_ms(1);
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		FlashEnable();
		// End of intitalization
	
		LCD_DATA_PORT |= ((0b00000010) << LCD_DATA_START_PIN);
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
		_delay_ms(10); // busy flag is not available yet
		LCDCmd(LCD_DISPLAY_ON | cursorStyle); // Turn on display, set cursor type
		//LCDCmd(0x28); // 4 bit mode. Function Set: 4-bit, 2 Line, 5x7 Dots
	#endif

	#ifdef CUSTOM_CHARS
	// Upload custom characters to LCD's volatile memory
	uint8_t i, location, pos;
	for(location=0; location < 8; location++){
		pos = location * 8;
		LCDCmd(0b01000000 + pos); // Set location in CGRAM where to write custom char
		for(i=pos; i < pos+8; i++){
			LCDData(LCD_custom_chars[i]);
		}
	}
	#endif
	
	#ifdef LCD_BACKLIGHT
	LCDBacklightPWM(100);
	#endif
	
	LCDClear();
	LCDHome();
}

void LCDWriteString(const char *msg){
	uint8_t pos=1;
	uint8_t line=1;
	
	while(*msg > 0){
		#ifdef LCD_WRAP
			if(pos > LCD_NR_OF_CHARACTERS){
				if(LCD_NR_OF_ROWS > 1){
					if(line == 1){
						LCDGotoXY(1, 2); // go to line 2
						line = 2;
					}else if(line == 2 && LCD_NR_OF_ROWS > 2){
						LCDGotoXY(1, 3); // go to line 3
						line = 3;
					}else if(line == 3 && LCD_NR_OF_ROWS > 3){
						LCDGotoXY(1, 4); // go to line 4
						line = 4;
					}
					
					if(line > 1 && *msg == 0x20) msg++; // remove space if it is at the beginning of the line
				}
				
				pos = 1;
			}
		#endif
		
		// Custom Char Support
		#ifdef CUSTOM_CHARS
			if(*msg == '%'){
				msg++;
				int8_t c = *msg - '0';

				if(c >= 0 && c < 8){
					LCDData(c);
				}else{
					LCDData('%');
					LCDData(*msg);
				}
			}else{
				LCDData(*msg);
			}
		#else
			LCDData(*msg);
		#endif
			
		msg++;
		pos++;
	}
}

uint8_t cursorPosition=1, cursorLine=1;
#if defined(BIG_DIGITS) && defined(CUSTOM_CHARS)
void LCDWriteBigSeparator(){
	// Calculate new cursor position based on current position
	LCDGotoXY(cursorPosition, 1);
		
	LCDData(0b10100101); // big dot
	LCDGotoXY(cursorPosition-1, 2);
	LCDData(0b10100101); // big dot
}

void LCDWriteIntBig(int16_t number, int8_t nrOfDigits){
	uint8_t new_pos, line, length=0, i;
	uint8_t buffer[7] = {0};
	int16_t copyOfNumber = number;
	
	// Find number of digits
	while(copyOfNumber != 0){ 
		length++;
		copyOfNumber /= 10;
	}
	
	if(number == 0){
		buffer[1] = 0;
		length = 1;
	}
	
	copyOfNumber = number;
	nrOfDigits -= length;
	
	if(nrOfDigits < 0) nrOfDigits = 0;
		
	if(number < 0){
		LCDGotoXY(cursorPosition, 1);
		LCDData('_');
		copyOfNumber = 0 - number;
	}
	
	length += nrOfDigits;
	for(i=0; i<length; i++){
		buffer[i] = copyOfNumber % 10;
		copyOfNumber /= 10;
	}
	
	
	// Display the numbers
	while(length){
		// Calculate new cursor position based on current position
		LCDGotoXY(cursorPosition, 1);
		new_pos = cursorPosition;
		line = 2;
	
		switch(buffer[length-1]){
			case 0:
				LCDData(1);
				LCDGotoXY(new_pos, line);
				LCDData(5);
			break;
			
			case 1:
				LCDData(0);
				LCDGotoXY(new_pos, line);
				LCDData(0);
			break;
			
			case 2:
				LCDData(7);
				LCDGotoXY(new_pos, line);
				LCDData(2);
			break;
			
			case 3:
				LCDData(6);
				LCDGotoXY(new_pos, line);
				LCDData(3);
			break;
			
			case 4:
				LCDData(5);
				LCDGotoXY(new_pos, line);
				LCDData(0);
			break;
			
			case 5:
				LCDData(2);
				LCDGotoXY(new_pos, line);
				LCDData(3);
			break;
			
			case 6:
				LCDData(2);
				LCDGotoXY(new_pos, line);
				LCDData(5);
			break;
			
			case 7:
				LCDData(7);
				LCDGotoXY(new_pos, line);
				LCDData(0);
			break;
			
			case 8:
				LCDData(4);
				LCDGotoXY(new_pos, line);
				LCDData(5);
			break;
			
			case 9:
				LCDData(4);
				LCDGotoXY(new_pos, line);
				LCDData(3);
			break;
		}
		
		length--;
	}
}
#endif

#if defined(BIG_DIGITS_3_CHARACTERS) && defined(CUSTOM_CHARS)
void LCDWriteIntBig3Chars(int16_t number, int8_t nrOfDigits){
	uint8_t new_pos, line, length=0, i;
	uint8_t buffer[7] = {0};
	int16_t copyOfNumber = number;
	
	// Clear previous digits
	new_pos = cursorPosition;
	line = cursorLine;
	LCDGotoXY(new_pos, 1);
	for(i=0; i<7; i++){
		LCDData(' ');
	}
	LCDGotoXY(new_pos, cursorLine+1);
	for(i=0; i<7; i++){
		LCDData(' ');
	}
	LCDGotoXY(new_pos, line);
	
	// Find number of digits
	while(copyOfNumber != 0){ 
		length++;
		copyOfNumber /= 10;
	}
	
	if(number == 0){
		buffer[1] = 0;
		length = 1;
	}
	
	copyOfNumber = number;
	nrOfDigits -= length;
	
	if(nrOfDigits < 0) nrOfDigits = 0;
		
	if(number < 0){
		LCDGotoXY(cursorPosition, 1);
		LCDData('_');
		copyOfNumber = 0 - number;
	}
	
	length += nrOfDigits;
	for(i=0; i<length; i++){
		buffer[i] = copyOfNumber % 10;
		copyOfNumber /= 10;
	}
	
	
	// Display the numbers
	while(length){
		// Calculate new cursor position based on current position
		LCDGotoXY(cursorPosition, 1);
		new_pos = cursorPosition;
		line = 2;
	
		switch(buffer[length-1]){
			case 0:
				LCDData(0);
				LCDData(1);
				LCDData(2);
				LCDGotoXY(new_pos, line);
				LCDData(3);
				LCDData(4);
				LCDData(5);
			break;
			
			case 1:
				LCDData(1);
				LCDData(2);
				LCDGotoXY(new_pos, line);
				LCDData(4);
				LCDData(7);
				LCDData(4);
			break;
			
			case 2:
				LCDData(6);
				LCDData(6);
				LCDData(2);
				LCDGotoXY(new_pos, line);
				LCDData(3);
				LCDData(4);
				LCDData(4);
			break;
			
			case 3:
				LCDData(6);
				LCDData(6);
				LCDData(2);
				LCDGotoXY(new_pos, line);
				LCDData(4);
				LCDData(4);
				LCDData(5);
			break;
			
			case 4:
				LCDData(3);
				LCDData(4);
				LCDData(7);
				LCDGotoXY(new_pos+2, line);
				LCDData(7);
			break;
			
			case 5:
				LCDData(3);
				LCDData(6);
				LCDData(6);
				LCDGotoXY(new_pos, line);
				LCDData(4);
				LCDData(4);
				LCDData(5);
			break;
			
			case 6:
				LCDData(0);
				LCDData(6);
				LCDData(6);
				LCDGotoXY(new_pos, line);
				LCDData(3);
				LCDData(4);
				LCDData(5);
			break;
			
			case 7:
				LCDData(1);
				LCDData(1);
				LCDData(2);
				LCDGotoXY(new_pos+2, line);
				LCDData(7);
			break;
			
			case 8:
				LCDData(0);
				LCDData(6);
				LCDData(2);
				LCDGotoXY(new_pos, line);
				LCDData(3);
				LCDData(4);
				LCDData(5);
			break;
			
			case 9:
				LCDData(0);
				LCDData(6);
				LCDData(2);
				LCDGotoXY(new_pos+2, line);
				LCDData(7);
			break;
		}
		
		LCDData(' '); // Insert a space between digits to distinguish them better
		length--;
	}
}
#endif

void LCDWriteInt(int16_t number, int8_t nrOfDigits){
	char string[7] = {0};
	uint8_t isNegative = 0, length = 0, divide;
	int16_t copyOfNumber = number;
	
	// Find number of digits
	while(copyOfNumber != 0){ 
		length++;
		copyOfNumber /= 10;
	}
	
	copyOfNumber = number;
	nrOfDigits -= length;
	
	if(nrOfDigits < 0) nrOfDigits = 0;
		
	if(number < 0){
		isNegative = 1;
		copyOfNumber = 0 - copyOfNumber;
		length++;
	}
		
	for(divide = length + nrOfDigits; divide > 0; divide--){
		string[divide-1] = (copyOfNumber % 10) + '0';
		copyOfNumber /= 10;
	}
	
	if(isNegative) string[0] = '-';
	
	LCDWriteString(string);
}

void LCDGotoXY(uint8_t x, uint8_t y){
	LCDBusyLoop();
	if(x == 0 || x == 255) x = 1; // User can use 0 or 1 as starting character position
	cursorPosition = x;
	cursorLine = y;
	
	switch(y){
		case 255: // If a variable is decremented and is negative it will reset to 255 because the parameter is unsigned
		case 0:
		case 1:
			x -= 1; // User can use values starting from 1, but LCD starts from 0 so we substract 1
		break;
		
		case 2:
			x += 63; // Line 2
		break;
		
		case 3:
			x += LCD_NR_OF_CHARACTERS - 1; // Line 3
		break;
		
		case 4:
			x += 79 + (LCD_NR_OF_CHARACTERS - 16); // Line 4
		break;
	}
	
	x |= 0b10000000;
	LCDCmd(x);
}

#ifdef LCD_BACKLIGHT
void LCDBacklightPWM(uint8_t brightness){
	unsigned long prescaler = 256;
	
	if(brightness > 99){ // account for values greater than 100
		// No need for PWM - stop the timer
		TCCR0B = 0;
		TCCR0A = 0;
		LCD_PWM_PORT |= 1 << LCD_PWM_PIN;
		LCDCmd(LCD_DISPLAY_ON); // turn on display
	}else if(brightness < 1){ // account for negative values
		// Stop the timer and turn off LCD backlight
		TCCR0B = 0;
		TCCR0A = 0;
		LCD_PWM_PORT &= ~(1 << LCD_PWM_PIN);
		LCDCmd(0x08); // turn off display and cursor without clearing DDRAM
	}else{ // values between 1 and 99
		// Set Timer0 in Fast PWM mode and set OC0B pin on compare match, OCR0A as TOP
		LCD_PWM_DDR |= 1 << LCD_PWM_PIN;
		TCCR0A 	|= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
		TCCR0B 	|= (1 << WGM02) | (1 << CS02); // prescaler set to 256
		
		// Calculate OCR0A (TOP value) and set the frequency
		OCR0A = (F_CPU / (prescaler * 400)) - 1; // prescaler * frequency
		
		// Set duty cycle
		OCR0B = (OCR0A * brightness) / 100;
		LCDCmd(LCD_DISPLAY_ON); // turn on display
	}
}
#endif

void LCDByte(uint8_t data, uint8_t isdata){
	LCDBusyLoop();
	
	if(isdata == 0){
		RS_OFF(); // Send command - RS to 0
		if(data == 0b10000000 || data == 0b00000001){
			cursorPosition = 1;
			cursorLine = 1;
		}
	}else{
		RS_ON(); // Send data - RS to 1
		cursorPosition++;
	}
	
	RW_OFF(); // RW to 0 - write mode
	
	#ifdef BIT_MODE_8
		LCD_DATA_PORT = data;
		FlashEnable();
		LCD_DATA_PORT = 0x00;
	#elif defined BIT_MODE_4
		unsigned char temp; // If signed, after shift MSB will be replaced with 1 instead of 0 and we don't want that
		uint8_t shift = PORT_SIZE - (LCD_DATA_START_PIN + 4);
	
		// Send high nibble
		temp = (data & 0xF0); // Mask the lower nibble
		LCD_DATA_PORT |= temp >> shift; // Put data on data port
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN);
	
		// Send low nibble
		temp = ((data << 4) & 0xF0); // Shift 4-bit and mask
		LCD_DATA_PORT |= temp >> shift;
		FlashEnable();
		LCD_DATA_PORT &= ~(0x0F << LCD_DATA_START_PIN); // Clear data port
	#endif
}

void LCDBusyLoop(){
	#ifdef BIT_MODE_8
		LCD_DATA_DDR = 0x00;
	#elif defined BIT_MODE_4
		LCD_DATA_DDR &= ~(0x0F << LCD_DATA_START_PIN); // Set DDR to input for reading LCD status
	#endif
	
	RW_ON();		// Read mode
	RS_OFF();		// Read status
	
	// Check LCD status 0b10000000 means busy, 0b00000000 means clear
	#ifdef BIT_MODE_8
		do{
			FlashEnable();
		}while(LCD_DATA_PIN >= 0x80);
	#elif defined BIT_MODE_4
		uint8_t busy, high_nibble;
		
		do{
			// Read high nibble
			E_ON();
			_delay_us(1); // Implement 'Delay data time' (160 nS) and 'Enable pulse width' (230 nS)
			high_nibble = LCD_DATA_PIN >> LCD_DATA_START_PIN;
			high_nibble = high_nibble << 4;
			E_OFF();
			_delay_us(1); // Implement 'Address hold time' (10 nS), 'Data hold time' (10 nS), and 'Enable cycle time' (500 nS )
			
			// No need for low nibble
			E_ON();
			_delay_us(1);
			E_OFF();
			_delay_us(1);
			
			busy = high_nibble & 0b10000000;
		}while(busy);
	#endif
		
	RW_OFF();
	RS_ON();
		
	#ifdef BIT_MODE_8
		LCD_DATA_DDR = 0xFF; // Set DDR to output again
	#elif defined BIT_MODE_4
		LCD_DATA_DDR |= (0x0F << LCD_DATA_START_PIN);
	#endif
}

void FlashEnable(){
	E_ON(); // Enable on
	_delay_us(50); // Wait
	E_OFF(); // Execute
}

/* ----------------------------------- ANIMATIONS */
#ifdef LCD_ANIMATIONS
void LCDScrollText(const char *text){
	uint8_t shift_number=1, i=0, chars_to_display=1;
	size_t text_size = strlen(text);
	const char *text_start_pos = text;

	LCDClear();
	LCDGotoXY(LCD_NR_OF_CHARACTERS, 1);
	
	while(shift_number < text_size + LCD_NR_OF_CHARACTERS){
		
		while(i < chars_to_display){
			LCDData(*text);
			i++;
			text++;
		}
		
		_delay_ms(LCD_SCROLL_SPEED); // 200 - 300 is a good choise
		LCDClear();
		
		if(shift_number < LCD_NR_OF_CHARACTERS && shift_number < text_size){
			LCDGotoXY(LCD_NR_OF_CHARACTERS - shift_number, 1);
			text = text_start_pos;
			chars_to_display++;
		}else{
			if(text_size > LCD_NR_OF_CHARACTERS){
				if(shift_number + 1 > text_size)
					chars_to_display--;
				else
					chars_to_display = LCD_NR_OF_CHARACTERS;
				
				text = text_start_pos + (shift_number - (LCD_NR_OF_CHARACTERS - 1));
				LCDGotoXY(1, 1);
			}else{
				if(shift_number + 1 > LCD_NR_OF_CHARACTERS){
					chars_to_display--;
					text = text_start_pos + (shift_number - LCD_NR_OF_CHARACTERS) + 1;
					LCDGotoXY(1, 1);
				}else{
					text = text_start_pos;
					chars_to_display = text_size;
					LCDGotoXY(LCD_NR_OF_CHARACTERS - shift_number, 1);
				}
			}
		}
		
		i = 0;
		shift_number++;
	}
	
	LCDHome();
}
#endif

/* ----------------------------------- UTILS */ 
#ifdef LCD_UTILS
void LCDFindCharPositions(void){
	uint8_t x;
	
	for(x=0; x<255; x++){
		LCDCmd(0x80 | x);
		LCDWriteString("x");
		
		LCDHome();
		LCDWriteString("X:");
		LCDWriteInt(x, 3);
		
		_delay_ms(LCD_X_POS_DELAY);
		LCDClear();
	}
}
#endif
#endif // OnLCDLib
