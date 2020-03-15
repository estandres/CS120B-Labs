/*
 *	Partner: Steven Strickland sstri014@ucr.edu SID:862155853
 *	Lab Section: 22
 *	Assignment: Lab 10 Exercise 3
 *
 *	Description: Modify the above example so the threeLEDs light for 300 ms, 
 *	while blinkingLED’s LED still blinks 1 second on and 1 second off.
 *	To the previous exercise's implementation, connect your speaker's red wire 
 *	to PB4 and black wire to ground. Add a third task that toggles PB4 on for 2 
 *	ms and off for 2 ms as long as a switch on PA2 is in the on position. Don’t 
 *	use the PWM for this task.
 *
 *
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>

//synchSMs
void TickFunc(int state){

}

enum States{INIT};
typedef struct tasks{
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFunc)(int);
}task;

task tasks[4];
const unsigned short tasksNum = 4;
unsigned int tasksperiod = 1;

//initialization
unsigned char three = 0x00;
unsigned char blinking = 0x00;
unsigned char sounds = 0x00;

enum ThreeLEDs{Three_Start, Three_LED0, Three_LED1, Three_LED2 };
int Tick_ThreeLEDs(int state){
	switch (state){
		case Three_Start:
			state = Three_LED0;
		break;
		case Three_LED0:
			state = Three_LED1;
		break;
		case Three_LED1:
			state = Three_LED2;
		break;
		case Three_LED2:
			state = Three_LED0;
		break;
		default:
			state = Three_Start;
		break;
	}
	switch (state){
		case Three_Start:
			three = 0x00;
		break;
		case Three_LED0:
			three = 0x01;
		break;
		case Three_LED1:
			three = 0x02;
		break;
		case Three_LED2:
			three = 0x04;
		break;
		default:
		break;
	}

	return state;
};

enum BlinkingLED{Blinking_Start, Blinking_LED};
int Tick_BlinkingLEDs(int state){
	switch(state){
		case Blinking_Start:
			state = Blinking_LED;
		break;
		case Blinking_LED:
			state = Blinking_Start;
		default:
		break;
	}
	switch(state){
		case Blinking_Start:
			blinking = 0x00;
		break;
		case Blinking_LED:
			blinking = 0x08;
		break;
		default:
		break;

	}
	return state;
};

enum CombineLEDs{Combine_Start, Combine_Display};
int Tick_CombinedLEDs(int state){
	unsigned char button = ~PINA & 0x04;
	switch(state){
		case Combine_Start:
			state = Combine_Display;
		break;
		case Combine_Display:
			state = Combine_Display;
		break;
	}
	switch (state){
		case Combine_Start:
		break;
		case Combine_Display:
			if(button)
				PORTB = three | blinking | sounds;
			else
				PORTB = three | blinking;
		break;
		default:
		break;
	}
	return state;
};

enum sound{init, off, on};
int Tick_Sound(int state){
	switch(state){
		case init:
			state = off;
		break;
		case off:
			state = on;
		break;
		case on:
			state = off;
		break;
		default:
			state = init;
		break;
	}
	
	switch(state){
		case init:
		break;
		case off:
			sounds = 0x00;
		break;
		case on:
			sounds = 0x10;
		break;
		default:
		break;
	}
	return state;
};

//timer
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR(){
	unsigned char i;
	for( i = 0; i < tasksNum; i++){
		if (tasks[i].elapsedTime>= tasks[i].period)
		{
			tasks[i].state = tasks[i].TickFunc(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksperiod;
	}
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

int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	
	unsigned char i = 0;
	tasks[i].state = Three_Start;
	tasks[i].period = 300;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFunc = &Tick_ThreeLEDs;
	++i;
	tasks[i].state = Blinking_Start;
	tasks[i].period = 1000;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFunc = &Tick_BlinkingLEDs;
	++i;
	tasks[i].state = Combine_Start;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFunc = &Tick_CombinedLEDs;
	++i;
	tasks[i].state = init;
	tasks[i].period = 2;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFunc = &Tick_Sound;
	
	TimerSet(1);
	TimerOn();
	
	while(1){}
	
	return 0;
}