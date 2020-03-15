/*
 *	Lab Section: 022
 *	Assignment: Lab 4 Exercise 3
 * 
 *	A household has a digital combination deadbolt lock system on the doorway. T
 *	he system has buttons on a keypad. Button 'X' connects to PA0, 'Y' to PA1, and
 *  '#' to PA2. Pressing and releasing '#', then pressing 'Y', should unlock the
 *  door by setting PB0 to 1. Any other sequence fails to unlock. Pressing a 
 *  button from inside the house (PA7) locks the door (PB0=0). For debugging
 *  purposes, give each state a number, and always write the current 
 *	state to PORTC (consider using the enum state variable). Also, be sure 
 *	to check that only one button is pressed at a time. 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */ 

#include <avr/io.h>
enum States {init, start, prime_hold, prime, unlock_hold, 
			unlock, start_hold, lock_hold, lock} state;

void tickFunc(){
	unsigned char tmpB = 0x00;
	switch (state){
		case init:
			state = start;
			break;
		case start:
			if (PINA == 0x04){
				state = prime_hold;
			}
			else if(PINA == 0x80){
				state = lock_hold;
			}
			else{
				state = start;
			}
			break;
		case prime_hold:
			if (PINA == 0x00){
				state = prime;
			}
			else if(PINA == 0x80){
				state = lock_hold;
			}
			else{
				state = prime_hold;
			}
			break;
		case prime:
			if (PINA == 0x02){
				state = unlock_hold;
			}
			else if (PINA == 0x80){
				state = lock_hold;
			}
			else{
				state = prime;
			}
			break;
		case unlock_hold:
			if (PINA == 0x00){
				state = unlock;
			}
			else{
				state = unlock_hold;
			}
			break;
		case unlock:
			state = start;
			break;
		case start_hold:
			if(PINA == 0x00){
				state = start;
			}
			else{
				state = start_hold;
			}
			break;
		case lock_hold:
			if (PINA == 0x00){
				state = lock;
			}
			else{
				state = lock_hold;
			}
			break;
		case lock:
			state = start;
			break;
	}

	switch (state){
		case init:
			break;
		case start:
			break;
		case prime_hold:
			break;
		case prime:
			break;
		case unlock_hold:
			break;
		case unlock:
			tmpB = 0x01;
			PORTB = tmpB;
			break;
		case start_hold:
			break;
		case lock_hold:
			break;
		case lock:
			tmpB = 0x00;
			PORTB = tmpB;
			break;
	}
	PORTC = state;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	state = init;
    while (1) 
    {
		tickFunc();
    }
}

