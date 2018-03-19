
/*
 *
 *	Cathooks mathlib!
 *
 *
 */

#include <cstdlib> // rand()

#include "../framework/entitys.hpp" // For fov, local players angles

#include "mathlib.hpp"

// Math utilities

namespace util {

// Returns angles to a point in space
CatVector VectorAngles(CatVector src_point, CatVector dest_point) {
	CatVector aim_point = dest_point - src_point;
	// Get angles
	CatVector out;
	out.y = atan2(aim_point.y, aim_point.x) * 180 / PI;
	out.x = atan2(-aim_point.z, sqrt(aim_point.x * aim_point.x + aim_point.y * aim_point.y)) * 180 / PI;

	// Clamp and return
	return ClampAngles(out);
}

// A function to get the difference from angles, Please make sure inputs are clamped
CatVector GetAngleDifference(CatVector cur_angles, CatVector dest_angles) {

	// Our output difference
	CatVector diff = CatVector();

	// Yaw
	if (cur_angles.y != dest_angles.y) {
		// Check if yaw is on opposing poles, if so, we do an alternative process
		if (cur_angles.y < -90 && dest_angles.y > 90)
			diff.y = cur_angles.y + 360 - dest_angles.y;
		else if (cur_angles.y > 90 && dest_angles.y < -90)
			diff.y = dest_angles.y + 360 - cur_angles.y;
		else
			diff.y = std::abs(cur_angles.y - dest_angles.y);
	}
	// Pitch
	if (cur_angles.x != dest_angles.x)
		diff.x = std::abs(cur_angles.x - dest_angles.x);

	return diff;
}
// Use input angles and our eye position to get fov to a destination point
float GetFov(CatVector orig_angle, CatVector eye_position, CatVector dest_point) {

	// Get the aimbots angles
	CatVector aim_angles = VectorAngles(eye_position, dest_point);
	// Get the delta from the current and aimbots angles
	CatVector delta = GetAngleDifference(orig_angle, aim_angles);
	// Return our fov
	return std::max<float>(delta.x, delta.y);
}

// Give it a starting position, the direction to go in and how far!
CatVector ExtendLine(CatVector src_pos, CatVector src_angles, float extend_amt) {

	// Math I dont understand
	float sp = sinf(src_angles.x * PI / 180.f);
	float cp = cosf(src_angles.x * PI / 180.f);
	float sy = sinf(src_angles.y * PI / 180.f);
	float cy = cosf(src_angles.y * PI / 180.f);

	CatVector ret;
	ret.x = cp * cy;
	ret.y = cp * sy;
	ret.z = -sp;
	ret = ret * extend_amt + src_pos;
	return ret;
}

// Paste from cathhok
float RandFloatRange(float min, float max) {
    return (min + 1) + ((float)rand() / (float)RAND_MAX) * (max - (min + 1));
}

}
