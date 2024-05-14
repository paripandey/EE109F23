#include <avr/io.h>

#include "adc.h"


void adc_init(void)
{
    // Initialize the ADC
    ADMUX |= ((1 << REFS0) | (1 << ADLAR));
    ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
}

uint8_t adc_sample(uint8_t channel)
{
    // Set ADC input mux bits to 'channel' value
    
    // copy from channel in bits to the MUX
    char MASKBITS = 0x0f;
    ADMUX &= ~MASKBITS;
    ADMUX |= (channel & MASKBITS);
    ADCSRA |= (1 << ADSC);

    // Convert an analog input and return the 8-bit result
    while (ADCSRA & (1 << ADSC));

    char result = ADCH;
    return result;
}
