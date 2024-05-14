/********************************************
 *
 *  Name: Pari Pandey
 *  Email: paripand@usc.edu
 *  Section: Wednesday 2 PM Lab
 *  Assignment: Lab 8 - Pulse Width Modulation
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"

volatile uint8_t new_state, old_state;
volatile uint8_t changed = 0;  // Flag for state change
volatile int16_t count = 0;		// Count to display

volatile char x; // Will store the signals from the PINC register

volatile uint8_t a, b; // Will store the signals from the rotary encoder

volatile PWM_width = 1500; // Minimum pulse width for 0.75msec

int main(void) {
    
    // Initialize DDR and PORT registers and LCD
	DDRB |= ((1 << PB3) | (1 << PB2));
	PORTC |= ((1 << PC4) | (1 << PC5));

	PINC = 128;
	
	//lcd_init();
	timer1_init();
	timer2_init();

	// Enable local interrupts
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT12) | (1 << PCINT13);
	
	// Enable global interrupts
	sei();

	// I have commented out all the LCD function calls, etc.
    // Write a splash screen to the LCD
	/*lcd_writecommand(1);
    char buf1[12];
    snprintf(buf1, 12, "%s", "EE109 Lab 8");
    lcd_moveto(0, 0);
    lcd_stringout(buf1);
    char buf2[5];
    snprintf(buf2, 5, "%s", "Pari");
    lcd_moveto(1, 0);
    lcd_stringout(buf2);
    _delay_ms(1000);*/
	
    // Read the A and B inputs to determine the intial state.
    // In the state number, B is the MSB and A is the LSB.
    // Warning: Do NOT read A and B separately.  You should read BOTH inputs
    // at the same time, then determine the A and B values from that value.

	x = PINC;
	a = (x & (1 << PC4)) >> PC4; // Stores the value as a 0 or a 1
	b = (x & (1 << PC5)) >> PC5; // Stores the value as a 0 or a 1

	
	/*lcd_moveto(1, 0);
	char buf3[17];
	snprintf(buf3, 17, "%16d", count);
	lcd_stringout(buf3);*/

	if (!a)
		OCR2A = 51; // 20% duty cycle

	else
		OCR2A = 204; // 80% duty cycle

    if (!b && !a) // 00 
	old_state = 0;
    else if (!b && a) // 01
	old_state = 1;
    else if (b && !a) // 10
	old_state = 2;
    else // 11
	old_state = 3;

    new_state = old_state;

    while (1) {                 // Loop forever

		if (changed) { // Did state change?
			changed = 0;        // Reset changed flag

			// Output count to LCD
			/*char buf3[17];
			lcd_moveto(1, 0);
			snprintf(buf3, 17, "%16d", count);
			lcd_stringout(buf3);*/

			OCR2A = count; // Copies new count value into OCR2A register
			OCR1B = PWM_width; // Copies the new PWM width into the OCR1B register

		}
    }
}


ISR(PCINT1_vect)
{

	// Read the input bits and determine A and B.

	x = PINC;
	a = (x & (1 << PC4)) >> PC4;
	b = (x & (1 << PC5)) >> PC5;

	// Now add code to this ISR to limit the count value to stay between 0 and 255. This number has to fit in the eight bit OCR2A register.

	if (old_state == 0) {

	// Handle A and B inputs for state 0

		if (!b && a) {
			new_state = 1;
			if (count < 255) {
				count++;
			}

			if (PWM_width < 4500) // Maximum pulse width
				PWM_width+=10;
		}

		else if (b && !a) {
			new_state = 2;
			if (count > 0) {
				count--;
			}

			if (PWM_width > 1500) // Minimum pulse width
				PWM_width-=10;
		}
	}

	else if (old_state == 1) {

		// Handle A and B inputs for state 1

		if (!b && !a) {
			new_state = 0;
			if (count > 0) {
				count--;
			}

			if (PWM_width > 1500)
				PWM_width-=10;
		}

		else if (b && a) {
			new_state = 3;
			if (count < 255) {
				count++;
			}

			if (PWM_width < 4500)
				PWM_width+=10;
		}

	}
	else if (old_state == 2) {

		// Handle A and B inputs for state 2

		if (!b && !a) {
			new_state = 0;
			if (count < 255) {
				count++;
			}

			if (PWM_width < 4500)
				PWM_width+=10;

		}

		else if (b && a) {
			new_state = 3;
			if (count > 0) {
				count--;
			}

			if (PWM_width > 1500)
				PWM_width-=10;
		}


	}
	else {   // old_state = 3

		// Handle A and B inputs for state 3

		if (!b && a) {
			new_state = 1;
			if (count > 0) {
				count--;
			}

			if (PWM_width > 1500)
				PWM_width-=10;

		}

		else if (b && !a) {
			new_state = 2;
			if (count < 255) {
				count++;
			}

			if (PWM_width < 4500)
				PWM_width+=10;
		}
	}

// If state changed, update the value of old_state,
// and set a flag that the state has changed.

	if (new_state != old_state) {
		changed = 1;
		old_state = new_state;
	}
}

void timer2_init(void)
{
	TCCR2A |= (0b11 << WGM20);  // Fast PWM mode, modulus = 256
	TCCR2A |= (0b10 << COM2A0); // Turn D11 on at 0x00 and off at OCR2A
	OCR2A = 128;                // Initial pulse duty cycle of 50%
	TCCR2B |= (0b111 << CS20);  // Prescaler = 1024 for 16ms period
}

void timer1_init()
{
    // In Task 7, add code to inititialize TIMER1, but don't start it counting
	TCCR1A |= ((1 << WGM10) | (1 << WGM11) | (1 << COM1B1));
	TCCR1B |= ((1 << WGM12) | (1 << WGM13) | (1 << CS11)); // prescalar of 8
	OCR1A = 40000;
	OCR1B = 3000; // Halfway between 1500 and 4500
}
