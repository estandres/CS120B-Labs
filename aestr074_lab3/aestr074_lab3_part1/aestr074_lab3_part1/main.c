/*
 * 
 *  Lab Section: 022
 *  Assignment Lab 3 Exercise 1 
 *	Count the number of 1s on ports A and B and output that number on
 *	port C. 
 *	I acknowledge all content contained herein, excluding template or example
 *  code, is my own original work
 */ 

#include <avr/io.h>

// Bit-access function
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

int main(void){
	DDRA = 0x00; PORTA = 0xFF; // Configure ports A's & B's 8 pins as inputs
	DDRB = 0x00; PORTB = 0xFF; 
	DDRC = 0xFF; PORTC = 0x00; // Configure port C's 8 pins as outputs, initialize to 0s
	unsigned char i;
	unsigned char cnt;
	while (1) {
		cnt=0;
		for (i=0; i<8; i++) {
			if (GetBit(PINA, i)) {	
				cnt++;
			}
			if (GetBit(PINB, i)) {
				cnt++;
			}
		}
		PORTC = cnt;
	}
}


