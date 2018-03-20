
/*
 *
 *	This is for most objects on the gui.
 *	Most things use this
 *
 */

#include "../../framework/drawing.hpp" // Draw stuff
#include "../../util/colors.hpp" // Draw stuff

#include "CBaseWindow.hpp"

namespace gui { namespace base {

// Constructor
CBaseWindow::CBaseWindow(std::string name) : CBaseContainer(name) {
}

// General functions
void CBaseWindow::Draw() {
	draw::RectFilled(global_pos.first, global_pos.second, size.first, size.second, colors::Transparent(colors::black, 0.9));
	draw::Rect(global_pos.first, global_pos.second, size.first, size.second, colors::pink);
	CBaseContainer::Draw();
}

// User Input
// We set our index to -1 to put us in front of other windows
/*bool CBaseWindow::TryFocusGain() {
	//zindex = -1;
	return CBaseContainer::TryFocusGain();
}*/

}}
