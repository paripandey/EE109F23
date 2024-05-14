/********************************************
*
*  Name: Pari Pandey
*  Email: paripand@usc.edu
*  Lab section: Wednesday 2 PM
*  Assignment: Lab 3 - Arduino Input and Output
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>


#define DOT_LENGTH  250         /* Define the length of a "dot" time in msec */

void dot(void);
void dash(void);
void makeOutput(char);
char checkInput(char);

int main(void)
{
    // Initialize appropriate DDR registers
    DDRD |= 1 << 2;
    DDRB &= 0x00;

    // Initialize the LED output to 0
    // (this is already done by default)
	
    // Enable the pull-up resistors for the 
    // 3 button inputs 
    PORTB |= ((1 << 3) | (1 << 4) | (1 << 5));
	
    // Loop forever
    while (1) {                 
    
	//  Use "if" statements and the checkInput()
	//  function to determine if a button
	//  is being pressed and then output
	//  the correct dot/dash sequence by
	//  calling the dot() and dash(), and 
	//  using appropriate delay functions
      
      // Button for the 'U' character
      if (checkInput(3) != 0) {
        dot();
        _delay_ms(DOT_LENGTH);
        dot();
        _delay_ms(DOT_LENGTH);
        dash();
        _delay_ms(3*DOT_LENGTH); // delay in between 2 characters
      }

      // Button for the 'S' character
      if (checkInput(4) != 0) {
        dot();
        _delay_ms(DOT_LENGTH);
        dot();
        _delay_ms(DOT_LENGTH);
        dot();
         _delay_ms(3*DOT_LENGTH); // delay in between 2 characters
      }

      // Button for the 'C' character
      if (checkInput(5) != 0) {
        dash();
        _delay_ms(DOT_LENGTH);
        dot();
        _delay_ms(DOT_LENGTH);
        dash();
        _delay_ms(DOT_LENGTH);
        dot();
        _delay_ms(3*DOT_LENGTH); // delay in between 2 characters
      }
    }

    return 0;   /* never reached */
}

/*
  dot() - Makes the output LED blink a "dot".

  Write code to generate a dot by using the makeOutput function
  to turn the output ON and OFF with appropriate delays.
  Be sure you don't forget to also delay an
  appropriate time after you turn the output off.
*/
void dot()
{
  makeOutput(1);
  _delay_ms(DOT_LENGTH);
  makeOutput(0);
  // not including the final delay in here
}

/*
  dash() - Makes the output LED blink a "dash".

  Write code to generate a dash by using the makeOutput function
  to turn the output ON and OFF with appropriate delays.
  Be sure you don't forget to also delay an
  appropriate time after you turn the output off.
*/
void dash()
{
  makeOutput(1);
  _delay_ms(3*DOT_LENGTH);
  makeOutput(0);
  // not including the final delay in here
}

/*
  makeOutput() - Changes the output bit (Group D, bit 2) to either
  a zero or one, based on the input argument "value".
  
  If the argument is zero, turn the output OFF,
  otherwise turn the output ON. 
  
  Do not use any delays here.  Just use bit-wise operations
  to make the appropriate PORT bit turn on or off.
*/
void makeOutput(char value)
{
  if (value) {
    PORTD |= (1 << 2);

  }

  else {
    PORTD &= ~(1 << 2);
  }

}


/*
  checkInput(bit) - Checks the state of the input bit in Group B specified by
  the "bit" argument (0-7), and returns 1 if the button is pressed, or 0 if
  the button is not pressed.
  
  Write code to use the appropriate group's PIN register and determine if  
  the specified bit (which is passed as the argument) of that group is
  pressed or not.  Think carefully about what bit value means "pressed"
  when connected to a pushbutton.
 */
char checkInput(char bit)
{
  //If the button is down, the function returns a 1.
  // If the button is not pressed it returns a 0.
  // When writing the function make sure to think about
  // what value the bit in the PINB register will be when a button is pressed.

  // if the button is pressed
  if ((PINB & (1 << bit)) == 0) {
    return 1;
  }

  // if the button is not pressed
  else {
    return 0;
  }

}
