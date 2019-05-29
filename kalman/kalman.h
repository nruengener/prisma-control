/*
 * kalman.h
 *
 *  Adapted from C++ code at https://github.com/TKJElectronics/KalmanFilter.
 *  Author: Nando Rüngener
 */

#ifndef KALMAN_KALMAN_H_
#define KALMAN_KALMAN_H_

typedef struct {
	float q_angle;

	// Process noise variance for the accelerometer
	float q_bias; // Process noise variance for the gyro bias
	float r_measure; // Measurement noise variance - this is actually the variance of the measurement noise

	float angle; // The angle calculated by the Kalman filter - part of the 2x1 state vector
	float bias; // The gyro bias calculated by the Kalman filter - part of the 2x1 state vector
	float rate; // Unbiased rate calculated from the rate and the calculated bias - you have to call getAngle to update the rate

	float P[2][2]; // Error covariance matrix - This is a 2x2 matrix
} Kalman;

//typedef struct {
//	float q_angle = 0.001f;
//
//	// Process noise variance for the accelerometer
//	float q_bias = 0.003f; // Process noise variance for the gyro bias
//	float r_measure = 0.03f; // Measurement noise variance - this is actually the variance of the measurement noise
//
//	float angle = 0.0f; // The angle calculated by the Kalman filter - part of the 2x1 state vector
//	float bias = 0.0f; // The gyro bias calculated by the Kalman filter - part of the 2x1 state vector
//	float rate = 0.0f; // Unbiased rate calculated from the rate and the calculated bias - you have to call getAngle to update the rate
//
//	float P[2][2] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Error covariance matrix - This is a 2x2 matrix
//} Kalman;

void updateAngle(Kalman* kalman, float measuredAngle, float measuredRate, float dt);

void initKalman(Kalman* kalman);

#endif /* KALMAN_KALMAN_H_ */
