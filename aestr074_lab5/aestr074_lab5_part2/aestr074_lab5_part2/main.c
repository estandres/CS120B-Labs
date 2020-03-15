/* 
 * Partner: Steven Strickland sstri014@ucr.edu SID:862155853  
 * Lab Section: 22
 * Assignment: Lab 5 Exercise 2
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */ 

#include <avr/io.h>

enum simpleCalculator {init, increment, decrement, reset, wait, waitincrement, waitdecrement, waitreset} state;
unsigned char button0 = 0x00;
unsigned char button1 = 0x00;

void calculate()
{
	//transitions
	switch(state)
	{
		case init:
			state = wait;
			break;
		
		case wait:
			if(!button0 && button1)
				state = increment;
			if(button0 && !button1)
				state = decrement;
			if(button0 && button1)
				state = reset;
			if(!button0 && !button1)
				state = wait;
			break;
		
		case increment:
			state = waitincrement;
			break;
		
		case waitincrement:
			if(!button0 && button1)
				state = waitincrement;
			else
				state = wait;
			break;
		
		case decrement:
			state = waitdecrement;
			break;
		
		case waitdecrement:
			if(button0 && !button1)
				state = waitdecrement;
			else
				state = wait;
			break;
		
		case reset:
			state = waitreset;
			break;
		
		case waitreset:
			if(button0 && button1)
				state = waitreset;
			else
				state = wait;
			break;
		
		default:
			state = init;
			break;
	}
	
	//actions
	switch(state)
	{
		case init:
			break;
		
		case wait:
			break;
		
		case increment:
			if(PORTC != 0x09)
				PORTC = PORTC + 0x01;
			break;
			
		case waitincrement:
			break;
		
		case decrement:
			if(PORTC != 0x00)
				PORTC = PORTC - 0x01;
			break;
			
		case waitdecrement:
			break;
		
		case reset:
			PORTC = 0x00;
			break;
			
		case waitreset:
			break;
		
		default:
			break;
	}
}

int main(void)
{
	
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	
	state = init;
	PORTC = 0x00; //initial starting value for PORTC
	
	while (1)
	{
		button0 = ~PINA & 0x01;
		button1 = ~PINA & 0x02;
		calculate();
	}
	return 0;
}
