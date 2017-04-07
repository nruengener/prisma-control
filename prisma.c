/*
 * prisma.c
 *
 *  Created on: 07.04.2017
 *      Author: Nando Rüngener
 */

#include "prisma.h"
#include <stdlib.h>

PrismaOrientation orientation;

void initOrientation() {
	orientation.unknown = 1;
	orientation.broadSide = 0;
	orientation.leftSide = 0;
	orientation.rightSide = 0;
	orientation.up = 0;
}

void detectOrientation(float xAngle, float yAngle, float zAngle) {
	// detect lying on broad side
	if (abs(yAngle) < 20) {
		orientation.unknown = 0;
		orientation.broadSide = 1;
//		orientation.leftSide = 0;
//		orientation.rightSide = 0;
//		orientation.up = 0;
	} else if (abs(yAngle) > 30) {
		orientation.broadSide = 0;
		orientation.unknown = 1;
	}

	if (zAngle < -25) {
		orientation.unknown = 0;
		orientation.rightSide = 1;
		orientation.leftSide = 0;
		orientation.up = 0;
	} else if (zAngle > 15) {
		orientation.unknown = 0;
		orientation.leftSide = 1;
		orientation.rightSide = 0;
		orientation.up = 0;
	} else {
		orientation.unknown = 0;
		orientation.rightSide = 0;
		orientation.leftSide = 0;
		orientation.up = 1;
	}

}

