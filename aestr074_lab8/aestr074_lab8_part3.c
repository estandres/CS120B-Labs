/* 
 * 	Partner: Steven Strickland sstri014@ucr.edu SID:862155853  
 * 	Lab Section: 22
 * 	Assignment: Lab 8 Exercise 3
 *  Description: Design a system where an LED is illuminated only 
 *	if enough light is detected from the photo resistor. Criteria:
 *	If the result of the ADC is >= MAX/2, the LED is illuminated.
 *	If the result of the ADC is < MAX/2, the LED is turned off.
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
		
		/*if((char)(x) >= (char)(0010000111 / 2))
			PORTB = 0x01;
		if((char)(x) <= (char)(0001000111 / 2))
			PORTB = 0x00;*/
			
		if(x >= 0x57)
			PORTB = 0x01;
		if(x <= 0x47)
			PORTB = 0x00;
	}
}

