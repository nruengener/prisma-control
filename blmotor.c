/*
 * blmotor.c
 *
 *  Created on: 14.02.2017
 *      Author: Tobias Ellermeyer
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "bgc_io.h"
#include "blmotor.h"
#include "config.h"

MotorControlState motorControlState;

// Space Vector PWM lookup table
// using uint8_t overflow for stepping
int8_t pwmSinMotor[] = { 0, 5, 10, 16, 21, 27, 32, 37, 43, 48, 53, 59, 64, 69, 74, 79, 84, 89, 94, 99, 104, 109, 111,
		112, 114, 115, 116, 118, 119, 120, 121, 122, 123, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127,
		127, 126, 126, 126, 125, 125, 124, 123, 123, 122, 121, 120, 119, 118, 116, 115, 114, 112, 111, 110, 112, 113,
		114, 116, 117, 118, 119, 120, 121, 122, 123, 124, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 126,
		126, 126, 125, 125, 124, 124, 123, 122, 121, 120, 119, 118, 117, 116, 114, 113, 112, 110, 106, 101, 97, 92, 87,
		82, 77, 72, 66, 61, 56, 51, 45, 40, 35, 29, 24, 18, 13, 8, 2, -2, -8, -13, -18, -24, -29, -35, -40, -45, -51,
		-56, -61, -66, -72, -77, -82, -87, -92, -97, -101, -106, -110, -112, -113, -114, -116, -117, -118, -119, -120,
		-121, -122, -123, -124, -124, -125, -125, -126, -126, -126, -127, -127, -127, -127, -127, -126, -126, -126,
		-125, -125, -124, -124, -123, -122, -121, -120, -119, -118, -117, -116, -114, -113, -112, -110, -111, -112,
		-114, -115, -116, -118, -119, -120, -121, -122, -123, -123, -124, -125, -125, -126, -126, -126, -127, -127,
		-127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -123, -122, -121, -120, -119, -118, -116,
		-115, -114, -112, -111, -109, -104, -99, -94, -89, -84, -79, -74, -69, -64, -59, -53, -48, -43, -37, -32, -27,
		-21, -16, -10, -5, 0 };

const float ocr_factor[] PROGMEM = { 5.5f, 5.1f, 4.7f, 4.35f, 4.05f, 3.75f, 3.5f, 3.25f, 3.05f, 2.85f, 2.65f, 2.5f,
		2.35f, 2.25f, 2.1f, 2.0f, 1.9f, 1.85f, 1.75f, 1.7f, 1.6f, 1.55f, 1.5f, 1.45f, 1.4f, 1.35f, 1.35f, 1.3f, 1.3f,
		1.25f, 1.25f, 1.2f, 1.2f, 1.15f, 1.15f, 1.15f, 1.1f, 1.1f, 1.1f, 1.1f, 1.1f, 1.05f, 1.05f, 1.05f, 1.05f, 1.05f,
		1.05f, 1.05f, 1.0f, 1.0f };

uint8_t ocr[50];

uint16_t MotorPower = 120; // 0 to 255, 255==100% power

uint8_t idl_motor_power = 80;

volatile uint8_t idx = 0;
volatile uint16_t tick = 0;

void setup_ocr_accel_values();

void initControlState() {
	motorControlState.brake = 0;
	motorControlState.desiredDirection = 1;
	motorControlState.direction = 1;
	motorControlState.enabled = 0;
	motorControlState.power = 255;
	motorControlState.speed = 0;
	motorControlState.desiredSpeed = 0;
	idx = 0;
	OCR0A = 50;
}

void setup_ocr_accel_values() {
	for (int i = 0; i < 50; i++) {
		ocr[i] = pgm_read_float(&ocr_factor[i]) * MAX_SPEED_OCR_VAL;
	}
}

//--------------------------------------------------------------------
//Motor control stuff-------------------------------------------------
//--------------------------------------------------------------------
void bl_setup() {
	setup_ocr_accel_values();

	// Set Motor Ports to output
//	MOT0_DDR |= (1 << MOT0A_BIT) | (1 << MOT0B_BIT) | (1 << MOT0C_BIT);
	MOT1_DDR |= (1 << MOT1A_BIT) | (1 << MOT1B_BIT) | (1 << MOT1C_BIT);

	initControlState();

	// switch off PWM Power
	motorPowerOff();
}

// switch off motor power
void motorPowerOff() {
	MoveMotors(1, 0, 0);
}

void MoveMotors(uint8_t motorNumber, uint8_t posStep, uint16_t power) {
	uint16_t pwm_a;
	uint16_t pwm_b;
	uint16_t pwm_c;

	// lookup sine values from table with 120deg. offsets
	pwm_a = pwmSinMotor[(uint8_t) posStep];
	pwm_b = pwmSinMotor[(uint8_t) (posStep + 85)];
	pwm_c = pwmSinMotor[(uint8_t) (posStep + 170)];

	// scale motor power
	pwm_a = power * pwm_a;
	pwm_a = pwm_a >> 8;
	pwm_a += 128;

	pwm_b = power * pwm_b;
	pwm_b = pwm_b >> 8;
	pwm_b += 128;

	pwm_c = power * pwm_c;
	pwm_c = pwm_c >> 8;
	pwm_c += 128;

	// set motor pwm variables
	if (motorNumber == 0) {
		PWM_A_MOTOR0 = (uint8_t) pwm_a;
		PWM_B_MOTOR0 = (uint8_t) pwm_b;
		PWM_C_MOTOR0 = (uint8_t) pwm_c;
	}

	if (motorNumber == 1) {
		PWM_A_MOTOR1 = (uint8_t) pwm_a;
		PWM_B_MOTOR1 = (uint8_t) pwm_b;
		PWM_C_MOTOR1 = (uint8_t) pwm_c;
	}
}

volatile uint8_t step = 0;
void commutate() {
	step++;
	uint16_t pwm_a;
	uint16_t pwm_b;
	uint16_t pwm_c;

	// lookup sine values from table with 120deg. offsets
	pwm_a = pwmSinMotor[(uint8_t) step];
	pwm_b = pwmSinMotor[(uint8_t) (step + 85)];
	pwm_c = pwmSinMotor[(uint8_t) (step + 170)];

	// scale motor power
	pwm_a = motorControlState.power * pwm_a;
	pwm_a = pwm_a >> 8;
	pwm_a += 128;

	pwm_b = motorControlState.power * pwm_b;
	pwm_b = pwm_b >> 8;
	pwm_b += 128;

	pwm_c = motorControlState.power * pwm_c;
	pwm_c = pwm_c >> 8;
	pwm_c += 128;

	// set motor pwm variables
	PWM_A_MOTOR1 = (uint8_t) pwm_a;
	if (motorControlState.direction > 0) {
		PWM_B_MOTOR1 = (uint8_t) pwm_b;
		PWM_C_MOTOR1 = (uint8_t) pwm_c;
	} else {
		PWM_C_MOTOR1 = (uint8_t) pwm_b;
		PWM_B_MOTOR1 = (uint8_t) pwm_c;
	}
}

volatile uint8_t doCommutate = 0;

void motorControl() {
	if (motorControlState.enabled) {
		if (!motorControlState.brake) {
			doCommutate = 1;
			if (motorControlState.desiredSpeed > motorControlState.speed) {
				motorControlState.accel = 1;
				motorControlState.decel = 0;
			} else if (motorControlState.desiredSpeed < motorControlState.speed) {
				motorControlState.accel = 0;
				motorControlState.decel = 1;
			} else {
				motorControlState.accel = 0;
				motorControlState.decel = 0;
			}
		}
	} else {
		motorPowerOff();
		doCommutate = 0;
	}

}

// keep short
ISR(TIMER0_COMPA_vect) {
	tick++;
	if (doCommutate) {
		commutate(); //MoveMotors(1, step, 255);
		if (motorControlState.accel && idx < 49 && tick % 200 == 0) {
			idx++;
			tick = 0;
			OCR0A = ocr[idx];
		} else if (motorControlState.decel && idx > 0 && tick % 200 == 0) {
			idx--;
			tick = 0;
			OCR0A = ocr[idx];
		}
	} else {
		motorPowerOff();
	}
}
