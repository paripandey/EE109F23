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

// 1500, 4500

void play_note(uint16_t);

volatile uint8_t new_state, old_state;
volatile uint8_t changed = 0;  // Flag for state change
volatile int16_t count = 0;		// Count to display

volatile char x; // will store the PINC register
volatile int16_t cycles = 0; // will be unique to each frequency
volatile int16_t count_timer = 0;

volatile uint8_t a, b;

int main(void) {
    
    // Initialize DDR and PORT registers and LCD
	DDRB |= (1 << PB3);
	PORTC |= ((1 << PC4) | (1 << PC5));

	/*
	At the beginning of the program, initialize the variable that is modified by the encoder
	to the initial value that was put in the OCR2A register in the timer2_init function.
	This will prevent the value from jumping around the first time the encoder state is changed.
	*/
	PINC = 128;
	
	//lcd_init();
	timer2_init();

	// Enable local interrupts
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT12) | (1 << PCINT13);
	
	// Enable global interrupts
	sei();

    // Write a splash screen to the LCD
	lcd_writecommand(1);
    char buf1[12];
    snprintf(buf1, 12, "%s", "EE109 Lab 8");
    lcd_moveto(0, 0);
    lcd_stringout(buf1);
    char buf2[5];
    snprintf(buf2, 5, "%s", "Pari");
    lcd_moveto(1, 0);
    lcd_stringout(buf2);
    _delay_ms(1000);
	
    // Read the A and B inputs to determine the intial state.
    // In the state number, B is the MSB and A is the LSB.
    // Warning: Do NOT read A and B separately.  You should read BOTH inputs
    // at the same time, then determine the A and B values from that value.

	x = PINC;
	a = (x & (1 << PC4)) >> PC4; // Stores the value as a 0 or a 1
	b = (x & (1 << PC5)) >> PC5; // Stores the value as a 0 or a 1

	
	lcd_moveto(1, 0);
	char buf3[17];
	snprintf(buf3, 17, "%16d", count);
	lcd_stringout(buf3);

	if (!a)
		OCR2A = 51;

	else
		OCR2A = 204;

    if (!b && !a)
	old_state = 0;
    else if (!b && a)
	old_state = 1;
    else if (b && !a)
	old_state = 2;
    else
	old_state = 3;

    new_state = old_state;

    while (1) {                 // Loop forever

		if (changed) { // Did state change?
			changed = 0;        // Reset changed flag

			// Output count to LCD
			char buf3[17];
			lcd_moveto(1, 0);
			snprintf(buf3, 17, "%16d", count);
			lcd_stringout(buf3);

			OCR2A = count;

		}
    }
}



/*
  Play a tone at the frequency specified for one second
*/
/*void play_note(uint16_t freq)
{
    cycles = 2 * freq;

	OCR1A = 16000000/(2 * freq);
    
	TCCR1B |= (1 << CS10);
	
}*/

/*
    variable_delay_us - Delay a variable number of microseconds
*/

/*void variable_delay_us(int delay)
{
    int i = (delay + 5) / 10;

    while (i--)
        _delay_us(10);
} */

ISR(PCINT1_vect)
{
    // In Task 6, add code to read the encoder inputs and determine the new
    // count value

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
		}

		else if (b && !a) {
			new_state = 2;
			if (count > 0) {
				count--;
			}
		}
	}

	else if (old_state == 1) {

		// Handle A and B inputs for state 1

		if (!b && !a) {
			new_state = 0;
			if (count > 0) {
				count--;
			}
		}

		else if (b && a) {
			new_state = 3;
			if (count < 255) {
				count++;
			}
		}

	}
	else if (old_state == 2) {

		// Handle A and B inputs for state 2

		if (!b && !a) {
			new_state = 0;
			if (count < 255) {
				count++;
			}
		}

		else if (b && a) {
			new_state = 3;
			if (count > 0) {
				count--;
			}
		}


	}
	else {   // old_state = 3

		// Handle A and B inputs for state 3

		if (!b && a) {
			new_state = 1;
			if (count > 0) {
				count--;
			}
		}

		else if (b && !a) {
			new_state = 2;
			if (count < 255) {
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
	TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);

}

ISR(TIMER1_COMPA_vect)
{
    // In Task 7, add code to change the output bit to the buzzer, and to turn
    // off the timer after enough periods of the signal

	PORTB ^= (1 << PB4); // Toggle the output bit of the buzzer

	count_timer++;

	if (count_timer == cycles) {
		TCCR1B &= ~(1 << CS10); // Clear the prescalar
		count_timer = 0;
	}
}
