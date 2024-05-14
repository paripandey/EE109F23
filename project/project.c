/********************************************
 *
 *  Name: Pari Pandey
 *  Email: paripand@usc.edu
 *  Section: Wednesday 2 PM Lab
 *  Assignment: Project
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "lcd.h"
#include "ds18b20.h"
#include "timers.h"

volatile uint8_t new_state, old_state; // used for the rotary encoder
uint8_t control_state = 0; // used to indicate HIGH/LOW setting
volatile uint8_t servo_state = 0; // used to determine the position of the server (before or after 4s)
volatile uint8_t changed = 0;  // flag for state change
uint8_t sounded = 0; // flag for buzzer sounding (>3°F below LOW setting or above HIGH setting)
volatile int16_t count;	// used to indicate the selected HIGH/LOW temperature
volatile int16_t cycles = 1000; // frequency for 500Hz
volatile int16_t count_timer = 0; // used for 4 second delay

volatile char x; // used to capture PINC register for rotary encoder
volatile uint8_t a, b; // used for rotary encoder inputs
volatile uint8_t low; // used to store the LOW setting
volatile uint8_t high; // used to store the HIGH setting
int f_integer; // used to store integer portion of temperature

// Corresponds to the PIN on the appropriate register
const char RED = 3;
const char GREEN = 4;
const char BLUE = 5;
const char LOW = 2;
const char HIGH = 3;
const char ROT1 = 1;
const char ROT2 = 2;
const char LOW_SETTING = 1;
const char HIGH_SETTING = 2;

int main(void) {
	const char LEDS = (1 << RED) | (1 << GREEN) | (1 << BLUE);
	const char ROT_ENC = (1 << ROT1) | (1 << ROT2);
	const char BUTTONS = (1 << LOW) | (1 << HIGH);

	unsigned char t[2]; // used when retrieving temperature from DS18B20
	char display_temp[6]; // used to display the temperature
	char display_low[3]; // used to display the LOW setting
	char display_high[3]; // used to display the HIGH setting
	char off_by_3 = 0; // used to indicate whether the temperature if +/- 3 degrees from HIGH/LOW setting
	int new_ocr = 0; // used to store the new OCR value for the servo


	low = eeprom_read_byte((void *) 1); // stored in address 1
	if (low < 50 || low > 90) {
		low = 70; // default LOW setting
		eeprom_update_byte((void *) 1, low);
	}

    high = eeprom_read_byte((void *) 2); // stored in address 2
	
	if (high < 50 || high > 90) {
		high = 85; // default HIGH setting
		eeprom_update_byte((void *) 2, high);
	}


	int16_t c_16; // used to store the temperature from the DS18B20
	int f_whole; // used to store the temperature in Fahrenheit
	int f_decimal; // used to store the decimal portion of the temperature in Fahrenheit

	// Initialize the DDR and PORT registers and LCD
	DDRB |= (1 << PB3) | (1 << PB4) | (1 << PB5);
	PORTB |= (1 << PB3);
	DDRC |= LEDS; // Configure the RGB LED pins as outputs
	PORTC |= LEDS; // All LEDs off
	PORTC |= ROT_ENC; // Pull-up resistors for the rotary encoder
	PORTD |= BUTTONS; // Pull-up resistors for the 2 buttons
	lcd_init();

	// Enable local interrupts
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT9) | (1 << PCINT10); // For pin-change interrupts (rotary encoder)
	
	// Enable global interrupts
	sei();

	// Initialize the timers
	timer0_init();
	timer1_init();
	timer2_init();


    // Write a splash screen to the LCD
	lcd_writecommand(1);
    char splash[14];
    snprintf(splash, 14, "%s", "EE109 Project");
    lcd_moveto(0, 0);
    lcd_stringout(splash);
    char name[5];
    snprintf(name, 5, "%s", "Pari");
    lcd_moveto(1, 0);
    lcd_stringout(name);
    _delay_ms(1000);
	

	// Prints initial temperature (which is blank before the while loop)
	lcd_writecommand(1);
	lcd_moveto(0,0);
	lcd_stringout("Temp: ----");

	// Prints default LOW and HIGH settings
	lcd_moveto(1, 0);
	lcd_stringout("Low= ");
	snprintf(display_low, 3, "%2d", low);
	lcd_stringout(display_low);
	lcd_moveto(1, 8);
	lcd_stringout("High= ");
	snprintf(display_high, 3, "%2d", high);
	lcd_stringout(display_high);

	x = PINC; // captures the PINC register
	a = (x & (1 << ROT1)) >> ROT1; // Stores the value as a 0 or a 1
	b = (x & (1 << ROT2)) >> ROT2; // Stores the value as a 0 or a 1

	// Determines the initial state of the encoder
    if (!b && !a) {
		old_state = 0;
	}
   
    else if (!b && a) {
		old_state = 1;
	}
    
	else if (b && !a) {
		old_state = 2;
	}
    
	else {
		old_state = 3;
	}

    new_state = old_state;
	

	if (ds_init() == 0) {    // Initialize the DS18B20
		// Sensor not responding
	}

	ds_convert();    // Start first temperature conversion

	while (1) {

		/* HANDLES THE TEMPERATURE READING/CONVERSIONS */

		if (ds_temp(t)) {    // True if conversion complete
			/*
			Process the values returned in t[0]
			and t[1] to find the temperature.
			*/
			lcd_moveto(0,5);
			c_16 = (t[1] << 8) + t[0]; // shift t[1] to the right by 8 bits
			f_whole = (9 * c_16) / 5 + 32*16; // temperature in fahrenheit
			f_integer = f_whole / 16; // integer portion
			f_decimal = f_whole % 16; // decimal portion
			
			// the initial f_decimal ÷ 16 is the actual decimal portion; round down to the nearest tenth
			f_decimal = (f_decimal * 10) / 16;
		
			snprintf(display_temp, 6, "%3d.%d", f_integer, f_decimal);
			lcd_stringout(display_temp); // prints the current temperature


			ds_convert();   // Start next conversion
		}



		// Default servo motor position
		if (!servo_state) { // servo points to the current temperature (not in HIGH or LOW setting)
			new_ocr = (-4*f_integer)/10 + 51;
			OCR2A = new_ocr;
		}




		/* HANDLES THE BUTTON PRESSES */

		// LOW setting is pressed
		if ((PIND & (1 << LOW)) == 0) {
			control_state = LOW_SETTING;
			count = low;
			lcd_moveto(1, 3);
			lcd_writedata('?');
			lcd_moveto(1, 12);
			lcd_writedata('=');
			
			// servo initially points to the LOW temperature
			servo_state = 1;
			new_ocr = (-4*low)/10 + 51;
			OCR2A = new_ocr;

			// start the 4 second timer
			TCCR1B |= (1 << CS10) | (1 << CS12);
		}

		// HIGH setting is pressed
		else if ((PIND & (1 << HIGH)) == 0) {
			control_state = HIGH_SETTING;
			count = high;
			lcd_moveto(1, 3);
			lcd_writedata('=');
			lcd_moveto(1, 12);
			lcd_writedata('?');

			// servo initially points to the HIGH temperature
			servo_state = 1;
			new_ocr = (-4*high)/10 + 51;
			OCR2A = new_ocr;

			// start the 4 second timer
			TCCR1B |= (1 << CS10) | (1 << CS12);
		}




		/* HANDLES THE LOW/HIGH SETTINGS */
		
		// Low state
		if (control_state == LOW_SETTING) {
			if (changed) { // Did state change?
				changed = 0;        // Reset changed flag
				lcd_moveto(1, 5);
				if (count <= high) { // LOW setting must be <= the HIGH setting
					
					// saves the LOW setting
					low = count;
					eeprom_update_byte((void *) 1, low);

					// 4 seconds have not elapsed yet -> servo should point to adjusted temperature
					if (servo_state) {
						new_ocr = (-4*low)/10 + 51;
						OCR2A = new_ocr;
					}

					// prints the LOW setting
					snprintf(display_low, 3, "%2d", low);
					lcd_stringout(display_low);
				}
			}
		}

		// High state
		else if (control_state == HIGH_SETTING) {
		
			if (changed) { // Did state change?
				changed = 0;        // Reset changed flag
				lcd_moveto(1, 14);
				if (count >= low) { // HIGH setting must be >= the LOW setting
					
					// saves the HIGH setting
					high = count;
					eeprom_update_byte((void *) 2, high);
					
					// 4 seconds have not elapsed yet -> servo should point to adjusted temperature
					if (servo_state) {
						new_ocr = (-4*high)/10 + 51;
						OCR2A = new_ocr;
					}

					// prints the HIGH setting
					snprintf(display_high, 3, "%2d", high);
					lcd_stringout(display_high);
				}
			}
		}



		/* HANDLES RGB LED */

		// Temperature is too cold
		if (f_integer < low) { // flash RED (heat up)
			PORTC &= ~(1 << RED); // connects only the RED cathode to ground
			PORTC |= (1 << BLUE) | (1 << GREEN);
		}

		// Temperature is too hot
		else if (f_integer > high) { // flash BLUE (cool down)
			PORTC &= ~(1 << BLUE); // connects only the blue cathode to ground
			PORTC |= (1 << RED) | (1 << GREEN);
		}

		// Temperature is in the desired range
		else { // flash GREEN
			PORTC &= ~(1 << GREEN); // connects only the green cathode to ground
			PORTC |= (1 << RED) | (1 << BLUE);
		}



		/* HANDLES THE BUZZER */

		// Determines whether the temperature is off by 3
		off_by_3 = (f_integer > 3 + high) || (f_integer < low - 3);

		// The temperature is off by 3, and the buzzer hasn't sounded yet -> sound the buzzer only ONCE
		if (!sounded && off_by_3) {
			TCCR0B |= (1 << CS00) | (1 << CS01); // turns on timer
			sounded = 1; // buzzer has sounded
		}

		// Buzzer sounds again ONLY ONCE the temperature is back in the desired range
		else if (sounded && !off_by_3) {
			//TCCR0B |= (1 << CS00) | (1 << CS01); // turns on timer
			sounded = 0; // reset: buzzer has not sounded
		}

	}

	return 0;
}




/* THE THREE ISRs ARE BELOW */


ISR(PCINT1_vect)
{
	// Read the input bits and determine A and B.

	x = PINC;
	a = (x & (1 << ROT1)) >> ROT1;
	b = (x & (1 << ROT2)) >> ROT2;


	if (old_state == 0) {

	    // Handle A and B inputs for state 0

		if (!b && a) {
			new_state = 1;

			// Temperature must be <= 90
			if (count < 90) {
				count++;
			}
		}

		else if (b && !a) {
			new_state = 2;
			
			// Temperature must be >= 50
			if (count > 50) {
				count--;
			}
		}

	}
	else if (old_state == 1) {

	    // Handle A and B inputs for state 1

		if (!b && !a) {
			new_state = 0;
			// Temperature must be >= 50
			if (count > 50) {
				count--;
			}
		}

		else if (b && a) {
			new_state = 3;
			
			// Temperature must be <= 90
			if (count < 90) {
				count++;
			}
		}

	}
	else if (old_state == 2) {

	    // Handle A and B inputs for state 2

		if (!b && !a) {
			new_state = 0;

			// Temperature must be <= 90
			if (count < 90) {
				count++;
			}
		}

		else if (b && a) {
			new_state = 3;
			// Temperature must be >= 50
			if (count > 50) {
				count--;
			}
		}


	}
	else {   // old_state = 3

	    // Handle A and B inputs for state 3

		if (!b && a) {
			new_state = 1;
			// Temperature must be >= 50
			if (count > 50) {
				count--;
			}
		}

		else if (b && !a) {
			new_state = 2;

			// Temperature must be <= 90
			if (count < 90) {
				count++;
			}
		}

	}

	// If state changed, update the value of old_state,
	// and set a flag that the state has changed.

	if (new_state != old_state) {
	    changed = 1;
	    old_state = new_state;
	}

}



ISR(TIMER0_COMPA_vect)
{
    // Change the output bit of the buzzer, and turn off the timer after enough periods of the signal

	PORTB ^= (1 << PB5); // Toggle the output bit of the buzzer

	count_timer++;

	if (count_timer == cycles) { // timer has reached 1000
		TCCR0B &= ~((1 << CS00) | (1 << CS01)); // Clear the prescalar
		count_timer = 0;
	}

}

// Used for the 4 second delay for the servo motor during LOW/HIGH settings
ISR(TIMER1_COMPA_vect)
{
	// Clear the prescalar
	TCCR1B &= ~(1 << CS10);
	TCCR1B &= ~(1 << CS12);
	servo_state = 0; // Servo should now go back and point to the original temperature
}
