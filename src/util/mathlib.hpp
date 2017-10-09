 
/*
 *
 *	An area to put all that good delicious math that can work crossgame, or in some of the framework!
 *		Dont put anything game specific here please!
 *			-Onee
 *
 */

#pragma once

// gets size of array
#define GET_ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

// Struct for point in 3d space
class CatVector { 
public:
	float x, y, z; 
	CatVector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { 
		x = _x; y = _y; z = _z; 
	}
	// So we can do math with our catvectors easily
	inline CatVector operator+(const CatVector& value) const { return CatVector(x + value.x, y + value.y, z + value.z); }
	inline CatVector operator-(const CatVector& value) const { return CatVector(x - value.x, y - value.y, z - value.z); }
	inline CatVector operator*(const CatVector& value) const { return CatVector(x * value.x, y * value.y, z * value.z); }
	inline CatVector operator/(const CatVector& value) const { return CatVector(x / value.x, y / value.y, z / value.z); }
	inline CatVector operator*(const float& value) const { return CatVector(x * value, y * value, z * value); }
	inline CatVector operator/(const float& value) const { return CatVector(x / value, y / value, z / value); }
	float DistTo(CatVector);
};

// Used to store points in a box
class CatBox {
public:
	CatBox();
	CatBox(CatVector min, CatVector max);
	void GetPoints(CatVector* points); // Used to get out all 8 points from our box, Be sure to pass an array with at least 8 values
	CatVector min = CatVector();
	CatVector max = CatVector();
	CatVector center();
};
