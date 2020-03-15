
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
task tasks[3];
const unsigned short tasksNum = 3;
unsigned int tasksPeriod = 100;
enum TL_States{TL_START, TL_BIT0, TL_BIT1, TL_BIT2 };
unsigned char threeLEDs;
unsigned char blinkingLED;
int TickFunc_ThreeLEDs(int state){
	switch (state){
		case TL_START:
			state = TL_BIT0;
		break;
		case TL_BIT0:
			state = TL_BIT1;
		break;
		case TL_BIT1:
			state = TL_BIT2;
		break;
		case TL_BIT2:
			state = TL_BIT0;
		break;
		default:
		break;
	}
	switch (state){
		case TL_START:
		break;
		case TL_BIT0:
			threeLEDs = 0x01;
		break;
		case TL_BIT1: 
			threeLEDs = 0x02;
		break;
		case TL_BIT2:
			threeLEDs = 0x04;
		break;
		default:
		break;
	}

	return state;
};
enum BL_States{BL_START, BL_BIT3};
int TickFunc_BlinkingLEDs(int state){
	switch(state){
		case BL_START:
			state = BL_BIT3;
		break;
		case BL_BIT3:
			state = BL_START;
		default:
		break;
	}
	switch(state){
		case BL_START:
			blinkingLED = 0x00;
		break;
		case BL_BIT3:
			blinkingLED = 0x08;
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
			PORTB = threeLEDs | blinkingLED;
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

int main(){
	DDRA = 0xFF;
	PORTA = 0x00;
	DDRB = 0xFF;
	PORTB = 0x00;
	unsigned char row[8] = {0x01, 0x20, 0x80, 0x08, 0x01, 0x40, 0x02, 0x10};
	unsigned char col[8] = {0x10, 0x04, 0x08, 0x02, 0x20, 0x04,0x40, 0x80};
	unsigned char i = 0;
	tasks[i].state = TL_START;
	tasks[i].period = 300;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFunc = &TickFunc_ThreeLEDs;
	i++;
	tasks[i].state = BL_START;
	tasks[i].period = 1000;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFunc = &TickFunc_BlinkingLEDs;
	i++;
	tasks[i].state = CL_START;
	tasks[i].period = 100;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFunc = &TickFunc_CombinedLEDs;
	TimerSet(tasksPeriod);
	TimerOn();
	while(1){
		
	}
	return 0;
}