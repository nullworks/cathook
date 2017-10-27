 
/*
 *
 *	An area to put all that good delicious math that can work crossgame, or in some of the framework!
 *		Dont put anything game specific here please!
 *			-Onee
 *
 */

#pragma once

// Macro for getting the size of an array, Uses the size of first element then compares it to the size of the entire array.
#define GET_ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

// Pi stuff
#define PI 3.14159265
#define RADPI 57.295779513082f

// Needed for fov calculations
#ifndef RAD2DEG
	#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / (float)PI) )
#endif

// 2d point
class CatVector2 { 
public:
	CatVector2(const float& _x = 0.0f, const float& _y = 0.0f) { x = _x; y = _y; z = _z; }
	float x, y; 
};

// A 2d box
class CatBox2 {
public:
	CatBox2(CatVector2 min = CatVector2(), CatVector2 max = CatVector2());
	CatVector2 min, max;
};

// Struct for point in 3d space
class CatVector { 
public:
	CatVector(const float& _x = 0.0f, const float& _y = 0.0f, const float& _z = 0.0f) { x = _x; y = _y; z = _z; }
	float x, y, z; 
	// So we can do math with our catvectors easily
	inline CatVector operator+(const CatVector& value) const { return CatVector(x + value.x, y + value.y, z + value.z); }
	inline CatVector operator-(const CatVector& value) const { return CatVector(x - value.x, y - value.y, z - value.z); }
	inline CatVector operator*(const CatVector& value) const { return CatVector(x * value.x, y * value.y, z * value.z); }
	inline CatVector operator/(const CatVector& value) const { return CatVector(x / value.x, y / value.y, z / value.z); }
	inline CatVector operator*(const float& value) 	   const { return CatVector(x * value, y * value, z * value); }
	inline CatVector operator/(const float& value)	   const { return CatVector(x / value, y / value, z / value); }
	inline bool 	operator==(const CatVector& value) const { return value.x == x && value.y == y && value.z == z; }
	inline bool 	operator!=(const CatVector& value) const { return value.x != x || value.y != y || value.z != z; }
	// Used to get the distance between 2 vectors
	float DistTo(const CatVector& end) { return sqrt((pow(x - end.x, 2) + pow(y - end.y, 2) + pow(z - end.z, 2))); }
};

// Used to store points in a box
class CatBox {
public:
	CatBox(const CatVector& min = CatVector(), const CatVector& max = CatVector()) : min(min), max(max) {};
	void GetPoints(CatVector* points); // Used to get out all 8 points from our box, Be sure to pass an array with at least 8 values
	CatVector min, max;
	inline CatVector center() { return (min + max) * 0.5; }
	inline bool operator==(const CatBox& value) const { return value.min == min && value.max == max; }
	inline bool operator!=(const CatBox& value) const { return value.min != min || value.max != max; }
};

// Struct for point in 4d space
class CatVector4 { 
public:
	CatVector4(const float& _x = 0.0f, const float& _y = 0.0f, const float& _z = 0.0f, const float& _a) { x = _x; y = _y; z = _z; a = _a; }
	float x, y, z, a;
	// So we can do math with our catvectors easily
	inline bool operator==(const CatVector4& value) const { return value.x == x && value.y == y && value.z == z && value.a == a; }
	inline bool operator!=(const CatVector4& value) const { return value.x != x || value.y != y || value.z != z || value.a != a; }
};

namespace util {

// Input the angles of your player, the vector position of your camera, and the destination point and it returns fov value
float GetFov(const CatVector&, const CatVector&, const CatVector&);
float GetFov(const CatVector&);
// Returns angles used to aim at an object from a point, to another
CatVector VectorAngles(const CatVector& src_point, const CatVector& dest_point);
	
}