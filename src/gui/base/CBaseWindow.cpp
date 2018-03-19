
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
CBaseWindow::CBaseWindow(const char* _name) : CBaseContainer(_name) {
	position_mode = FLOATING;
}

// General functions
void CBaseWindow::Draw() {
	auto abs = AbsolutePosition();
	draw::RectFilled(abs.first, abs.second, size.first, size.second, colors::Transparent(colors::black, 0.9));
	draw::Rect(abs.first, abs.second, size.first, size.second, colors::pink);
	CBaseContainer::Draw();
}

// User Input
// We set our index to -1 to put us in front of other windows
bool CBaseWindow::TryFocusGain() {
	zindex = -1;
	return CBaseContainer::TryFocusGain();
}

}}
