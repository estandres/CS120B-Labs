/*
 *	Lab Section: 022	
 *	Assignment: Lab 1 Exercise 2
 *	Exercise Description: Port A's pins 3 to 0, each connect to 
 *	a parking space sensor, 1 meaning a car is parked in the space, 
 *	of a four-space parking lot. Write a program that outputs in 
 *	binary on port C the number of available spaces (Hint: declare 
 *	a variable "unsigned char cntavail"; you can assign a number 
 *	to a port as follows: PORTC = cntavail;).
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */ 

#include <avr/io.h>
#include <stdio.h>
int main(void){
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
	DDRC = 0xFF; PORTC = 0x00; // Configure port B's 8 pins as outputs
	// Initialize output on PORTB to 0x00
	unsigned char cntAvail = 0x00;
	//unsigned char tmpA = 0x00;
	while(1){
		//tmpA = PINA;
		if (PINA == 0x00){
				cntAvail = 0x04;
		}
		else if ((PINA == 0x01) || (PINA == 0x02) || (PINA == 0x04) || (PINA == 0x08)){
			cntAvail = 0x03;
		}
		else if ((PINA == 0x0C) || (PINA == 0x0A) || (PINA == 0x09) || (PINA == 0x06) || (PINA == 0x05) || (PINA == 0x03)){
				cntAvail = 0x02;
		}
		else if ((PINA == 0x0E) || (PINA == 0x0D) || (PINA == 0x0B) || (PINA == 0x07)){
				cntAvail = 0x01;
		}
		else if (PINA == 0x0F){
				cntAvail = 0x00;
		}
		PORTC = cntAvail;
	}
}


