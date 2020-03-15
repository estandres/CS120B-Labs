/* 
 * Partner: Steven Strickland sstri014@ucr.edu SID:862155853  
 * Lab Section: 22
 * Assignment: Lab 5 Exercise 1
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */ 

#include <avr/io.h>

int main(void)
{
	
	DDRA = 0x00;
	DDRB = 0x00;
	DDRC = 0xFF;
	
	PORTA = 0xFF;
	PORTB = 0xFF;
	PORTC = 0x00;
	
	unsigned char temp = 0x00;
	
    while (1) 
    {
		temp = ~PINA & 0x0F;
		
		if((temp == 0))
			PORTC = 0x40;
		if((temp == 1) || (temp == 2))
			PORTC = 0x60;	
		if((temp == 3) || (temp == 4))
			PORTC = 0x70;
		if((temp == 5) || (temp == 6))
			PORTC = 0x38;
		if((temp == 7) || (temp == 8) || (temp == 9))
			PORTC = 0x3C;
		if((temp == 10) || (temp == 11) || (temp == 12))
			PORTC = 0x3E;
		if((temp == 13) || (temp == 14) || (temp == 15))
			PORTC = 0x3F;
		
    }
	return 0;
}