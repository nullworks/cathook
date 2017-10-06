 
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

#include "colors.hpp"

namespace colors {

// Use this to get a color from an entity!
rgba_t(*StoredColorEntity)(CatEntity*) = nullptr;
rgba_t Entity(CatEntity* entity) {
	// If a module has registered a custom color scheme, we apply that here
	if (StoredColorEntity != nullptr) return StoredColorEntity(entity);
	
	// Without a custom scheme, we guess with the information given to the framework
	
	// All ents have white by defaults
	rgba_t ent_color = rgba_t(255, 255, 255, 255); 
	
	// Different strokes for different folks
	if (entity->type == ETYPE_PLAYER || entity->type == ETYPE_OTHERHOSTILE || entity->type == ETYPE_PLAYER || entity->type == ETYPE_PROJECTILE) {
		switch(entity->team) {
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
			ent_color = red; break;
		}
	
	// Pickups
	} else if (entity->type == ETYPE_PICKUP_HEALTH || entity->type == ETYPE_PICKUP_SHEILD) {
		switch(entity->type) {
		case ETYPE_PICKUP_HEALTH:
			ent_color = green; break;
		case ETYPE_PICKUP_SHEILD:
			ent_color = yellow; break;
		}
	}
	return ent_color;
}

// If you wish to use colors seperate from what the framework has in mind, you may register your func with this
void RegisterCustomColorToEntity(void *func(CatEntity*)) {	// The best name evar, I promise ;)
	StoredColorEntity = func;
}

// Returns a color based on entity health
rgba_t Health(CatEntity* entity) {
	if (entity->health > entity->max_health)	// Health is tuu much, they must be over their normal health so we make them blue
		return rgba_t(64, 128, 255, 255);
	
	// Percentage of health our of max
	float hf = (float)entity->health / (float)entity->max_health;
	// Sick logic
	return rgba_t((hf <= 0.5 ? 1.0 : 1.0 - 2 * (hf - 0.5)) * 255, (hf <= 0.5 ? (2 * hf) : 1) * 255, 0, 255);
}

// Currently broken without a way to get time.
// on inject make "time injected" chrono and save it. get duration since make and cast to count. use as time for rainbow.
// have it be in seconds in a float with lesser miliseconds be used as 
rgba_t RainbowCurrent() {
	static float tick = 0;
	tick += 0.003;
	return colors::FromHSL(fabs(sin(tick / 2.0f)) * 360.0f, 0.85f, 0.9f);	// Remove this once u get it working
	
	static std::chrono::steady_clock::time_point time_point = std::chrono::steady_clock::now();
	float curtime = std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count() * 0.001f;
	return colors::FromHSL(fabs(sin(curtime / 2.0f)) * 360.0f, 0.85f, 0.9f);
}

// Colored string constructors
ColoredString::ColoredString(std::string string_in, rgba_t color_in) : string(string_in), color(color_in) {}
ColoredString::ColoredString(std::string string_in) : string(string_in) {}
ColoredString::ColoredString() {}
	
}