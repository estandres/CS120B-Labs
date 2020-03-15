/* 
 * 	Partner: Steven Strickland sstri014@ucr.edu SID:862155853  
 * 	Lab Section: 22
 * 	Assignment: Lab 8 Exercise 2
 *
 *	Revise exercise 1 by replacing the potentiometer with a 
 *	photoresistor and 330 ohm resistor. Take note of the highest
 *	ADC value displayed (MAX) when the photoresistor is exposed 
 *	to light, and the lowest ADC value displayed (MIN) when the 
 *	photoresistor is deprived of all light. These values will be 
 *	used for the remaining lab exercises.
 *
 */

#include <avr/io.h>

void ADC_init()
{
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	ADC_init();
	
	while (1)
	{
		unsigned short x = ADC;
		PORTB = (char)(x);
		PORTD = (char)(x >> 8);
	}
}

