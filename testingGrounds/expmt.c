#include <avr/interrupt.h>
#include <avr/io.h>
#define ROW1A 0x01
#define ROW2A 0x20
#define ROW3B 0x80
#define ROW4A 0x08
#define ROW5B 0x01
#define ROW6B 0x40
#define ROW7B 0x02
#define ROW8B 0x10
#define COL1A 0x10
#define COL2B 0x04
#define COL3B 0x08
#define COL4A 0x02
#define COL5B 0x20
#define COL6A 0x04
#define COL7A 0x40
#define COL8A 0x80

#include <avr/interrupt.h>
#include <avr/io.h>
/*
 *	Partner: Steven Strickland sstri014@ucr.edu SID:862155853
 *	Lab Section: 22
 *	Assignment: Lab 10 Exercise 1
 *
 *	Description: Modify the above example so the threeLEDs light for 300 ms, 
 *	while blinkingLED’s LED still blinks 1 second on and 1 second off.
 *
 *
 */

// 0.954 hz is lowest frequency possible with this function
//  based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound

void set_PWM(double frequency){
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if(frequency != current_frequency){
		if(!frequency) {TCCR3B &= 0x08;}
		else{ TCCR3B |= 0x03;}

		// prevents 0CR3A from overflowing, using prescalar 64
		// 0.954 is smallest frequency that will not result in overflow
		if(frequency<0.954){OCR3A = 0xFFFF;}

		// prevents 0CR3A from underflowing, using prescalar 64
		// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250){OCR3A = 0x0000;}
		// set OCR3A based on desired frequency
		else {OCR3A = (short)(8000000/ (128*frequency)) - 1;}
		TCNT3 = 0;
		current_frequency = frequency; // Updates current frequency
	}
}

void PWM_on(){
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB3 on compare match between counter and 0CR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT) matches 0CR3A, reset counter
	// CS31 & CS30: Set a prescalar of 64
	set_PWM(0);
}

void PWM_off(){
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

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
task tasks[2];
const unsigned short tasksNum = 2;
unsigned int tasksPeriod = 300; //GCD of all task periods
//globals
unsigned char GL_A;
unsigned char GL_B;
unsigned char blinkingLED;
unsigned char button = 0x00;
unsigned char stack_inc = 7;
unsigned char curr_row = 0x00;
unsigned char row[7] = {ROW2A, ROW3B, ROW4A, ROW5B, ROW6B, ROW7B, ROW8B};
unsigned char a_pins[7], b_pins[7];
unsigned char i = 0;

enum GL_States{GL_INIT, GL_CYCLE, GL_BUTTON};
int TickFunc_GameLogic(int state){
	button = ~PINC & 0x01;
	switch (state){
		case GL_INIT:
			state = GL_CYCLE;
		break;
		case GL_CYCLE:
			if(button){
				state = GL_BUTTON;
			}
			else{
				state = GL_CYCLE;
			}
		break;
		case GL_BUTTON:
			if(button){
				state = GL_BUTTON ;
			}
			else{
				state = GL_CYCLE;
				stack_inc--;
			}
		break;
		default:
		break;
	}
	switch (state){
		case GL_INIT:
		break;
		case GL_CYCLE:
			i++;
			curr_row = row[stack_inc-1];
			if(curr_row == ROW2A || curr_row == ROW4A){
				if (i == 1){
					GL_B = COL2B | COL3B | COL5B;//||12345   ||
					GL_A = curr_row | COL1A | COL4A;
				}
				else if(i == 2 || i == 10){
					GL_B = COL2B | COL3B;//||1234   8||
					GL_A = curr_row | COL1A | COL4A | COL8A;
				}
				else if(i == 3 || i == 9){
					GL_B = COL2B | COL3B;//||123   78||
					GL_A = curr_row | COL1A | COL7A | COL8A;
				}
				else if(i == 4 || i == 8){
					GL_B = COL2B;			//||12   678||
					GL_A = curr_row | COL1A | COL6A | COL7A | COL8A;
				}
				else if(i == 5 || i == 7){
					GL_B = COL5B;//||1   5678||
					GL_A = curr_row | COL1A | COL6A | COL7A |COL8A;
				}
				else if(i == 6){
					GL_B = COL5B;//||  45678||
					GL_A = curr_row | COL4A | COL6A | COL7A | COL8A;
				}
				else if (i == 11){
					GL_B = COL2B | COL3B | COL5B;//||12345   ||
					GL_A = curr_row | COL1A | COL4A;
					i = 1;
				}
			}
			else if (curr_row == ROW3B || curr_row == ROW5B || curr_row == ROW6B || curr_row == ROW7B || curr_row == ROW8B){
				if (i == 1){
					GL_B = curr_row | COL2B | COL3B | COL5B;//||12345   ||
					GL_A = COL1A | COL4A;
				}
				else if(i == 2 || i == 10){
					GL_B = curr_row | COL2B | COL3B;//||1234   8||
					GL_A = COL1A | COL4A | COL8A;
				}
				else if(i == 3 || i == 9){
					GL_B = curr_row | COL2B | COL3B;//||123   78||
					GL_A = COL1A | COL7A | COL8A;
				}
				else if(i == 4 || i == 8){
					GL_B = curr_row | COL2B;			//||12   678||
					GL_A = COL1A | COL6A | COL7A | COL8A;
				}
				else if(i == 5 || i == 7){
					GL_B = curr_row | COL5B;//||1   5678||
					GL_A = COL1A | COL6A | COL7A |COL8A;
				}
				else if(i == 6){
					GL_B = curr_row | COL5B;//||  45678||
					GL_A = COL4A | COL6A | COL7A | COL8A;
				}
				else if (i == 11){
					GL_B = curr_row | COL2B | COL3B | COL5B;//||12345   ||
					GL_A = COL1A | COL4A;
					i = 1;
				}
			}
			else {stack_inc = 7;}
			PORTA = GL_A;
			PORTB = GL_B;
		break;
		case GL_BUTTON: 
			if (stack_inc <= 6){
				a_pins [6-stack_inc] = GL_A;
				b_pins [6-stack_inc] = GL_B;
			}
		break;
		default:
		break;
	}

	return state;
};
enum CL_States{CL_START, CL_DISPLAY};
int TickFunc_CombinedLEDs(int state){
	switch(state){
		case CL_START:
			state = CL_DISPLAY;
		break;
		case CL_DISPLAY:
		break;
	}
	switch (state){
		case CL_START:
		break;
		case CL_DISPLAY:
			PORTA = GL_A;
			PORTB = GL_B;
		break;
		default:
		break;
	}
	return state;
};
// Internal variables for mapping AVR'S ISR to out cleaner TimerISR model.
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
	unsigned char j;
	for( j = 0; j < tasksNum; ++j){
	    if (tasks[j].elapsedTime>= tasks[j].period){
	        tasks[j].state = tasks[j].TickFunc(tasks[j].state);
	        tasks[j].elapsedTime = 0;
	    }
	    tasks[j].elapsedTime += tasksPeriod;
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

int main(){
	DDRA = 0xFF;
	PORTA = 0x00;
	DDRB = 0xFF;
	PORTB = 0x00;
	DDRC = 0x00;
	PORTC = 0xFF;
	unsigned char j = 0;
	tasks[j].state = GL_INIT;
	tasks[j].period = 300;
	tasks[j].elapsedTime = 0;
	tasks[j].TickFunc = &TickFunc_GameLogic;
	j++;
	tasks[j].state = CL_START;
	tasks[j].period = 300;
	tasks[j].elapsedTime = 0;
	tasks[j].TickFunc = &TickFunc_CombinedLEDs;
	TimerSet(tasksPeriod);
	TimerOn();
	while(1){
		
	}
	return 0;
}