
/*
 *
 *	A Root of a gui
 *
 */

#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "root.hpp"

// TODO, make root as an base element

namespace gui { namespace element {
	
CRoot::CRoot() { 					// When creating this, i want to push it to the list
	CRootList().push_back(this);
}
void CRoot::draw(rgba_t color) {
	if (!visible) return;
	drawmgr::RectFilled(rootx - 1, rooty - 1, widthx - 1, widthy - 1, rgba_t(25, 25, 25, 192));// Nice transparent gray inner box, 75% opacity
	drawmgr::Rect(rootx, rooty, widthx, widthy, color);
}
	
std::vector<CRoot*>& CRootList();
}}