/*
 * config.h
 *
 *  Created on: 24.03.2017
 *      Author: Nando Rüngener
 */

#ifndef CONFIG_H_
#define CONFIG_H_

// motor data
#define MOTORUPDATE_FREQ 15625 // 31250, 15625, 10417
#define POLENUMBER 7
#define KV 80L
#define VOLTAGE 14
#define MAX_RPM_UNLOADED (KV*VOLTAGE) // unloaded! TODO: reduce
#define MAX_RPM (MAX_RPM_UNLOADED*0.5f)  //1.04f)
#define TIMER_US 4 // with OCR = 1
#define COM_STEPS_PER_TURN (POLENUMBER * 255) // 255 steps per commutation phase
#define MAX_SPEED_COM_FREQ ((MAX_RPM*COM_STEPS_PER_TURN) / 60)
#define MAX_SPEED_DELAY (1000000L / MAX_SPEED_COM_FREQ) // in microseconds
#define MAX_SPEED_OCR_VAL ((MAX_SPEED_DELAY / TIMER_US) - 1)

//#define TICK_DIVIDER_MAX 200

// sensor data
#define RAD_TO_DEG (57.29578f)
#define PI (3.14159f)
#define MAX_ANGLE	((45.0f * PI) / 180)
#define INIT_VARIANCE_ANGLE ((MAX_ANGLE * MAX_ANGLE) / 3.0f)
// offsets
#define Z_ANGLE_OFFSET 103

#endif /* CONFIG_H_ */
