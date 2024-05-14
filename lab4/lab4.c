/********************************************
*
*  Name: Pari Pandey
*  Email: paripand@usc.edu
*  Section: Wednesday 2 PM Lab
*  Assignment: Lab 4 - Up/Down counter on LCD display
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

enum states { UP, DOWN, PAUSE };

int main(void) {

    // Setup DDR and PORT bits for the 3 input buttons as necessary
    DDRC &= ~((1 << PC2) | (1 << PC4)); 
    DDRB &= ~(1 << 3);
    // Enabling pull-up resistors for the buttons
    PORTC |= ((1 << 2) | (1 << 4));
    PORTB |= (1 << 3);

    // Initialize the LCD
    lcd_init();


    // Use a state machine approach to organize your code
    //   - Declare and initialize a variable to track what state you
    //     are in by assigning the values UP, DOWN or PAUSE to that variable.

    char my_counter_state = UP; // stores the current state
    char count = 0; // stores the number to be displayed on the screen
    char new_value = 0; // will store the next number to be printed
    char counter = 0; // will help track the delays between each number 

    while (1) {               // Loop forever

        // Use "if" statements to read the buttons and determine which
        // state you are in
        
            if (my_counter_state == UP) {
                if ((PINC & (1 << 4)) == 0) {
                    my_counter_state = DOWN;
                }

                else if ((PINB & (1 << 3)) == 0) {
                    my_counter_state = PAUSE;
                }
            }

            else if (my_counter_state == DOWN) {
                if ((PINC & (1 << 2)) == 0) {
                    my_counter_state = UP;
                }

                else if ((PINB & (1 << 3)) == 0) {
                    my_counter_state = PAUSE;
                }
            }


            else if (my_counter_state == PAUSE) {
                if ((PINC & (1 << 2)) == 0) {
                    my_counter_state = UP;
                }

                else if ((PINC & (1 << 4)) == 0) {
                    my_counter_state = DOWN;
                }
            }

            new_value = count;

            if (my_counter_state == UP) {
                // Determines the next value by incrementing count's value by 1
                if (new_value < 9) { new_value++; }
                // If count has reached 9, wrap around and "reset" it to 0
                else { new_value = 0; }
            }

            else if (my_counter_state == DOWN) {
                // Determines the next value by decrementing count's value by 1
                if (new_value > 0) { new_value--; }
                // If count has reached 0, wrap around and "reset" it to 9
                else { new_value = 9; }
            }

            // This means 500 ms have passed
            if (counter == 10) {
                // Clears the LCD
                lcd_writecommand(1);
                // Prints count to the LCD
                lcd_writedata(count + '0');
                // Updates count with the next value in the sequence
                count = new_value;
                // Resets the counter to store the next delay
                counter = 0;
            }

            // Delay by 50 ms for each iteration
            _delay_ms(50);
            // Increments the counter to track this delay
            counter++;
    }

    return 0;   /* never reached */
}
