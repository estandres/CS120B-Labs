/*
 *	Lab Section: 022
 *	Assignment: Lab 3 Exercise 2
 *	Exercise Description: A car has
 *	a fuel-level sensor that sets
 *	PA3..PA0 to a value between 0 (empty) and 
 *	15 (full). A series of LEDs connected to 
 *	PC5..PC0 should light to graphically 
 *	indicate the fuel level. If the fuel level 
 *	is 1 or 2, PC5 lights. If the level is 3 or 4,
 *  PC5 and PC4 light. Level 5-6 lights PC5..PC3. 
 *	7-9 lights PC5..PC2. 10-12 lights PC5..PC1. 13-15 
 *	lights PC5..PC0. Also, PC6 connects to a "Low fuel"
 *	icon, which should light if the level is 4 or less. 
 *
 * 	I acknowledge all content contained herein, excluding template or example
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
	double val;
	unsigned char tmpC;
	double i;
    //double base;
	while (1) {
		tmpC = 0x00;
		val = 0;
		for (i=0; i<4; i++) {
			if (GetBit(PINA, i)) {	
				val += floor(pow(2, i) + .5);	
			}
		}
		if (val == 0){
			
		}
		else if (val <= 2){
			tmpC = (0x05)<<4;
		}
		else if (val <= 4){
			tmpC = (0x07)<<4;
		}
		else if (val <= 6){
			tmpC = (0x07)<<3;
		}
		else if (val <= 9){
			tmpC = (0x0F)<<2;
		}
		else if (val <= 12){
			tmpC = (0x1F)<<1;
		}
		else if (val <= 15){
			tmpC = 0x3F;
		}
		PORTC = tmpC;
	}
}


