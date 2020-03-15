/*
 *	Partner: Steven Strickland sstri014@ucr.edu SID:862155853
 *	Lab Section: 22
 *	Assignment: Lab 10 Exercise 2
*/

#include <avr/io.h>
#include "keypad.h"
#include "scheduler.h"
#include "timer.h"
#include "io.c"
#include "io.h"

unsigned int tasksperiod = 5;
unsigned char lcd = 0x00;


enum statemachine{state_start};
int keypad_LCD(int state)
{
	unsigned char x;
	x = GetKeypadKey();
	switch(state)
	{
		case state_start:
		switch (x)
		{
			case '\0':
			break;
			case '1':
				//lcd = 0x01;
				LCD_Cursor(1);
				LCD_WriteData(1 + '0');
			break;
			case '2':
				//lcd = 0x32;
				LCD_Cursor(1);
				LCD_WriteData(2 + '0');
			break;
			case '3':
				LCD_Cursor(1);
				LCD_WriteData(3 + '0');
			break;
			case '4':
				LCD_Cursor(1);
				LCD_WriteData(4 + '0');
			break;
			case '5':
				LCD_Cursor(1);
				LCD_WriteData(5 + '0');
			break;
			case '6':
				LCD_Cursor(1);
				LCD_WriteData(6 + '0');
			break;
			case '7':
				LCD_Cursor(1);
				LCD_WriteData(7 + '0');
			break;
			case '8':
				LCD_Cursor(1);
				LCD_WriteData(8 + '0');
			break;
			case '9':
				LCD_Cursor(1);
				LCD_WriteData(9 + '0');
			break;
			
			case 'A':
				lcd = 0x41;
				LCD_Cursor(1);
				LCD_WriteData(lcd);
			break;
			
			case 'B':
				lcd = 0x42;
				LCD_Cursor(1);
				LCD_WriteData(lcd);
			break;
			case 'C':
				lcd = 0x43;
				LCD_Cursor(1);
				LCD_WriteData(lcd);
			break;
			case 'D':
				lcd = 0x44;
				LCD_Cursor(1);
				LCD_WriteData(lcd);
			break;
			case '*':
				lcd = 0x2A;
				LCD_Cursor(1);
				LCD_WriteData(lcd);
			break;
			case '0':
				LCD_Cursor(1);
				LCD_WriteData(0 + '0');
			break;
			case '#':
				lcd = 0x23;
				LCD_Cursor(1);
				LCD_WriteData(lcd);
			break;
			default:
			break;
		}
		state = state_start;
		PORTC = lcd;
		break;

	}
	return state;
}

int main()
{
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0xF0; PORTA = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	
	static task task1;
	task *tasks[] = { &task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	task1.state = state_start;
	task1.period = tasksperiod;
	task1.elapsedTime = tasksperiod;
	task1.TickFct = &keypad_LCD;

	TimerSet(tasksperiod);
	TimerOn();
	
	LCD_init();
	
	unsigned short i;
	while(1) {
		for (i = 0; i < numTasks; i++)
		{
			if (tasks[i]->elapsedTime >= tasks[i]->period )
			{
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