/*
 * 
 *	Lab Section: 022
 *  Assignment: Lab 3 Exercise 3
 *	Count the number of 1s on ports A and B and output that number on
 *	port C. 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *  code, is my own original work
 */ 

#include <avr/io.h>
#include <math.h>
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
	double lowVal;
	double hiVal;
	unsigned char tmpC;
	double i;
	double j;
    //double base;
	while (1) {
		tmpC = 0x00;
		lowVal = 0;
		hiVal = 0;
		for (i=0; i<4; i++) {
			if (GetBit(PINA, i)) {	
				lowVal += floor(pow(2, i) + .5);	
			}
		}
		for (j=4; j<8; j++) {
			if (GetBit(PINA, j)) {
				hiVal += floor(pow(2, j-4) + .5);
			}
		}
		if (lowVal == 0){}
		else if (lowVal <= 2){
			tmpC = (0x05)<<4;
		}
		else if (lowVal <= 4){
			tmpC = (0x07)<<4;
		}
		else if (lowVal <= 6){
			tmpC = (0x07)<<3;
		}
		else if (lowVal <= 9){
			tmpC = (0x0F)<<2;
		}
		else if (lowVal <= 12){
			tmpC = (0x1F)<<1;
		}
		else if (lowVal <= 15){
			tmpC = 0x3F;
		}

		if(hiVal == 3){
			tmpC = (tmpC | 0x80);
		}
		PORTC = tmpC;
	}
}


