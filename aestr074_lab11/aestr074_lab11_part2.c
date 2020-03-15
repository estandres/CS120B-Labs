/*
 *	Partner: Steven Strickland sstri014@ucr.edu SID:862155853
 *	Lab Section: 22
 *	Assignment: Lab 10 Exercise 2
*/
#include <avr/io.h>
#include "keypad.h"
#include "io.c"
#include "timer.h"
#include "scheduler.h"

unsigned char counter = 1;
unsigned char i;
const unsigned char text[70]={' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','C','S','1','2','0','B',' ','i','s',' ','L','e','g','e','n','d','.','.','.',' ','w','a','i','t',' ','f','o','r',' ','i','t',' ','D','A','R','Y','!',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
unsigned int tasksPeriod = 2;

enum states{state_start};
int tick(int state)
{
	switch(state)
	{
		case state_start:
		for (i = 1; i <= 16; i++)
		{
			LCD_Cursor(i);
			LCD_WriteData(text[counter + i - 2]);
			if (counter+i+1 == 70)
			{
				counter = 1;
			}
		}
		counter++;
		break;
	}
	return state;
}
int main() 
{
	DDRA = 0xF0; PORTA = 0x0F;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	static task task1;
	task *tasks[] = { &task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = state_start;
	task1.period = tasksPeriod;
	task1.elapsedTime = tasksPeriod;
	task1.TickFct = &tick;

	TimerSet(200);
	TimerOn();
	LCD_init();

	unsigned short i;
	while(1) {
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}

