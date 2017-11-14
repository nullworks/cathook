 
/*
 *
 *	Cathooks mathlib!	
 *
 *
 */

#include "../framework/entitys.hpp" // For fov, local players angles

#include "mathlib.hpp"

// Create points for the box based on max and mins
void CatBox::GetPoints(CatVector* points) {	// Be sure to pass an array with at least 8 values
	
	static float x, y, z;
	x = max.x - min.x;
	y = max.y - min.y;
	z = max.z - min.z;
	points[0] = min;
	points[1] = min + CatVector(x, 0, 0);
	points[2] = min + CatVector(x, y, 0);
	points[3] = min + CatVector(0, y, 0);
	points[4] = min + CatVector(0, 0, z);
	points[5] = min + CatVector(x, 0, z);
	points[6] = min + CatVector(x, y, z);
	points[7] = min + CatVector(0, y, z);
}

// Math utilities

namespace util {

// Clamps angles to prevent them from going out of bounds
void ClampAngles(CatVector& angles) {
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
}
// Does clamping, but returns the result instead of changing the input
CatVector ClampAngles_r(const CatVector& angles) {
	static CatVector tmp;
	tmp = angles;
	ClampAngles(tmp);
	return tmp;
}
	
// Returns angles to a point in space
CatVector VectorAngles(const CatVector& src_point, const CatVector& dest_point) {
	static CatVector aim_point;
	aim_point = dest_point - src_point;
	
	static float tmp, yaw, pitch;
	yaw = atan2(aim_point.y, aim_point.x) * 180 / PI;
	tmp = sqrt(aim_point.x * aim_point.x + aim_point.y * aim_point.y);
	pitch = atan2(-aim_point.z, tmp) * 180 / PI;
	
	return ClampAngles_r(CatVector(pitch, yaw));
}

// A function to get the difference from angles, Please make sure inputs are clamped
CatVector GetAngleDifference(const CatVector& cur_angles, const CatVector& dest_angles) {
	
	// Our output difference
	CatVector diff;
	
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
	if (cur_angles.x != dest_angles.x) {
		diff.x = std::abs(cur_angles.x - dest_angles.x);
	}
	return diff;
}
// Use input angles and our eye position to get fov to a destination point
float GetFov(const CatVector& orig_angle, const CatVector& eye_position, const CatVector& dest_point) {
	static CatVector aim_angles, delta;
	// Get the aimbots angles
	aim_angles = VectorAngles(eye_position, dest_point);
	// Get the delta from the current and aimbots angles
	delta = GetAngleDifference(orig_angle, aim_angles);
	// Return our fov
	return std::max(delta.x, delta.y);
}
// Used to get fov without the need to input camera angles or eye position by using the local player.
float GetFov (const CatVector& dest_point) {
	return GetFov(g_LocalPlayer.camera_angles, g_LocalPlayer.camera_position, dest_point);
}

}

