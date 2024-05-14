/********************************************
 *
 *  Name: Pari Pandey
 *  Email: paripand@usc.edu
 *  Section: Wednesday 2 PM Lab
 *  Assignment: Lab 5 - Analog-to-digital conversion
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "lcd.h"
#include "adc.h"
#include <stdio.h>//

void rand_init(void);

int main(void)
{

    // Initialize the LCD
    lcd_init();

    // CHECKPOINT
    /*lcd_stringout("PARI");
    char month = 8;
    char day = 1;
    char year = 4;
    char birthday[9];
    snprintf(birthday, 9, "%02d/%02d/%02d",  month, day, year);
    lcd_moveto(1, 4);
    lcd_stringout(birthday);
    _delay_ms(1000);*/


    // Initialize the ADC
    adc_init();

    // Initialize the random number function
    rand_init();

    // Write splash screen and delay for 1 second

    lcd_writecommand(1); // Clear the LCD

    // Write the first line ("EE109 Lab 5") to the LCD
    char buf1[12];
    snprintf(buf1, 12, "%s", "EE109 Lab 5");
    lcd_moveto(0, 0);
    lcd_stringout(buf1);

    // Write the second line to the LCD
    char buf2[5];
    snprintf(buf2, 5, "%s", "Pari");
    lcd_moveto(1, 0); // move to the next row
    lcd_stringout(buf2);
    
    // Delay for 1 second
    _delay_ms(1000);

    // Find three random numbers and display on top line
    
    lcd_writecommand(1); // clears the LCD

    // Generates 3 random numbers
    char num1 = rand() % 32;
    char num2 = rand() % 32;
    char num3 = rand() % 32;

    // Initializes the location/column of the cursor
    char cursor_column = 0;

    // Will store the number of correct inputs to the combination lock
    char correct = 0;

    // Converts the 3 numbers to a string and prints them on the LCD
    char num1_[4], num2_[4], num3_[4];
    snprintf(num1_, 4, "%3d", num1);
    snprintf(num2_, 4, "%3d", num2);
    snprintf(num3_, 4, "%3d", num3);
    lcd_moveto(0, 0);
    lcd_stringout(num1_);
    lcd_stringout(num2_);
    lcd_stringout(num3_);

    // Show the number selector ('>') on the bottom line
    lcd_moveto(1, 0);
    lcd_stringout(">");

    while (1) {                 // Loop forever
        uint8_t adc_result = adc_sample(0);

	// Convert ADC channel for buttons to 0-255
    
        // If you click "LEFT" ("true" value is 156)
        if (adc_result >= 154 && adc_result <= 158) {
            // The cursor can't move "any more left" than the 1st input
            if (cursor_column > 0) {
                lcd_moveto(1, cursor_column);
                lcd_stringout(" "); // "clears" the cursor
                cursor_column -= 3;
                lcd_moveto(1, cursor_column); // moves to the previous input
                lcd_stringout(">");
            }
        }

        // If you click "RIGHT" ("true" value is 0)
        else if (adc_result >= 0 && adc_result <= 2) {
            // The cursor can't move "any more right" than the 3rd input
            if (cursor_column < 6) {
                lcd_moveto(1, cursor_column);
                lcd_stringout(" "); // "clears" the cursor
                cursor_column += 3; // moves to the next input
                lcd_moveto(1, cursor_column);
                lcd_stringout(">");
            }
        }

        // Read potentiometer ADC channel
        uint8_t pot_result = adc_sample(3)/8;
    

	// Convert ADC channel for pote/ntiometer to 0-255, change to 0-31 and display
        
        // Conversion is done in the step above
        char buf3[3];
        snprintf(buf3, 3, "%2d", pot_result);
        lcd_moveto(1, 14);
        lcd_stringout(buf3); // prints the current input to the LCD

	// If select button pressed copy number to one of the lock inputs positions

        // ("true" value is 204)
        if (adc_result >= 202 && adc_result <= 206) {
            lcd_moveto(1, cursor_column+1);
            lcd_stringout(buf3); // "copies" the input into the selection

            // Check to see if all three lock inputs match the combination
            
            // If the 1st number is right
            if ((cursor_column + 1 == 1) && (pot_result == num1)) {
                correct++;
            }

            // If the 2nd number is right
            else if ((cursor_column + 1 == 4) && (pot_result == num2)) {
                correct++;
            }

            // If the 3rd number is right
            else if ((cursor_column + 1 == 7) && (pot_result == num3)) {
                correct++;
            }
            
            // If all the inputs are right, then the lock is "Unlocked"
            if (correct == 3) {
                lcd_moveto(0, 10);
                lcd_stringout("Unlock");
            }
        }

        // Delay by 100ms
        _delay_ms(100);
	
    }

    return 0;   /* never reached */
}

void rand_init()
{
    int16_t seed = 0;
    uint8_t i, j, x;

    // Build a 15-bit number from the LSBs of an ADC
    // conversion of the channels 1-5, 3 times each
    for (i = 0; i < 3; i++) {
	for (j = 1; j < 6; j++) {
	    x = adc_sample(j);
	    x &= 1;	// Get the LSB of the result
	    seed = (seed << 1) + x; // Build up the 15-bit result
	}
    }
    srand(seed);	// Seed the rand function
}
