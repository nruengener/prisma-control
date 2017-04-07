/*
 * prisma.h
 *
 *  Created on: 07.04.2017
 *      Author: Wohn
 */
#include <stdint.h>

#ifndef PRISMA_H_
#define PRISMA_H_

typedef struct {
	int8_t broadSide;
	int8_t leftSide;
	int8_t rightSide;
	int8_t up;
	int8_t unknown;
} PrismaOrientation;

//extern volatile Orientation orientation;

void initOrientation();
void detectOrientation(float xAngle, float yAngle, float zAngle);

#endif /* PRISMA_H_ */
