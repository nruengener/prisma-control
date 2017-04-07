#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "stdlib.h"
#include "math.h"
#include "uart.h"
#include "bgc_io.h"
#include "blmotor.h"
#include "config.h"

#define 	F_CPU   16000000UL

/*
 * two_timer_example.c
 *
 *  Created on: 03.04.2017
 */

extern MotorControlState motorControlState;


void setup_timers() {

	// Timer 0 -> control
	TCNT0 = 0;
	TCCR0A |= (1 << WGM21); // Configure timer 2 for CTC mode
	TCCR0B |= (1 << CS01) | (1 << CS00); // prescaler 64
	TIMSK0 |= (1 << OCIE2A); // Enable CTC interrupt
	OCR0A = 50 - 1; // 5000Hz, 200us initial frequeny

	// Timer 1 and 2 -> PWM for motor
	// timer setup for 31.250KHZ phase correct PWM
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
	TCCR1B = (1 << CS10);
	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM20);
	TCCR2B = (1 << CS20);

	//turn off all PWM signals
	OCR2A = 0;  //11  APIN
	OCR1A = 0;  //D9  CPIN
	OCR1B = 0;  //D10 BPIN

	// enable Timer 1 overflow interrupt // TODO: needed?
	//TIMSK1 = 0;
	//TIMSK1 |= (1<<TOIE1);

}

void setup() {

	cli();
	uart_stdio();
	bl_setup();
	setup_timers();
	sei();

}

volatile uint8_t u = 0; // byte
volatile uint8_t v = 0;
volatile uint8_t w = 0;

//ISR( TIMER1_OVF_vect)   // overflow triggers at BOTTOM, update all the compare regs, which are sampled at TOP
//{
////	OCR1A = u;
////	OCR1B = v;
////	OCR2A = w;
//	OCR1A = w;
//	OCR1B = v;
//	OCR2A = u;
//}



int main() {
	setup();

	motorControlState.desiredSpeed = 1200;

	while (1) {
//		phase += freq;
//		int newu = my_cosine(phase);
//		int newv = my_cosine(phase - DEG_120);
//		int neww = -newu - newv;
//		newu += HALF;
//		newv += HALF;
//		neww += HALF;
//		u = newu;  // no masking of interrupts needed as u,v,w variables are single byte each.
//		v = newv;
//		w = neww;

//		printf("u: %i, v: %i, w: %i\n", u, v, w);
		 motorControl();
		_delay_us(200);

	}
	return 0;
}



