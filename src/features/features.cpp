 
/*
 *
 *	Here is the features file, this connects the cheats main features to the framework.
 *
 *
 */

// Stuff to init with
#include "../framework/drawmgr.hpp"
// Stuff to init
#include "esp.hpp"

#include "features.hpp"

namespace features {
	
void Init() {
	
	drawmgr::RequestDrawOnDraw(esp::Draw);
	
}
	
	
}