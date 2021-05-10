/*	Author: Jaired Jawed
 *  	Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab 9  Exercise 3
 *	Exercise Description:
 *
 *
 *	Demo Link:
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


volatile unsigned char TimerFlag = 0;
void TimerISR() { TimerFlag = 1;}

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet (unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short) (8000000 / (128 * frequency)) -1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}


double notes[17] = {
	392.00,
	0.0,
        392.00,
	0.0,
	392.00,
	0.0,
	329.63,
	0.0,
	440.00,
	0.0,
	392.00,
	0.0,
	329.63,
	0.0,
	493.88,
	0.0,
	329.00
};

int times[17] = {
	5,
	6,
	10,
	11,
	15,
	16,
	20,
	21,
	24,
	25,
	27,
	28,
	31,
	32,
	35,
	36,
	37
};

int endSongTime = 38;
int idx = 0;
int counter = 0;


enum States { Init, Wait, Reset } state;

void SM_Tick() {
	unsigned char button1 = (~PINA & 0x01) == 0x01;

	switch(state) {	
		case Init:
			if (button1) {
				state = Wait;
			}
			else {
				state = Init;
			}
			break;
		case Reset:
			if (!button1) {
				state = Init;
			}
			else {
				state = Reset;
			}
			break;
		case Wait:
			// after all the time slots from the song are done,
			// reset so that it can be played again once the song is over
			if (counter <= endSongTime) {
				state = Wait;
			}
			else {
				state = Reset;
			}
			break;
		default:
			state = Init;
			break;
	}


	switch (state) {
		case Init:
			set_PWM(0);
			break;
		case Reset:
			// reset everything back to their default values
			set_PWM(0);
			counter = 0;
			idx = 0;
			break;
		case Wait:
			// play the note			
			set_PWM(notes[idx]);

			// increment index when it's time to move onto the next note
			if (counter > times[idx] && idx < 17) {
				idx++;
			}
				
			counter++;
			break;
		default:
			break;
	}
}

int main() {
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    
    TimerSet(100);
    TimerOn();
    PWM_on();

    state = Init;

    while (1) {
	SM_Tick();
	while(!TimerFlag) {};
	TimerFlag = 0;
    }

    return 1;
}
