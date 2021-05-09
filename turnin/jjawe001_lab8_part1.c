/*	Author: Jaired Jawed
 *	Lab Section:
 *	Assignment: Lab 8  Exercise 1
 *	Exercise Description:
 *
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


enum States { Init, C_4, D_4, E_4, Stop } state;

void SM_Tick() {
	switch (state) {
		case Init:
			if ((~PINA & 0x07) == 0x01) {
				state = C_4;
			}
			else if ((~PINA & 0x07) == 0x02) {
				state = D_4;
			}	
			else if ((~PINA & 0x07) == 0x04) {
				state = E_4;
			}
			else {
				state = Stop;
			}
			break;
		case Stop:
			state = Init;
			break;
		case C_4:
			if ((~PINA & 0x07) != 0x01) {
				state = Init;
			}
			else {
				state = C_4;
			}
			break;
		case D_4:
			if ((~PINA & 0x07) != 0x02) {
				state = Init;
			}
			else {
				state = D_4;
			}
			break;
		case E_4:
			if ((~PINA & 0x07) != 0x04) {
				state = Init;
			}
			else {
				state = E_4;
			}
			break;
	}

	switch (state) {
		case Stop:
		case Init:
			set_PWM(0);
			break;

		case C_4:
			set_PWM(261.63);
			break;
		
		case D_4:
			set_PWM(293.66);
			break;

		case E_4:
			set_PWM(329.63);
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
