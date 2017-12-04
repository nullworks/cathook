
/*
 *
 *	Here is the features file, this connects the cheats main features to the framework.
 *
 *
 */

// Stuff to init
#include "esp.hpp"
#include "aimbot.hpp"

#include "features.hpp"

namespace features {

void Init() {

	// Esp
	esp::Init();

	// Aimbot
	aimbot::Init();
}


}
