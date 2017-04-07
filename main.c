#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "stdlib.h"
#include "math.h"
#include "util/fasttrigo.h"
#include "mpu6050/mpu6050registers.h"
#include "mpu6050/mpu6050.h"
#include "uart.h"
#include "bgc_io.h"
#include "blmotor.h"
#include "kalman/kalman.h"
#include "config.h"

#define 	F_CPU   16000000UL

/*
 * two_timer_example.c
 *
 *  Created on: 03.04.2017
 */

extern MotorControlState motorControlState;

Kalman kalX;
Kalman kalY;
Kalman kalZ;

volatile uint8_t updatePosition = 0;
volatile uint8_t debugPrint = 0;

uint8_t warnFlag = 0;

void initKalmanFilters();
void updateKalmanFilters(int delay);

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
	TIMSK1 = 0;
	TIMSK1 |= (1 << TOIE1);

}

void setup() {

	cli();
	uart_stdio();
	mpu6050_init();
	initKalmanFilters();
	_delay_ms(10);
	bl_setup();
	setup_timers();
	sei();

}

void resetMotor() {
	initControlState();
}

int main() {
	setup();

	for (int i = 0; i < 255; i++) { // wait for some calibration
		_delay_ms(10);
		updateKalmanFilters(10);
	}

	motorControlState.desiredSpeed = 1200;
	motorControlState.enabled = 1;

	while (1) {
		motorControl();

		// wait in case of warning
		if (warnFlag) {
			resetMotor();
			_delay_ms(2000);
			warnFlag = 0;
		}

		if (updatePosition) {
			updateKalmanFilters(10);
			updatePosition = 0;

			// position dependent actions
			if (abs(kalY.angle) < 20) {
				// set flag
				warnFlag = 1;
				// broad side -> all off
				motorControlState.enabled = 0;
			}

			if (abs(kalY.angle) > 30 && !warnFlag) {
				motorControlState.enabled = 1;
				motorControlState.desiredSpeed = 1200;
			}

		}

		if (debugPrint) {
			debugPrint = 0;
//			printf("kalZ.angle: %.2f, kalZ.rate: %.2f\n", kalZ.angle, kalZ.rate);
//			printf("kalY.angle: %.2f, kalY.rate: %.2f\n", kalY.angle, kalY.rate);
			printf("kalX.angle: %.2f, kalY.angle: %.2f\n", kalX.angle, kalY.angle);
		}

		_delay_us(200);
	}
	return 0;
}

void initKalmanFilters() {
	initKalman(&kalX);
	initKalman(&kalY);
	initKalman(&kalZ);
	double axg = 0;
	double ayg = 0;
	double azg = 0;
	double gxds = 0;
	double gyds = 0;
	double gzds = 0;
	// initial values
	mpu6050_getConvData(&axg, &ayg, &azg, &gxds, &gyds, &gzds);
	int8_t sign = azg > 0 ? 1 : -1;
	kalX.angle = (FT_Atan2(ayg, sign * FT_Sqrt(0.001 * axg * axg + azg * azg)) - PI) * RAD_TO_DEG;
	kalY.angle = (FT_Atan2(-axg, azg) - PI) * RAD_TO_DEG;
	kalZ.angle = (FT_Atan2(axg, ayg) - PI) * RAD_TO_DEG;

	// test z angle init var
//	kalZ.P[0][0] = INIT_VARIANCE_ANGLE;
//	kalZ.P[0][1] = INIT_VARIANCE_ANGLE;
//	kalZ.P[1][0] = INIT_VARIANCE_ANGLE;
//	kalZ.P[1][1] = INIT_VARIANCE_ANGLE;
}

/** Updates the kalman filters. Delay is given in ms. */
void updateKalmanFilters(int delay) {
	double axg = 0;
	double ayg = 0;
	double azg = 0;
	double gxds = 0;
	double gyds = 0;
	double gzds = 0;
	float dt = (float) delay / 1000;
	mpu6050_getConvData(&axg, &ayg, &azg, &gxds, &gyds, &gzds);

	int8_t sign = azg > 0 ? 1 : -1;
	float xAngle = (FT_Atan2(ayg, sign * FT_Sqrt(0.001 * axg * axg + azg * azg)) - PI) * RAD_TO_DEG; // correct
	float yAngle = (atan2f(-axg, FT_Sqrt(ayg * ayg + azg * azg))) * RAD_TO_DEG;
	float zAngle = (FT_Atan2(axg, ayg) - PI) * RAD_TO_DEG;
// offset
	zAngle += Z_ANGLE_OFFSET;
	if (zAngle < -180)
		zAngle = 360 + zAngle;

	if ((xAngle < -90 && kalX.angle > 90) || (xAngle > 90 && kalX.angle < -90)) {
		kalX.angle = xAngle;
	} else {
		updateAngle(&kalX, xAngle, gxds, dt);
	}

	// pitch is used to detect if prisma lies on broad side -> abs(kalY.angle) < 60 : motor off
	// TODO: maybe use also roll (depends on orientation of sensor)
	if ((yAngle < -90 && kalY.angle > 90) || (yAngle > 90 && kalY.angle < -90)) {
		kalY.angle = yAngle;
	} else {
		updateAngle(&kalY, yAngle, gyds, dt); // use roll or pitch?
	}

	// yaw represents angle of prisma if attached flat to surface
	if ((zAngle < -90 && kalZ.angle > 90) || (zAngle > 90 && kalZ.angle < -90)) {
		kalZ.angle = zAngle;
	} else {
		updateAngle(&kalZ, zAngle, gzds, dt);
	}

}

volatile int freqCounter = 0;
volatile int msCounter = 0;

/********************************/
/* Control IRQ Routine    */
/********************************/
// is called every 31.5us (32)
// minimize interrupt code (20 instructions)
ISR( TIMER1_OVF_vect ) {
	freqCounter++;

	// every 1 ms
	if ((freqCounter & 0x01f) == 0) {
		msCounter++;
	}

	// every 10 ms
	if ((freqCounter & 0x01f) == 0 && msCounter % 10 == 0) {
		updatePosition = 1;
	}

	// every 1000ms
	if ((freqCounter & 0x01f) == 0 && msCounter % 1000 == 0) {
		msCounter = 0;
		debugPrint = 1;
	}
}

