/*
 *	Lab Section: 022
 *	Assignment: Lab 4 Exercise 2
 * 
 *	Buttons are connected to PA0 and PA1. Output for PORTC is initially 7. Pressing
 *	PA0 increments PORTC once (stopping at 9). Pressing PA1 decrements PORTC once
 *  (stopping at 0). If both buttons are depressed (even if not initially 
 *  simultaneously), PORTC resets to 0.
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */ 

#include <avr/io.h>
enum States {init, display, inc_hold, dec_hold, increment, decrement, reset_hold, reset} state;

unsigned char tickFunc(unsigned char cnt){
	switch (state){
		case init:
			state = display;
			break;
		case display:
			if (PINA == 0x00){
				state = display;
			}
			else if (PINA == 0x01){
				state = dec_hold;
			}
			else if (PINA == 0x02){
				state = inc_hold;
			}
			else if (PINA == 0x03){
				state = reset_hold;
			}
			break;
		case inc_hold:
			if (PINA == 0x00){
				state = increment;
			}
			else if (PINA == 0x01){}
			else if (PINA == 0x02){
				state = inc_hold;
			}
			else if (PINA == 0x03){
				state = reset_hold;
			}
			break;
		case dec_hold:
			if (PINA == 0x00){
				state = decrement;
			}
			else if (PINA == 0x01){
				state = dec_hold;
			}
			else if (PINA == 0x02){}
			else if (PINA == 0x03){
				state = reset_hold;
			}
			break;
		case decrement:
			break;
		case increment:
			break;
		case reset:
			break;
		case reset_hold:
			if (PINA == 0x00){
				state = reset;
			}
			else{
				state = reset_hold;
			}
			break;
	}

	switch (state){
		case init:		
			break;
		case display:
			PORTC = cnt;
			break;
		case inc_hold:
			break;
		case dec_hold:
			break;
		case decrement:
			state = display;
			if (cnt>0){
				cnt--;
			}
			else{};
			break;
		case increment:
			state = display;
			if (cnt<10){
				cnt++;
			}
			else{};
			break;
		case reset:
			state = display;
			cnt = 0;
			break;
		case reset_hold:
			break;
			}
		return cnt;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	state = init;
    unsigned char count = 7;
    while (1) 
    {
		count = tickFunc(count);
    }
}

