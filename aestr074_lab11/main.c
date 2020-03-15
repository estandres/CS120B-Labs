/*
 *	Partner: Steven Strickland sstri014@ucr.edu SID:862155853
 *	Lab Section: 22
 *	Assignment: Lab 10 Exercise 1
 *	
 *	Description: Modify the keypad code to be in an SM task. Then, modify the 
 *	keypad SM to utilize the simple task scheduler format. All code from here on out 
 *	should use the task scheduler. 
 */
#include "keypad.h"
#include "scheduler.h"

void TickFunc(int state){

}
volatile unsigned char TimerFlag = 0; // TimerISR() sers this to 1. C programmer should clear to 0;
enum States{INIT};
typedef struct tasks{
	int state;					//Task's current state
	unsigned long period;		//Task's period
	unsigned long elapsedTime;  //Time elapsed since last task tick
	int (*TickFunc)(int);		//Task tick functionS
}task;
task tasks[1];
const unsigned short tasksNum = 1;
unsigned int tasksPeriod = 1;
unsigned char x;
enum KP_States{KP_START, KP_INPUT};
unsigned char keyPad;
int TickFunc_keyPad(int state){
	switch (state){
		case KP_START:
			state = KP_INPUT;
		break;
		case KP_INPUT:
		break;
		default:
		break;
	}
	switch (state){
		case KP_START:
		break;
		case KP_INPUT:
			x = GetKeypadKey();
			switch(x){
				case '\0': PORTB = 0x1F; break;
				case '1': PORTB = 0x01; break;
				case '2': PORTB = 0x02; break;
				case '3': PORTB = 0x03; break;
				case '4': PORTB = 0x04; break;
				case '5': PORTB = 0x05; break;
				case '6': PORTB = 0x06; break;
				case '7': PORTB = 0x07; break;
				case '8': PORTB = 0x08; break;
				case '9': PORTB = 0x09; break;
				case 'A': PORTB = 0x0A; break;
				case 'B': PORTB = 0x0B; break;
				case 'C': PORTB = 0x0C; break;
				case 'D': PORTB = 0x0D; break;
				case '*': PORTB = 0x0E; break;
				case '0': PORTB = 0x00; break;
				case '#': PORTB = 0x0F; break;
				default: PORTB = 0x1B; break;
			}
		break;
		default:
		break;
	}
	return state;
}
unsigned long _avr_timer_M = 1;// Start count from here, down to 0, Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
void TimerOn(){
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;	// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scalar /64
	// 00001011: 0x0B
	// SO, 8MHz clock or 8,000,000 /64 = 125,000 ticks/sec
	// Thus, TCNT1 register will count at 125,000 tick/sec

	// AVR output com           pare register OCR1A
	OCR1A = 125;	//	Timer interrupt will be generated when TCNT1 == OCR1A
	//	We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	//	So when TCNT1 register equals 125,
	//	1 ms has passed. Thus, we compare to 125.
	//	AVR timer interrupt mask register
	TIMSK1 = 0x02;	//	bit1: OCIE1A -- enables compare match interrupt

	//	Initialize avr counter
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	//	TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80;	//	0x80: 1000000
}

void TimerOff(){
	TCCR1B = 0x00;	//	bit3bit1bit0=000: timer off
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
void TimerISR(){
	unsigned char i;
	for( i = 0; i < tasksNum; ++i){
		if (tasks[i].elapsedTime>= tasks[i].period){
			tasks[i].state = tasks[i].TickFunc(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriod;
	}
	TimerFlag = 1;
};
//	CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
ISR(TIMER1_COMPA_vect){
	// CPU automatically calls when TCNT1 == 0CR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--;	//	Count down to 0 rather than up to TOP
	if(_avr_timer_cntcurr ==0){	//	results in a more efficient compare
		TimerISR();	//	Call the ISR that the user uses.
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
//	Set TimerISR() to tick every M ms
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

int main(void){
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	unsigned char i = 0;
	tasks[i].state = KP_START;
	tasks[i].period = 1;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFunc = &TickFunc_keyPad;
	TimerSet(tasksPeriod);
	TimerOn();
	while(1){

	}
}