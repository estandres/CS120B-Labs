/* 
 * 	Partner: Steven Strickland sstri014@ucr.edu SID:862155853  
 * 	Lab Section: 22
 * 	Assignment: Lab 6 Exercise 1
 *
 * 	Description: Create a synchSM to blink three LEDs connected 
 *	to PB0, PB1, and PB2 in sequence, 1 second each. Implement that 
 *	synchSM in C using the method defined in class. In addition 
 *	to demoing your program, you will need to show that your code
 *	adheres entirely to the method with no variations. 
 *  Video Demonstration: http://youtu.be/ZS1Op26WiBM
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

enum statemachine{init, led0, led1, led2} state;
	
void lightLEDS()
{
	//transitions
	switch(state)
	{
		case init:
			state = led0;
			break;
		case led0:
			state = led1;
			break;
		case led1:
			state = led2;
			break;
		case led2:
			state = led0;
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
		case led0:
			PORTB = 0x01;
			break;
		case led1:
			PORTB = 0x02;
			break;
		case led2:
			PORTB = 0x04;
			break;
		default:
			break;
	}
}

///////////Timer Start///////////////////
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A =125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
};

void TimerOff(){
	TCCR1B = 0x00;
};

void TimerISR(){
	TimerFlag = 1;
};

ISR(TIMER1_COMPA_vect){

	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr ==0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
///////////Timer End///////////////////

int main(void)
{
	TimerSet(110);
	TimerOn();
	
	DDRB = 0xFF; PORTB = 0x00;
	
	while (1)
	{
		lightLEDS();
		while(!TimerFlag);
		TimerFlag = 0;
	}
}

