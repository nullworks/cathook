
/*
 *
 *	This is the colors file, its used for everything related to color!
 *	Right now, go find the nearest walmart and buy some spray paint.
 * 	Spray the paint into your mouth.
 *	You are now officially gay or suicidal, your choice!
 *		-Onee
 *
 *
 */

#include <math.h>	// fabs() sin()
#include <chrono>	// For time keeping
#include <ratio> // above

#include "../features/aimbot.hpp"
#include "../features/followbot.hpp"

#include "colors.hpp"

namespace colors {

// Use this to get a color from an entity!
CMFunction<CatVector4(CatEntity*)> EntityColor {
	[](CatEntity* entity) {

	// Highlight targets
	if (entity == features::aimbot::highlight_target)
		return pink;
	if (entity == features::followbot::follow_target)
		return green;

	// Default color
	CatVector4 ent_color = white;

	// Different strokes for different folks
	auto ent_type = GetType(entity);
	if (ent_type == ETYPE_PLAYER || ent_type == ETYPE_OTHERHOSTILE || ent_type == ETYPE_PROJECTILE) {
		switch(GetTeam(entity)) {
		case ETEAM_RED:
			ent_color = red; break;
		case ETEAM_BLUE:
			ent_color = blue; break;
		case ETEAM_GREEN:
			ent_color = green; break;
		case ETEAM_YELLOW:
			ent_color = yellow; break;
		case ETEAM_ALLY:	// Blue good
			ent_color = blue; break;
		case ETEAM_ENEMY:	// Red bad
			ent_color = red;
		}
	}
	// Pickups
	switch(ent_type) {
	case ETYPE_PICKUP_HEALTH:
		ent_color = green; break;
	case ETYPE_PICKUP_SHEILD:
		ent_color = yellow;
	}
	return ent_color;
}};

// Returns a rainbow color based on time
CatVector4 RainbowCurrent() {
	static auto start_time = std::chrono::steady_clock::now();
	std::chrono::duration<float, std::deca> curtime = std::chrono::steady_clock::now() - start_time;
	return colors::FromHSL(fabs(sin(curtime.count())) * 360.0f, 0.85f, 0.9f);
}

// Returns a random color 
CatVector4 RandomColor() {
	return colors::FromHSL(rand() % 360, float(rand() % 10)/50+0.5f, float(rand() % 10)/50+0.5f);
}

}
