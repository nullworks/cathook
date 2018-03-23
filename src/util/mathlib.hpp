
/*
 *
 *	An area to put all that good delicious math that can work crossgame, or in some of the framework!
 *		Dont put anything game specific here please!
 *			-Onee
 *
 */

#pragma once

#include <math.h> // This is a mathlib, did you think we didnt need math?
#include <array> // std::array<>
// Macro for getting the size of an array, Uses the size of first element then compares it to the size of the entire array. This is optimised out.
#define GET_ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

// Pi stuff
#define PI 3.14159265
#define RADPI 57.295779513082f

// Struct for point in 3d space
class CatVector {
public:
	inline CatVector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : x(_x), y(_y), z(_z) {}
	float x, y, z;
	// So we can do math with our catvectors easily
	inline CatVector operator+(CatVector value) const { return CatVector(x + value.x, y + value.y, z + value.z); }
	inline CatVector operator-(CatVector value) const { return CatVector(x - value.x, y - value.y, z - value.z); }
	inline CatVector operator*(CatVector value) const { return CatVector(x * value.x, y * value.y, z * value.z); }
	inline CatVector operator/(CatVector value) const { return CatVector(x / value.x, y / value.y, z / value.z); }
	inline CatVector operator*(float value) 	  const { return CatVector(x * value, y * value, z * value); }
	inline CatVector operator/(float value)	    const { return CatVector(x / value, y / value, z / value); }
	inline bool operator==(CatVector value) const { return value.x == x && value.y == y && value.z == z; }
	inline bool operator!=(CatVector value) const { return value.x != x || value.y != y || value.z != z; }
	// Used to get the distance between 2 vectors
	inline float DistTo(CatVector end) const { return sqrt(pow(x - end.x, 2) + pow(y - end.y, 2) + pow(z - end.z, 2)); }
};

// Used to store points in a box
class CatBox {
public:
	inline CatBox(CatVector min = CatVector(), CatVector max = CatVector()) : min(min), max(max) {};
	CatVector min, max;

	inline auto GetPoints() const { // Used to get out all 8 points from our box, Be sure to pass an array with at least 8 values
		// Get deltas
		auto delta = this->GetDelta();
		// Expand deltas into 8 points for box
		std::array<CatVector, 8> points;
		points[0] = min;
		points[1] = min + CatVector(delta.x, 0,       0);
		points[2] = min + CatVector(delta.x, delta.y, 0);
		points[3] = min + CatVector(0,       delta.y, 0);
		points[4] = min + CatVector(0,       0,       delta.z);
		points[5] = min + CatVector(delta.x, 0,       delta.z);
		points[6] = min + CatVector(0,       delta.y, delta.z);
		points[7] = max;
		return points;
	}
	// Credits to cathook
	inline bool LineIntersects(CatVector src, CatVector dst) const {
    if (dst.x < min.x && src.x < min.x)
        return false;
    if (dst.y < min.y && src.y < min.y)
        return false;
    if (dst.z < min.z && src.z < min.z)
        return false;
    if (dst.x > max.x && src.x > max.x)
        return false;
    if (dst.y > max.y && src.y > max.y)
        return false;
		if (dst.z > max.z && src.z > max.z)
			return false;
		return true;
	}
	inline CatVector GetCenter() const { return (min + max) * 0.5; }
	inline CatVector GetDelta() const { return max - min; }
	inline bool operator==(CatBox value) const { return value.min == min && value.max == max; }
	inline bool operator!=(CatBox value) const { return value.min != min || value.max != max; }
	inline CatBox operator+(CatVector value) const { return CatBox(min + value, max + value); }
	inline CatBox operator-(CatVector value) const { return CatBox(min - value, max - value); }
	inline CatBox operator*(float value) const { // for expanding relative to center
		// Take the center so we can operate on the delta later
		CatVector center = this->GetCenter();
		// Take the delta and do the operation on our box, devide by 2 after so we can add to center
		CatVector delta = (this->GetDelta() * value) * 0.5;
		// Recreate the box with the new delta and return
		return CatBox(center - delta, center + delta);
	}
	inline CatBox operator/(float value) const { // for shrinking relative to center
		// Take the center so we can operate on the delta later
		CatVector center = this->GetCenter();
		// Take the delta and do the operation on our box, devide by 2 after so we can add to center
		CatVector delta = (this->GetDelta() / value) * 0.5;
		// Recreate the box with the new delta and return
    return CatBox(center - delta, center + delta);
	}
};

// Struct for point in 4d space
class CatVector4 {
public:
	inline CatVector4(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _a = 0.0f) : x(_x), y(_y), z(_z), a(_a) {}
	float x, y, z, a;

	// So we can do math with our catvectors easily
  inline CatVector4 operator*(float value)	const { return CatVector4(x * value, y * value, z * value, a * value); }
  inline CatVector4 operator/(float value)	const { return CatVector4(x / value, y / value, z / value, a / value); }
	inline bool operator==(CatVector4 value) const { return value.x == x && value.y == y && value.z == z && value.a == a; }
	inline bool operator!=(CatVector4 value) const { return value.x != x || value.y != y || value.z != z || value.a != a; }
};

namespace util {

// Clamps angles to prevent them from going out of bounds, this is simple and should be inlined
inline CatVector& ClampAngles(CatVector& angles) {
	// Pitch
	while(angles.x > 89)
		angles.x -= 180;
	while(angles.x < -89)
		angles.x += 180;
	// Yaw
	while(angles.y > 180)
		angles.y -= 360;
	while(angles.y < -180)
		angles.y += 360;
	// Roll
	angles.z = 0;
	return angles;
}

// Input 2 angles to get the delta of difference
CatVector GetAngleDifference(CatVector cur_angles, CatVector dest_angles);
// Input the angles of your player, the vector position of your camera, and the destination point and it returns fov value
float GetFov(CatVector orig_angle, CatVector eye_position, CatVector dest_point);
// Returns angles used to aim at an object from a point, to another
CatVector VectorAngles(CatVector src_point, CatVector dest_point);
// Give it a position, angles to go in, and how far to extend, then it returns a vector in that direction
CatVector ExtendLine(CatVector src_pos, CatVector src_angles, float extend_amt);
// Useful random float gen
float RandFloatRange(float min, float max);

}
