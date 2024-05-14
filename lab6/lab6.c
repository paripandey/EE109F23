/********************************************
 *
 *  Name: Pari Pandey
 *  Email: paripand@usc.edu
 *  Section: Wednesday 2 PM Lab
 *  Assignment: Lab 6 - Timers
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include <stdio.h>

void debounce(uint8_t);

enum states { STARTRUN, PAUSE, RUN, LAPPED };
const char START_STOP = 2; // for easier reference
const char LAP_RESET = 4; // for easier reference
volatile char tens = 0; // will store the tens digit in timer
volatile char ones = 0; // will store the ones digit in timer
volatile char tenths = 0; // will store the tenths digit in timer
volatile char ones_changed = 0; // tracks whether the ones digit changed
volatile char tens_changed = 0; // tracks whether the tens digit changed


int main(void) {

    uint8_t state;

    // Initialize the LCD and TIMER1
    lcd_init();
    timer1_init();


    // Enable pull-ups for buttons
    PORTC |= (1 << START_STOP) | (1 << LAP_RESET);

    // Show the splash screen
    lcd_writecommand(1);
    char buf1[12];
    snprintf(buf1, 12, "%s", "EE109 Lab 6");
    lcd_moveto(0, 0);
    lcd_stringout(buf1);
    char buf2[5];
    snprintf(buf2, 5, "%s", "Pari");
    lcd_moveto(1, 0);
    lcd_stringout(buf2);
    _delay_ms(1000);

    state = PAUSE; // starts in the PAUSE state (by default)
    
    // initializes the timer
    lcd_writecommand(1); // clears the LCD
    lcd_moveto(0,1);
    lcd_writedata('0');
    lcd_writedata('.');
    lcd_writedata('0');

    // Enable interrupts
    sei();

    while (1) {                 // Loop forever
            
    // Read the buttons

        /* NEXT STATE LOGIC*/

        if (state == PAUSE) { // PAUSE state

            // START_STOP WAS PRESSED: START/RESUME THE TIMER
            if ((PINC & (1 << START_STOP)) == 0) {
                state = STARTRUN; // starts timer immediately, even if button is not released
            }

            // LAP_RESET WAS PRESSED: RESET THE TIMER
            else if ((PINC & (1 << LAP_RESET)) == 0) {
                debounce(LAP_RESET);
                
                // Resets the timer to 0.0
                tens = 0;
                ones = 0;
                tenths = 0;
                lcd_moveto(0,0);
                lcd_writedata(' ');
                lcd_writedata('0');
                lcd_writedata('.');
                lcd_writedata('0');
            }
        }

        else if (state == RUN) { // RUN state
            // START_STOP WAS PRESSED: PAUSE THE TIMER
            if ((PINC & (1 << START_STOP)) == 0) {
                debounce(START_STOP);
                state = PAUSE;
            }

            // LAP_RESET WAS PRESSED: LAP THE TIMER
            else if ((PINC & (1 << LAP_RESET)) == 0) {
                debounce(LAP_RESET);
                state = LAPPED;
            }   
        }

        else if (state == LAPPED) { // LAPPED state
            // START_STOP WAS PRESSED: START/RESUME THE TIMER
            if ((PINC & (1 << START_STOP)) == 0) {
                debounce(START_STOP);
                state = RUN;
            }

            // LAP_RESET WAS PRESSED: ALSO START/RESUME THE TIMER
            else if ((PINC & (1 << LAP_RESET)) == 0) {
                debounce(LAP_RESET);
                state = RUN;
            }
        }

        /* CURRENT STATE LOGIC */

        if (state == STARTRUN) {
            TCCR1B |= (1 << CS12);
            _delay_ms(5);

            // while the button is pressed
            while ((PINC & (1 << START_STOP)) == 0) {
                // pretty much the same behavior as the RUN state

                // if the tens digit has changed, then print it; why reprint on the display unnecessarily
                if (tens_changed) {
                lcd_moveto(0, 0);

                    if (!tens) { // if the tens digit is 0, print nothing
                        lcd_writedata(' ');
                    }

                    else { // otherwise, print the tens digit
                        lcd_writedata(tens + '0');
                    }
                }

                // if the ones digit has changed, then print it; why reprint on the display unnecessarily
                if (ones_changed) {
                    lcd_moveto(0, 1);
                    lcd_writedata(ones + '0');
                }

                // prints the tenths digit, don't reprint the "."
                lcd_moveto(0, 3);
                lcd_writedata(tenths + '0');
            }
            _delay_ms(5);
            state = RUN; // officially moves to RUN state, as soon as the button is released
        }
        
        else if (state == RUN) {
            // same logic as in STARTRUN state
            if (tens_changed) {
                lcd_moveto(0, 0);

                if (!tens) {
                    lcd_writedata(' ');
                }

                else {
                    lcd_writedata(tens + '0');
                }
            }

            if (ones_changed) {
                lcd_moveto(0, 1);
                lcd_writedata(ones + '0');
            }

            lcd_moveto(0, 3);
            lcd_writedata(tenths + '0');
        }

        else if (state == PAUSE) {
            // turn off the prescalar
            TCCR1B &= ~(1 << CS12);
        }

    }
    
    return 0;   /* never reached */
}

/* ----------------------------------------------------------------------- */

void debounce(uint8_t bit)
{
    // Add code to debounce input "bit" of PINC
    // assuming we have sensed the start of a press.

    // same code as in lecture slides
    _delay_ms(5);
    while((PINC & (1 << bit)) == 0){}
    _delay_ms(5);
}

/* ----------------------------------------------------------------------- */

void timer1_init(void)
{
    // Add code to configure TIMER1 by setting or clearing bits in
    // the TIMER1 registers.

    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 6250; // used prescalar of 256, so 16 * 10^6 / 256 = 6250
    TCCR1B |= (1 << CS12);

}

ISR(TIMER1_COMPA_vect)
{
    if (tenths < 9) {
        tenths++;
        tens_changed = 0;
        ones_changed = 0;
    }

    else { // wraps around
        tenths = 0;
        ones_changed = 1; // ones digit is updated
        if (ones < 9) {
            ones++;
        }
        else {
            ones = 0; // wraps around
            tens_changed = 1; // tens digit is updated
            if (tens < 5) {
                tens++;
            }

            // wraps around, resets to 0.0
            else {
                tens = 0;
            }
        }
    }

}


