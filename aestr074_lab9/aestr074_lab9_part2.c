/* 
 * Partner: Steven Strickland sstri014@ucr.edu SID:862155853  
 * Lab Section: 22
 * Assignment: Lab 9 Exercise 2
 *
 *	Using the ATmega1284’s PWM functionality, design a system where the notes: C4, 
 *	D, E, F, G, A, B, and C5,  from the table at the top of the lab, can be generated 
 *	on the speaker by scaling up or down the eight note scale. Three buttons are used 
 *	to control the system. One button toggles sound on/off. The other two buttons 
 *	scale up, or down, the eight note scale. 
 *	Criteria:
 *	The system should scale up/down one note per button press.
 *	When scaling down, the system should not scale below a C.
 *	When scaling up, the system should not scale above a C.

 *
 */ 

#include <avr/interrupt.h>
#include <avr/io.h>
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


volatile unsigned char TimerFlag = 0; // TimerISR() sers this to 1. C programmer should clear to 0;
enum States{INIT, IDLE, SOUND, ON_HOLD, OFF_HOLD, INC_HOLD, DEC_HOLD} state;
unsigned char button_start = 0x00;
unsigned char button_inc = 0x00;
unsigned char button_dec = 0x00;
double freqs [8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
unsigned int flags[2] = {0, 0};
unsigned int i = 0;
// Internal variables for mapping AVR'S ISR to out cleaner TimerISR model.
unsigned long _avr_timer_M = 1;// Start count from here, down to 0, Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
void tickFunc(){
	switch(state){ //transition switch
		case INIT:
			state = IDLE;
		break;
		case IDLE:
			if(button_start){
				state = ON_HOLD;
			}
			else if(!button_start){
				state = IDLE;
			}
		break;
		case ON_HOLD:
			if(button_start){
				state = ON_HOLD;
			}
			else{
				state = SOUND;
			}
		break;
		case OFF_HOLD:
			if(button_start){
				state = OFF_HOLD;
			}
			else{ 
				state = IDLE;
			}
		break;
		case SOUND:
			if(button_start){
				state = OFF_HOLD;
			}
			else if (button_inc ){
				state = INC_HOLD;
			}
			else if (button_dec){
				state = DEC_HOLD;
			}
			else {
				state = SOUND;
			}
			
		break;
		case INC_HOLD:
			if (button_inc){
				state = INC_HOLD;
			}
			else{
				state = SOUND;
			}
		break;
		case DEC_HOLD:
			if (button_dec){
				state = DEC_HOLD;
			}
			else{
				state = SOUND;
			}
		break;		
		default:
		break;
	}
	switch (state){//action switch
		case INIT:
		break;

		case IDLE:
			set_PWM(0);
			PWM_off();
		break;
		case INC_HOLD:
			flags[0] = 1;
		break;
		case DEC_HOLD:
			flags[1] = 1;
		break;
		case ON_HOLD:
			PWM_on();	
			set_PWM(freqs[i]);
		break;
		case OFF_HOLD:
		break;
		case SOUND:
			if (flags[0] == 1){
				if (i < 7){
					i++;
				}
				set_PWM(freqs[i]);
				flags[0] = 0;
			}
			else if (flags[1] == 1){
				if (i>0){
					i--;
				}
				set_PWM(freqs[i]);
				flags[1] = 0;
			}
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
	//TimerSet(3);
	//TimerOn();
	PWM_on();
	state = INIT;
	while (1){
		// User code
		button_start = ~PINA & 0x01;
		button_inc = ~PINA & 0x02;
		button_dec = ~PINA & 0x04;
		tickFunc();
		//Note: For the above a better style would use a synchSM with TickSM()
		// This example just illustrates the use of the ISR and flag
	}
}

