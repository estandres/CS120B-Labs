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
#include <util/delay.h>

// 0.954 hz is lowest frequency possible with this function
//  based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

unsigned char GetBit(unsigned char port, unsigned char number)
{
	return ( port & (0x01 << number) );
}
typedef enum {false, true} bool;

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))

#define CLOCK  2 // Output
#define LATCH  1 // Output
#define DATA   0 // Input

enum inputs {none, B, Y, Select, Start, Up, Down, Left, Right, A, X, L, R};

unsigned short GetSNESIn()
{
	unsigned short data = 0x0000;
	
	//request data from controller
	PORTC = SetBit(PINC, LATCH, 1);
	_delay_us(12);
	PORTC = SetBit(PINC, LATCH, 0);
	_delay_us(6);

	//read data
	unsigned char it;
	for (it = 0; it < 12; it++) {
		if(!GetBit(PINC, DATA)) {
			SET_BIT(data, it);
			} else {
			CLR_BIT(data, it);
		}
		
		//tick clock and wait for next bit
		PORTC = SetBit(PINC, CLOCK, 1);
		_delay_us(6);
		PORTC = SetBit(PINC, CLOCK, 0);
		_delay_us(6);
	}

	return data;
}

void mapPlayerInput(unsigned short rawInput, unsigned char* ins) {
	unsigned char count = 0;
	unsigned char it;
	for(it = 0; it < 12; it++) {
		if(GET_BIT(rawInput, it)) {
			ins[count] = it + 1;
			count++;
		}
	}
	ins[count] = 0;
}

bool inputContains(unsigned char* ins, unsigned char input) {
	unsigned char it;
	for(it = 0; it < 12; it++) {
		if(ins[it] == input) {
			return true;
			} else if(ins[it] == 0) {
			return false;
		}
	}
	return false;
}

unsigned char ins[12];

/****************************************************************/
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
const unsigned short tasksNum = 2;
unsigned int tasksPeriod = 1; //GCD of all task periods
//globals
unsigned char GL_A;
unsigned char GL_B;
unsigned char endReached = 0;
unsigned short button = 0x0000;
unsigned char stack_inc = 7;
unsigned char curr_row = 0x00;
unsigned char frames_to_match [5];
unsigned char frames_missed = 0;
unsigned char init_frame;
unsigned char curr_frame = 0;
unsigned char new_frame = 0;
unsigned long speed = 300;
unsigned char row[7] = {ROW2A, ROW3B, ROW4A, ROW5B, ROW6B, ROW7B, ROW8B};
unsigned char col_shapes_a[6] = {COL1A | COL4A,
								COL1A | COL4A | COL8A, 
								COL1A | COL7A | COL8A, 
								COL1A | COL6A | COL7A | COL8A, 
								COL1A | COL6A | COL7A | COL8A,
								COL4A | COL6A | COL7A | COL8A};
unsigned char col_shapes_b[6] = {COL2B | COL3B | COL5B,
								 COL2B | COL3B,
								 COL2B| COL3B, 
								 COL2B, 
								 COL5B,
								 COL5B};
unsigned char a_pins[7], b_pins[7];
unsigned char i = 0;
enum GL_States{GL_INIT, GL_CYCLE, GL_CYCLE_GOOD, GL_CYCLE_BAD, GL_LOSE_STATE, GL_WIN_STATE, GL_BUTTON}; //TODO: FINISH GL_CYCLE_BAD & GL_CYCLE_GOOD & GL_LOSE_STATE
int TickFunc_GameLogic(int state){
	//button = ~PINC & 0x01;
	button = GetSNESIn();
	mapPlayerInput(button, ins);
	switch (state){
		case GL_INIT:
			state = GL_CYCLE;
		break;
		case GL_CYCLE:
			if(inputContains(ins, A)){ 
				state = GL_BUTTON;
			}
			else if (inputContains(ins, Select)){
				state = GL_CYCLE;
				stack_inc=7;
				frames_missed = 0;
			}
			else if(stack_inc == 0){
				state = GL_WIN_STATE;
			}
			else{
				state = GL_CYCLE;
			}
		break;
		case GL_CYCLE_GOOD:
			if(inputContains(ins, A)){
				state = GL_BUTTON;
			}
			else if (inputContains(ins, Select)){
				state = GL_CYCLE;
				stack_inc=7;
				frames_missed = 0;
			}
			else if(stack_inc == 0){
				state = GL_WIN_STATE;
			}
			else{
				state = GL_CYCLE_GOOD;
			}
		break;
		case GL_CYCLE_BAD:
			if(inputContains(ins, A)){
				state = GL_BUTTON;
			}
			else if (inputContains(ins, Select)){
				state = GL_CYCLE;
				stack_inc=7;
				frames_missed = 0;
			}
			else if(stack_inc == 0){
				state = GL_WIN_STATE;
			}
			else{
				state = GL_CYCLE_BAD;
			}	
		break;
		case GL_LOSE_STATE:
			if (inputContains(ins, Select)){
				state = GL_CYCLE;
				stack_inc=7;
				frames_missed = 0;
			}
			else{
				state = GL_LOSE_STATE;
			}
		break;
		case GL_WIN_STATE:
			if (inputContains(ins, Start)){
				state = GL_CYCLE;
				stack_inc=7;
				frames_missed = 0;
			}
			else{
				state = GL_WIN_STATE;
			}
		break;
		case GL_BUTTON:
			if(inputContains(ins, A)){
				state = GL_BUTTON ;
			}
			else{
				if(frames_missed == 0){
					state = GL_CYCLE;
				}
				else if(frames_missed == 1){
					state = GL_CYCLE_GOOD;
				}
				else if (frames_missed == 2){
					state = GL_CYCLE_BAD;
				}
				else {
					state = GL_LOSE_STATE;
				}
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
			if (endReached == 1){i--;}
			else{i++;}
			curr_row = row[stack_inc-1];
			if(curr_row == ROW2A || curr_row == ROW4A){
				if (i == 1){
					GL_B = COL2B | COL3B | COL5B;//||12345   ||
					GL_A = curr_row | COL1A | COL4A;
					endReached = 0;
				}
				else if(i == 2){
					GL_B = COL2B | COL3B;//||1234   8||
					GL_A = curr_row | COL1A | COL4A | COL8A;
				}
				else if(i == 3){
					GL_B = COL2B | COL3B;//||123   78||
					GL_A = curr_row | COL1A | COL7A | COL8A;
				}
				else if(i == 4){
					GL_B = COL2B;			//||12   678||
					GL_A = curr_row | COL1A | COL6A | COL7A | COL8A;
				}
				else if(i == 5){
					GL_B = COL5B;//||1   5678||
					GL_A = curr_row | COL1A | COL6A | COL7A |COL8A;
				}
				else if(i == 6){
					GL_B = COL5B;//||  45678||
					GL_A = curr_row | COL4A | COL6A | COL7A | COL8A;
					endReached = 1;
				}
			}
			else if (curr_row == ROW3B || curr_row == ROW5B || curr_row == ROW6B || curr_row == ROW7B || curr_row == ROW8B){
				if (i == 1){
					GL_B = curr_row | COL2B | COL3B | COL5B;//||12345   ||
					GL_A = COL1A | COL4A;
					endReached = 0;
				}
				else if(i == 2){
					GL_B = curr_row | COL2B | COL3B;//||1234   8||
					GL_A = COL1A | COL4A | COL8A;
				}
				else if(i == 3){
					GL_B = curr_row | COL2B | COL3B;//||123   78||
					GL_A = COL1A | COL7A | COL8A;
				}
				else if(i == 4){
					GL_B = curr_row | COL2B;			//||12   678||
					GL_A = COL1A | COL6A | COL7A | COL8A;
				}
				else if(i == 5){
					GL_B = curr_row | COL5B;//||1   5678||
					GL_A = COL1A | COL6A | COL7A |COL8A;
				}
				else if(i == 6){
					GL_B = curr_row | COL5B;//||  45678||
					GL_A = COL4A | COL6A | COL7A | COL8A;
					endReached = 1;
				}
			}
			//else {stack_inc = 7;}
			PORTA = GL_A;
			PORTB = GL_B;
		break;
		case GL_CYCLE_GOOD:
		if (endReached == 1){i--;}
		else{i++;}
		curr_row = row[stack_inc-1];
		if(curr_row == ROW2A || curr_row == ROW4A){
			if (i == 1){
				GL_B = COL2B | COL3B | COL5B ;//||123456   ||
				GL_A = curr_row | COL1A | COL4A | COL6A;
				endReached = 0;
			}
			else if(i == 2){
				GL_B = COL2B | COL3B | COL5B;//||12345   8||
				GL_A = curr_row | COL1A | COL4A | COL8A;
			}
			else if(i == 3){
				GL_B = COL2B | COL3B;//||1234   78||
				GL_A = curr_row | COL1A | COL4A | COL7A | COL8A;
			}
			else if(i == 4){
				GL_B = COL2B | COL3B;			//||123   678||
				GL_A = curr_row | COL1A | COL6A | COL7A | COL8A;
			}
			else if(i == 5){
				GL_B = COL5B | COL2B;//||12   5678||
				GL_A = curr_row | COL1A | COL6A | COL7A |COL8A;
			}
			else if(i == 6){
				GL_B = COL5B;//||1  45678||
				GL_A = curr_row | COL1A | COL4A | COL6A | COL7A | COL8A;
				//endReached = 1;
			}
			else if(i == 7){
				GL_B = COL5B | COL3B;//||  345678||
				GL_A = curr_row | COL4A | COL6A | COL7A | COL8A;
				endReached = 1;
			}
		}
		else if (curr_row == ROW3B || curr_row == ROW5B || curr_row == ROW6B || curr_row == ROW7B || curr_row == ROW8B){
			if (i == 1){
				GL_B = curr_row | COL2B | COL3B | COL5B ;//||123456   ||
				GL_A = COL1A | COL4A | COL6A;
				endReached = 0;
			}
			else if(i == 2){
				GL_B = curr_row | COL2B | COL3B | COL5B;//||12345   8||
				GL_A = COL1A | COL4A | COL8A;
			}
			else if(i == 3){
				GL_B = curr_row | COL2B | COL3B;//||1234   78||
				GL_A = COL1A | COL4A | COL7A | COL8A;
			}
			else if(i == 4){
				GL_B = curr_row | COL2B | COL3B;			//||123   678||
				GL_A = COL1A | COL6A | COL7A | COL8A;
			}
			else if(i == 5){
				GL_B = curr_row | COL5B | COL2B;//||12   5678||
				GL_A = COL1A | COL6A | COL7A |COL8A;
			}
			else if(i == 6){
				GL_B = curr_row | COL5B;//||1  45678||
				GL_A = COL1A | COL4A | COL6A | COL7A | COL8A;
				//endReached = 1;
			}
			else if(i == 7){
				GL_B = curr_row | COL5B | COL3B;//||  345678||
				GL_A = COL4A | COL6A | COL7A | COL8A;
				endReached = 1;
			}
		}
		//else {stack_inc = 7;}
		PORTA = GL_A;
		PORTB = GL_B;
		break;
		case GL_CYCLE_BAD:
		if (endReached == 1){i--;}
		else{i++;}
		curr_row = row[stack_inc-1];
		if(curr_row == ROW2A || curr_row == ROW4A){
			if (i == 1){
				GL_B = COL2B | COL3B | COL5B ;//||1234567 ||
				GL_A = curr_row | COL1A | COL4A | COL6A | COL7A;
				endReached = 0;
			}
			else if(i == 2){
				GL_B = COL2B | COL3B | COL5B;//||123456 8||
				GL_A = curr_row | COL1A | COL4A | COL6A | COL8A;
			}
			else if(i == 3){
				GL_B = COL2B | COL3B | COL5B;//||12345 78||
				GL_A = curr_row | COL1A | COL4A | COL7A | COL8A;
			}
			else if(i == 4){
				GL_B = COL2B | COL3B;	//||1234 678||
				GL_A = curr_row | COL1A | COL4A | COL6A | COL7A | COL8A;
			}
			else if(i == 5){
				GL_B = COL5B | COL2B | COL3B;//||123 5678||
				GL_A = curr_row | COL1A | COL6A | COL7A |COL8A;
			}
			else if(i == 6){
				GL_B = COL5B | COL2B;//||12 45678||
				GL_A = curr_row | COL1A | COL4A | COL6A | COL7A | COL8A;
				//endReached = 1;
			}
			else if(i == 7){
				GL_B = COL5B | COL3B;//||1 345678||
				GL_A = curr_row | COL1A | COL4A | COL6A | COL7A | COL8A;
			}
			else if (i == 8){
			
				GL_B = COL5B | COL2B | COL3B;//|| 2345678||
				GL_A = curr_row | COL4A | COL6A | COL7A | COL8A;
				endReached = 1;
			}
		}
		else if (curr_row == ROW3B || curr_row == ROW5B || curr_row == ROW6B || curr_row == ROW7B || curr_row == ROW8B){
			if (i == 1){
				GL_B = curr_row | COL2B | COL3B | COL5B ;//||1234567 ||
				GL_A = COL1A | COL4A | COL6A | COL7A;
				endReached = 0;
			}
			else if(i == 2){
				GL_B = curr_row | COL2B | COL3B | COL5B;//||123456 8||
				GL_A = COL1A | COL4A | COL6A | COL8A;
			}
			else if(i == 3){
				GL_B = curr_row | COL2B | COL3B | COL5B;//||12345 78||
				GL_A = COL1A | COL4A | COL7A | COL8A;
			}
			else if(i == 4){
				GL_B = curr_row | COL2B | COL3B;	//||1234 678||
				GL_A = COL1A | COL4A | COL6A | COL7A | COL8A;
			}
			else if(i == 5){
				GL_B = curr_row | COL5B | COL2B | COL3B;//||123 5678||
				GL_A = COL1A | COL6A | COL7A |COL8A;
			}
			else if(i == 6){
				GL_B = curr_row | COL5B | COL2B;//||12 45678||
				GL_A = COL1A | COL4A | COL6A | COL7A | COL8A;
			}
			else if(i == 7){
				GL_B = curr_row | COL5B | COL3B;//||1 345678||
				GL_A = COL1A | COL4A | COL6A | COL7A | COL8A;
			}
			else if (i == 8){
				
				GL_B = curr_row | COL5B | COL2B | COL3B;//|| 2345678||
				GL_A = COL4A | COL6A | COL7A | COL8A;
				endReached = 1;
			}
		}
	//	else {stack_inc = 7;}
		PORTA = GL_A;
		PORTB = GL_B;
		break;
		case GL_LOSE_STATE:
			PORTA = GL_A;
			PORTB = GL_B;
		break;
		case GL_WIN_STATE:
			PORTA = COL1A | COL4A | COL6A | COL7A | COL8A;
			PORTB = COL2B | COL3B | COL5B;
		break;
		case GL_BUTTON: 
			curr_frame = i; //store the current frame when button is pressed
			if (stack_inc <= 7){	//store the shape of the LEDS
				a_pins [7-stack_inc] = GL_A;
				b_pins [7-stack_inc] = GL_B;
			}
			
			if (stack_inc == 7){			//determine within which frame the stack starts
				frames_to_match[0] = curr_frame-2;
				frames_to_match[1] = curr_frame-1;
				frames_to_match[2] = curr_frame;	//store the frames that are valid for the second stack
				frames_to_match[3] = curr_frame+1;
				frames_to_match[4] = curr_frame+2;

				init_frame = curr_frame;	//store initial starting frame
			}
			else{	// behavior of every other stack besides the first (THE IMPORTANT STUFF).
				if ( i == frames_to_match[2]){
					//perfect, stay in current state
				}
				else{	//not perfect, player has missed to some degree
					if (curr_frame >= init_frame){
						new_frame = curr_frame; //update which frame the player needs to match to
					}
					else if (curr_frame == init_frame-1){
						new_frame = curr_frame+1;
					}
					else if (curr_frame == init_frame-2){
						new_frame = curr_frame+2;
					}
					switch(frames_missed){
						case 0:
							init_frame = new_frame; // replace initial starting frame with the new frame.
							if ((i == frames_to_match[3]) || (i == frames_to_match[1])){
								//state = GL_CYCLE_GOOD;
								frames_missed ++;
							}
							else if ((i == frames_to_match[4]) || (i == frames_to_match[0])){
								//state = GL_CYCLE_BAD;
								frames_missed += 2;
							}
							else{
								frames_missed += 3;
								//	state = GL_LOSE_STATE;
							}
						break;
						case 1:
							init_frame = new_frame; // replace initial starting frame with the new frame.
							if( (i == frames_to_match[3]) || (i == frames_to_match[1])){
								//		state = GL_CYCLE_BAD;
								frames_missed ++;
							}
							else{
								frames_missed += 3 ;
							}
						break;
						default:
							frames_missed += 3;
						break;
					}
					//TODO: Fix frame logic
					frames_to_match[0] = new_frame-2;
					frames_to_match[1] = new_frame-1;
					frames_to_match[2] = new_frame;
					frames_to_match[3] = new_frame+1;
					frames_to_match[4] = new_frame+2;
				}
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
			//state = CL_DISPLAY;
		break;
		case CL_DISPLAY:
		break;
	}
	switch (state){
		case CL_START:
		break;
		case CL_DISPLAY:
		//	PORTA = GL_A;
	//		PORTB = GL_B;
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

void display(unsigned char a_bits, unsigned char b_bits){
	for (int it = 0; it < 100; it++){
		PORTA = a_bits;
		PORTB = b_bits;
	}
}

int main(){
	DDRA = 0xFF;
	PORTA = 0x00;
	DDRB = 0xFF;
	PORTB = 0x00;
	//DDRC = 0x00;
	//PORTC = 0xFF;
	DDRC = 0xFE; PORTC = 0x01;
	unsigned char j = 0;
	tasks[j].state = GL_INIT;
	tasks[j].period = speed;
	tasks[j].elapsedTime = 0;
	tasks[j].TickFunc = &TickFunc_GameLogic;
	j++;
	tasks[j].state = CL_START;
	tasks[j].period = 1;
	tasks[j].elapsedTime = 0;
	tasks[j].TickFunc = &TickFunc_CombinedLEDs;
	TimerSet(tasksPeriod);
	TimerOn();
	while(1){
		if (stack_inc == 6){
			display(a_pins[0], b_pins[0]);
		}
		else if (stack_inc == 5){
			display(a_pins[0], b_pins[0]); display(a_pins[1], b_pins[1]);
		}
		else if (stack_inc == 4){
			display(a_pins[0], b_pins[0]); display(a_pins[1], b_pins[1]); display(a_pins[2], b_pins[2]);

		}
		else if (stack_inc == 3){
			display(a_pins[0], b_pins[0]); display(a_pins[1], b_pins[1]); display(a_pins[2], b_pins[2]); display(a_pins[3], b_pins[3]);
		}
		else if (stack_inc == 2){
			display(a_pins[0], b_pins[0]); display(a_pins[1], b_pins[1]); display(a_pins[2], b_pins[2]); display(a_pins[3], b_pins[3]); display(a_pins[4], b_pins[4]);
		}
		else if (stack_inc == 1){
			display(a_pins[0], b_pins[0]); display(a_pins[1], b_pins[1]); display(a_pins[2], b_pins[2]); display(a_pins[3], b_pins[3]); display(a_pins[4], b_pins[4]); display(a_pins[5], b_pins[5]);
		}
		else if(stack_inc == 0){
			display(a_pins[0], b_pins[0]); display(a_pins[1], b_pins[1]); display(a_pins[2], b_pins[2]); display(a_pins[3], b_pins[3]); display(a_pins[4], b_pins[4]); display(a_pins[5], b_pins[5]); display(a_pins[6], b_pins[6]);
	
		}
		//PORTA = GL_A;
		//PORTB = GL_B;
		display(GL_A, GL_B);
	}
	return 0;
}