/*
 *	Lab Section: 022
 *	Assignment: Lab 1 Exercise 1
 *	Exercise Description: Garage open at night-- A garage door 
 *	sensor connects to PA0 (1 means door open), and a light sensor 
 *	connects to PA1 (1 means light is sensed). Write a program that 
 *	illuminates an LED connected to PB0 (1 means illuminate) if the 
 *	garage door is open at night.
 * 
 */ 

#include <avr/io.h>
#ifdef _SIMULATE_
#include "simsAVRHeader.h"
#endif
int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
	DDRB = 0xFF; PORTB = 0x00; // Configure port B's 8 pins as outputs, initialize to 0s
	unsigned char led = 0x00; // Temporary variable to hold the value of B
	unsigned char button = 0x00; // Temporary variable to hold the value of A
	while(1)
	{
		// 1) Read input
		button = ~PINA & 0x01;
		// 2) Perform computation
		// if PA0 is 1, set PB1PB0 = 01, else = 10
		if (button) { // True if PA0 is 1
			led =  0x01; // Sets tmpB to 00000001 meaning led is on
		}
		else {
			led =  0x02; // Sets tmpB to 00000000 meaning led is off
		}
		// 3) Write output
		PORTB = led;
	}
	return 0;
}


