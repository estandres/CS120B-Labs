

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
enum States{INIT, CYCLE, BHOLD} state;
unsigned char button = 0x00;
unsigned char stack_inc = 7;
unsigned char curr_row = 0x00;
unsigned char row[7] = {ROW2A, ROW3B, ROW4A, ROW5B, ROW6B, ROW7B, ROW8B};
unsigned char a_pins[7], b_pins[7];
unsigned char i = 0;
unsigned char A, B;
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
				state = CYCLE;
				stack_inc--;
			}
		break;
	}
	//action switch
	switch(state){
		case INIT:
		break;
		case CYCLE:
			i++;
			curr_row = row[stack_inc-1];
			if(curr_row == ROW2A || curr_row == ROW4A){
				if (i == 1){
					B = COL2B | COL3B | COL5B;//||12345   ||
					A = curr_row | COL1A | COL4A;
				}
				else if(i == 2 || i == 10){
					B = COL2B | COL3B;//||1234   8||
					A = curr_row | COL1A | COL4A | COL8A;
				}
				else if(i == 3 || i == 9){
					B = COL2B | COL3B;//||123   78||
					A = curr_row | COL1A | COL7A | COL8A;
				}
				else if(i == 4 || i == 8){
					B = COL2B;			//||12   678||
					A = curr_row | COL1A | COL6A | COL7A | COL8A;
				}
				else if(i == 5 || i == 7){
					B = COL5B;//||1   5678||
					A = curr_row | COL1A | COL6A | COL7A |COL8A;
				}
				else if(i == 6){
					B = COL5B;//||  45678||
					A = curr_row | COL4A | COL6A | COL7A | COL8A;
				}
				else if (i == 11){
					B = COL2B | COL3B | COL5B;//||12345   ||
					A = curr_row | COL1A | COL4A;
					i = 1;
				}
			}
			else if (curr_row == ROW3B || curr_row == ROW5B || curr_row == ROW6B || curr_row == ROW7B || curr_row == ROW8B){
				if (i == 1){
					B = curr_row | COL2B | COL3B | COL5B;//||12345   ||
					A = COL1A | COL4A;
				}
				else if(i == 2 || i == 10){
					B = curr_row | COL2B | COL3B;//||1234   8||
					A = COL1A | COL4A | COL8A;
				}
				else if(i == 3 || i == 9){
					B = curr_row | COL2B | COL3B;//||123   78||
					A = COL1A | COL7A | COL8A;
				}
				else if(i == 4 || i == 8){
					B = curr_row | COL2B;			//||12   678||
					A = COL1A | COL6A | COL7A | COL8A;
				}
				else if(i == 5 || i == 7){
					B = curr_row | COL5B;//||1   5678||
					A = COL1A | COL6A | COL7A |COL8A;
				}
				else if(i == 6){
					B = curr_row | COL5B;//||  45678||
					A = COL4A | COL6A | COL7A | COL8A;
				}
				else if (i == 11){
					B = curr_row | COL2B | COL3B | COL5B;//||12345   ||
					A = COL1A | COL4A;
					i = 1;
				}
			}
			else {stack_inc = 7;}
			PORTA = A;
			PORTB = B;
		break;
		case BHOLD:	
			if (stack_inc <= 6){
				a_pins [6-stack_inc] = A;
				b_pins [6-stack_inc] = B;
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

void display(unsigned char a_bits, unsigned char b_bits){
	for (int it = 0; it < 300; it++){
		PORTA = a_bits;
		PORTB = b_bits;
	}
}
int main(void){
	/* Replace with your application code */
	DDRA = 0xFF;
	PORTA = 0x00;
	DDRB = 0xFF;
	PORTB = 0x00;
	DDRC = 0x00;
	PORTC = 0xFF;
	TimerSet(300);
	TimerOn();
	state = INIT;
	//PORTB = ROW8B| COL2B | COL3B | COL5B;
	//PORTA = COL1A | COL4A;
	while (1){
		// User code
		button = ~PINC & 0x01;
		while(!TimerFlag); // Wait 3 sec
		TimerFlag = 0;
		tickFunc();
		if (stack_inc == 6){
			display(a_pins[0], b_pins[0]);
		}
		else if (stack_inc == 5){
			display(a_pins[0], b_pins[0]);
			display(a_pins[1], b_pins[1]);
		}
		else if (stack_inc == 4){
			display(a_pins[0], b_pins[0]);
			display(a_pins[1], b_pins[1]);
			display(a_pins[2], b_pins[2]);

		}
		else if (stack_inc == 3){
			display(a_pins[0], b_pins[0]);
			display(a_pins[1], b_pins[1]);
			display(a_pins[2], b_pins[2]);
			display(a_pins[3], b_pins[3]);
		}
		else if (stack_inc == 2){
			display(a_pins[0], b_pins[0]);
			display(a_pins[1], b_pins[1]);
			display(a_pins[2], b_pins[2]);
			display(a_pins[3], b_pins[3]);
			display(a_pins[4], b_pins[4]);
		}
		else if (stack_inc == 1){
			display(a_pins[0], b_pins[0]);
			display(a_pins[1], b_pins[1]);
			display(a_pins[2], b_pins[2]);
			display(a_pins[3], b_pins[3]);
			display(a_pins[4], b_pins[4]);
			display(a_pins[5], b_pins[5]);
		}
		// Note: For the above a better style would use a synchSM with TickSM()
		// This example just illustrates the use of the ISR and flag
	}
}

