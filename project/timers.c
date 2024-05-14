#include "timers.h"
#include <avr/io.h>

void timer0_init() // 1 to 0
{
	// Used to the buzzer

	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = 250;
}

void timer1_init(void)
{
	// Used for the 4-second delay

    TCCR1B |= (1 << WGM12);     // Set for CTC mode using OCR1A for the modulus
    TIMSK1 |= (1 << OCIE1A);    // Enable CTC interrupt
    OCR1A = 62500;              // Set the counter modulus correctly (prescalar: 1024)
	TCCR1B &= ~(1 << CS10); // Clear the prescalar
	TCCR1B &= ~(1 << CS12);
}

void timer2_init(void)
{
	// Used for the servo motor
	TCCR2A |= (0b11 << WGM20);  // Fast PWM mode, modulus = 256
	TCCR2A |= (0b10 << COM2A0); // Turn D11 on at 0x00 and off at OCR2A
	OCR2A = 35;          // default/starting position is to the left      
	TCCR2B |= (0b111 << CS20);  // Prescaler = 1024 for 16ms period
}