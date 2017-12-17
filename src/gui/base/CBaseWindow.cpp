
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

// General functions
void CBaseWindow::Draw() {
	auto abs = AbsolutePosition();
	draw::RectFilled(abs.first, abs.second, size.first, size.second, colors::Transparent(colors::black, 0.9));
	draw::Rect(abs.first, abs.second, size.first, size.second, colors::pink);
	CBaseContainer::Draw();
}

// User Input
void CBaseWindow::OnFocusGain() {
	zindex = zindex + 1;
	CBaseContainer::OnFocusGain();
}
void CBaseWindow::OnFocusLose() {
	zindex = zindex - 1;
	CBaseContainer::OnFocusLose();
}

// Child related util
void CBaseWindow::MoveChildren() { // TODO, Reverse!!!

}

}}
