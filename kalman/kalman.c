/*
 * kalman.c
 *
 *  Created on: 01.04.2017
 *      Author: Nando Rüngener
 */
#include <stdio.h>

#include "kalman.h"

void updateAngle(Kalman *kalman, float measuredAngle, float measuredRate,
		float dt) {

//	printf("angle before: %.2f\n", kalman->angle);

	kalman->rate = measuredRate - kalman->bias;
	kalman->angle += dt * kalman->rate;
//	printf("dt: %.2f \t kalman->rate: %.2f \n", dt,  kalman->rate);
	// Update estimation error covariance - Project the error covariance ahead
	/* Step 2 */
	kalman->P[0][0] += dt
			* (dt * kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0]
					+ kalman->q_angle);
	kalman->P[0][1] -= dt * kalman->P[1][1];
	kalman->P[1][0] -= dt * kalman->P[1][1];
	kalman->P[1][1] += kalman->q_bias * dt;

	// Discrete Kalman filter measurement update equations - Measurement Update ("Correct")
	// Calculate Kalman gain - Compute the Kalman gain
	/* Step 4 */
	float s = kalman->P[0][0] + kalman->r_measure; // Estimate error
	/* Step 5 */
	float K[2]; // Kalman gain - This is a 2x1 vector
	K[0] = kalman->P[0][0] / s;
	K[1] = kalman->P[1][0] / s;

	// Calculate angle and bias - Update estimate with measurement zk (newAngle)
	/* Step 3 */
	float y = measuredAngle - kalman->angle; // Angle difference
	/* Step 6 */
//	printf("K[0]: %.2f \t y: %.2f \n", K[0], y);
	kalman->angle += K[0] * y;
//	printf("updated angle: %.2f\n", kalman->angle);
	kalman->bias += K[1] * y;

	// Calculate estimation error covariance - Update the error covariance
	/* Step 7 */
	float P00_temp = kalman->P[0][0];
	float P01_temp = kalman->P[0][1];

	kalman->P[0][0] -= K[0] * P00_temp;
	kalman->P[0][1] -= K[0] * P01_temp;
	kalman->P[1][0] -= K[1] * P00_temp;
	kalman->P[1][1] -= K[1] * P01_temp;

//	printf("angle after: %.2f\n", kalman->angle);
}

void initKalman(Kalman* kalman) {
	kalman->angle = 0.0f;
	kalman->bias = 0.0f;
	kalman->rate = 0.0f;

	kalman->q_angle = 0.001f;
	kalman->q_bias = 0.003f;
	kalman->r_measure = 0.03f;

	kalman->P[0][0] = 0.0f;
	kalman->P[0][1] = 0.0f;
	kalman->P[1][0] = 0.0f;
	kalman->P[1][1] = 0.0f;
}
