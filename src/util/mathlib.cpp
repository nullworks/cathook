 
/*
 *
 *	Cathooks mathlib!	
 *
 *
 */

#include <math.h> // This is a mathlib, did you think we didnt need math?

#include "mathlib.hpp"

// Used to get the distance between 2 vectors
float CatVector::DistTo(CatVector end) {
	// 3d pythagorean theorem formula, wew
	return sqrt((pow(x - end.x, 2) + pow(y - end.y, 2) + pow(z - end.z, 2)));
}

// Constructor
CatBox::CatBox(){}
CatBox::CatBox(CatVector min, CatVector max) : min(min), max(max) {}

// Returns the center of a catbox
CatVector CatBox::center() {return (min + max) * 0.5;}

// Create points for the box based on max and mins
void CatBox::GetPoints(CatVector* points) {	// Be sure to pass an array with at least 8 values
	
	float x = max.x - min.x;
	float y = max.y - min.y;
	float z = max.z - min.z;
	points[0] = min;
	points[1] = min + CatVector(x, 0, 0);
	points[2] = min + CatVector(x, y, 0);
	points[3] = min + CatVector(0, y, 0);
	points[4] = min + CatVector(0, 0, z);
	points[5] = min + CatVector(x, 0, z);
	points[6] = min + CatVector(x, y, z);
	points[7] = min + CatVector(0, y, z);
}