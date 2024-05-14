/********************************************
 *
 *  Name: Pari Pandey
 *  Email: paripand@usc.edu
 *  Section: Wednesday 2 PM Lab
 *  Assignment: Lab 9 - Hardware components
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "adc.h"

void timer1_init(void);
void shift_load(uint8_t, uint8_t);
void shift1bit(uint8_t);

#define BUTTON_CHAN 0

// Values returned by the DFRobot LCD shield
#define ADC_RIGHT    0
#define ADC_UP      51
#define ADC_DOWN   101
#define ADC_LEFT   156
#define ADC_SELECT 204

volatile char changed = 0;
volatile char x;

int main(void)
{
    uint8_t adc_result;
    uint8_t right_button, left_button, up_button, down_button;
    uint8_t color, level, update;

    // Initialize the LCD
	lcd_init();


    // Initialize the ADC
	adc_init();


    // Set DDR bits for output to the counter and shift register
	DDRB |= (1 << PB4);
	DDRC |= ((1 << PC3) | (1 << PC5)); // PC3 = data, PC5 = clock signal

    // Initialize the timer
	timer1_init();


    // Enable interrupts
	sei();

    // Write splash screen
	lcd_writecommand(1);
    char buf1[12];
    snprintf(buf1, 12, "%s", "EE109 Lab 9");
    lcd_moveto(0, 0);
    lcd_stringout(buf1);
    char buf2[5];
    snprintf(buf2, 5, "%s", "Pari");
    lcd_moveto(1, 0);
    lcd_stringout(buf2);
    _delay_ms(1000);

    // Write the LED level and color screen
    lcd_moveto(0, 0);
    lcd_stringout("Level: >D  M  B");
    lcd_moveto(1, 0);
    lcd_stringout("Color: >R  G  B");

    color = 0;
    level=0;
    update = 1;

	lcd_writecommand(1);

    while (1) {                 // Loop forever
		adc_result = adc_sample(BUTTON_CHAN);

		// Left and right buttons select which color LED to light up
		right_button = adc_result < ADC_RIGHT+20;
		left_button = (adc_result > ADC_LEFT-20) && (adc_result < ADC_LEFT+20);

		// Move the indicator around
		if (right_button && (color < 2)) {
			lcd_moveto(1, color * 3 + 7);
			lcd_writedata(' ');
			color++;
			lcd_moveto(1, color * 3 + 7);
			lcd_writedata('>');
			update = 1;
			_delay_ms(200);
		}

		else if (left_button && (color > 0)) {
			lcd_moveto(1, color * 3 + 7);
			lcd_writedata(' ');
			color--;
			lcd_moveto(1, color * 3 + 7);
			lcd_writedata('>');
			update = 1;
			_delay_ms(200);
		}

		// Up and downbuttons select which PWM level to use
		up_button = (adc_result > ADC_UP-20) && (adc_result < ADC_UP+20);
		down_button = (adc_result > ADC_DOWN-20) && (adc_result < ADC_DOWN+20);

		// Move the indicator around
		if (up_button && (level < 2)) {
			lcd_moveto(0, level * 3 + 7);
			lcd_writedata(' ');
			level++;
			lcd_moveto(0, level * 3 + 7);
			lcd_writedata('>');
			update = 1;
			_delay_ms(200);
		}

		else if (down_button && (level > 0)) {
			lcd_moveto(0, level * 3 + 7);
			lcd_writedata(' ');
			level--;
			lcd_moveto(0, level * 3 + 7);
			lcd_writedata('>');
			update = 1;
			_delay_ms(200);
		}

		if (update) {
			shift_load(level, color);
			update = 0;
		}
    }

    return 0;   /* never reached */
}

void shift_load(uint8_t mux, uint8_t demux)
{
	shift1bit((demux & 2) >> 1); // AND MSB of demux with 0b10, shift to the right by 1
    shift1bit(demux & 1); // AND LSB of demux with 0b01
	shift1bit((mux & 2) >> 1); // AND MSB of mux with 0b10, shift to the right by 1
    shift1bit(mux & 1); // AND LSB of mux with 0b01
}

void shift1bit(uint8_t bit)
{
    // Write code to shift one bit into the shift register
	if (!bit) // the bit is 0
		PORTC &= ~(1 << PC3); // generate 0V on PC3

	else // the bit is non-zero
		PORTC |= (1 << PC3); // generate 5V on PC3
	
	// 0-1-0 clock signal on PC5
	PORTC &= ~(1 << PC5);
	PORTC |= (1 << PC5);
	PORTC &= ~(1 << PC5);
}

void timer1_init(void)
{
    TCCR1B |= (1 << WGM12);     // Set for CTC mode using OCR1A for the modulus
    TIMSK1 |= (1 << OCIE1A);    // Enable CTC interrupt
    // 16,000,000 * 0.0005 = 8,000
    OCR1A = 7999;              // Set the counter modulus correctly
    TCCR1B |= (0b001 << CS10);  // Set prescaler for /1, starts timer
}


ISR(TIMER1_COMPA_vect)
{
    PORTB ^= (1 << PB4);
}

