#ifndef BLMOTOR_H
#define BLMOTOR_H

typedef struct {
	int8_t enabled;
	int8_t brake;
	int8_t accel;
	int8_t decel;
	int8_t direction;
	int8_t desiredDirection;
	int16_t speed;
	int16_t desiredSpeed;
	uint16_t power;
} MotorControlState;

void MoveMotors(uint8_t motorNumber, uint8_t posStep, uint16_t power);
void motorPowerOff();
void bl_setup();
void commutate();
void motorControl();
void initControlState();

#endif
