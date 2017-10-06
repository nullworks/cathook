 
/*
 *
 *	Here is the features file, this connects the cheats main features to the framework.
 *
 *
 */

// Stuff to init with
#include "../framework/drawmgr.hpp"
#include "../framework/worldtickmgr.hpp"
// Stuff to init
#include "esp.hpp"

#include "features.hpp"

namespace features {
	
void Init() {
	
	// Esp
	drawmgr::RequestDrawOnDraw(esp::Draw);
	worldtick::RequestBeforeWTick(esp::WorldTick);
	
}
	
	
}