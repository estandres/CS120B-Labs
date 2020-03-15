/* 
 * 	Partner: Steven Strickland sstri014@ucr.edu SID:862155853  
 * 	Lab Section: 22
 * 	Assignment: Lab 8 Exercise 1
 *	Description: Make sure your breadboard is wired according to the prelab.
 *	The potentiometer is used to adjust the voltage supplied to 
 *	the microcontroller for ADC . Design a system that reads the
 *	10-bit ADC result from the ADC register, and displays the 
 *	result on a bank of 10 LEDs.
 *	Hints: Assuming the breadboard has been wired according to 
 *	the prelab photo, display the lower 8 bits on port B, and the 
 *	upper 2 bits on port D. Use a “short” variable to hold the ADC result.
 *	Use bit shifting and casting to align the proper bits to the proper
 *	ports. For example:
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

