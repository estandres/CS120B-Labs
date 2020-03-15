/* 
 * Partner: Steven Strickland sstri014@ucr.edu SID:862155853  
 * Lab Section: 22
 * Assignment: Lab 6 Exercise 2
 *
 * Description: Create a simple light game that requires pressing a button on PA0 while the middle 
 * of three LEDs on PB0, PB1, and PB2 is lit. The LEDs light for 300 ms each in 
 * sequence. When the button is pressed, the currently lit LED stays lit. Pressing
 * the button again restarts the game. 
 * Video Demonstration: http://youtu.be/inmzsXz_HG0
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sers this to 1. C programmer should clear to 0;
enum States{INIT, CYCLE, BHOLD, RESET, RHOLD} state;
unsigned char button = 0x00;
int i = 0;
// Internal variables for mapping AVR'S ISR to out cleaner TimerISR model.
unsigned long _avr_timer_M = 1;// Start count from here, down to 0, Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
void tickFunc(){
	//transition switch
	switch(state){
		case INIT:
			state = CYCLE;
		break;
		case CYCLE:
			if(button){
				state = BHOLD;
			}
			else{
				state = CYCLE;
			}
		break;
		case BHOLD:
			if(button){
				state = BHOLD ;
			}
			else{
				state = RESET;
			}
		break;
		case RESET:
			if(button){
				state = RHOLD;
			}
			else{
				state = RESET;
			}
		break;
		case RHOLD:
			if(button){
				state = RHOLD;
			}
			else{
				state = CYCLE;
			}
		break;
	}
	//action switch
	switch(state){
		case INIT:
		break;
		case CYCLE:
			i++;
			if (i == 1){
				PORTB = 0x01;
			}
			else if(i == 2){
				PORTB = 0x02;
			}
			else if(i == 3){
				PORTB = 0x04;
			}
			else if(i >= 4){
				PORTB = 0x02;
				i = 0;//i=0
			}
		break;
		case BHOLD:
			if (i == 1){
				PORTB = 0x01;
			}
			else if(i == 2){
				PORTB = 0x02;
			}
			else if(i == 3){
				PORTB = 0x04;
			}
			else if(i >= 4){
				PORTB = 0x02;
			}
		break;
		case RESET:
		break;
		case RHOLD:
		break;
		default:
		break;
	}
}
void TimerOn(){
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;	// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scalar /64
	// 00001011: 0x0B
	// SO, 8MHz clock or 8,000,000 /64 = 125,000 ticks/sec
	// Thus, TCNT1 register will count at 125,000 tick/sec

	// AVR output compare register OCR1A
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
	/* Replace with your application code */
	DDRA = 0x00;
	PORTA = 0xFF;
	DDRB = 0xFF;	//	Set port B to output
	PORTB = 0x00;	// Init port B to 0s
	TimerSet(3);
	TimerOn();
	state = INIT;
	PORTB = 0x01;
	while (1){
		// User code
		button = ~PINA & 0x01;
		while(!TimerFlag); // Wait 3 sec
		TimerFlag = 0;
		tickFunc();
		//Note: For the above a better style would use a synchSM with TickSM()
		// This example just illustrates the use of the ISR and flag
	}
}

zz