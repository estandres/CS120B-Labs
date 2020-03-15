/*
 *	Lab Section: 022
 *	Assignment: Lab 4 Exercise 1
 * 
 *	Exercise Description: PB0 and PB1 each connect to an LED, and PB0's LED is
 *	initially on. Pressing a button connected to PA0 turns off PB0's LED and turns
 *	on PB1's LED, staying that way after button release. Pressing the button 
 *	again turns off PB1's LED and turns on PB0's LED. 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */ 

//#include "RIMS.h"
#include <avr/io.h>
enum States {init, led1, led1_hold, led2, led2_hold} state;
void tickFunc(){
	unsigned char b0_ON = 0x01;
	unsigned char b1_ON = 0x02;
	switch(state){
		case init:
			state = led1;
			break;
		case led1:
			if (PINA == 0x00){
				state = led1;
			}
			else if (PINA == 0x01){
				state = led2_hold;
			}
			break;
		case led2_hold:
			if (PINA == 0x00){
				state = led2;
			}
			else if (PINA == 0x01){
				state = led2_hold;
			}
			break;
		case led2:
			if (PINA == 0x00){
				state = led2;
			}
			else if (PINA == 0x01){
				state = led1_hold;
			}
			break;
		case led1_hold:
			if (PINA == 0x00){
				state = led1_hold;
			}
			else if (PINA == 0x01){
				state = led1;
			}
			break;
		default: 
			state = init;
			break;
	}
	switch(state){
		case init:
			PORTB = b0_ON;
			break;
		case led1:
			PORTB = b0_ON;
			break;
		case led1_hold:
			PORTB = b0_ON;
			break;
		case led2:
		PORTB = b1_ON;
			break;
		case led2_hold:
			PORTB = b1_ON;
			break;
	}

}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	state = init;
    /* Replace with your application code */
    while (1) 
    {
		tickFunc();

    }
}

