/*	Author: Jaired Jawed
 *	Lab Section:
 *	Assignment: Lab 8  Exercise 2
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


enum States { Init, ScaleUp, UpRelease, ScaleDown, DownRelease, ToggleSwitch } state;

// C4 = 261.63
// D4 = 293.66
// E4 = 329.63
// F4 = 329.23
// G4 = 392.00
// A4 = 440.00
// B4 = 493.88
// C5 = 523.25

double frequencies[8] = {
	261.63,
	293.66,
	329.66,
	329.23,
	392.00,
	440.00,
	493.88,
	523.25
};

unsigned char idx = 0x00;

// 1 when button1 and button2 can be pressed, 0 when it can't be pressed
unsigned char isOn = 1;

void SM_Tick() {
	unsigned char button1 = (~PINA & 0x07) == 0x01;
	unsigned char button2 = (~PINA & 0x07) == 0x02;
	unsigned char button3 = (~PINA & 0x07) == 0x04;

	switch (state) {
		case Init:
			if (button1) {
				state = ScaleUp;
			}
			else if (button2) {
				state = ScaleDown;
			}
			else if (button3) {
				state = ToggleSwitch;
			}
			break;
		case ScaleUp:
			state = UpRelease;
			break;
		case ScaleDown:
			state = DownRelease;
			break;
		case UpRelease:
			if (!button1) {
				state = Init;
			}
			break;
		case DownRelease:
			if (!button2) {
				state = Init;
			}
			break;
		case ToggleSwitch:
			// turn off
			//
			if (!button3) {
				state = Init;
			}
			break;
	}

	switch (state) {
		case ScaleUp:
			// stop incrementing if idx reaches the
			// length of the frequencies array - 1
			if (idx < 0x07 && isOn == 1) {
				idx++;
				set_PWM(frequencies[idx]);
			}
			break;
		case ScaleDown:
			if (idx > 0x00 && isOn == 1) {
				idx--;
				set_PWM(frequencies[idx]);
			}
			break;
		case ToggleSwitch:
			if (isOn == 1) {
				isOn = 0;
			}
			else {
				isOn = 1;
			}
			break;
		case DownRelease:
		case UpRelease:
		case Init:
			set_PWM(0);
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
