
/*
 *
 *	Here is the features file, this connects the cheats main features to the framework.
 *
 *
 */

// Stuff to init
#include "esp.hpp"
#include "aimbot.hpp"
#include "antiaim.hpp"
#include "followbot.hpp"
#include "spam.hpp"
#include "bhop.hpp"

#include "features.hpp"

namespace features {

void Init() {

	// Esp
	esp::Init();

	// Antiaim, Must be before anything that changes angles!!!
	antiaim::Init();

	// Aimbot
	aimbot::Init();

	// Followbot
	followbot::Init();

	// Chat Spam
	spam::Init();

	// Bunnyhop
	bhop::Init();
}


}
