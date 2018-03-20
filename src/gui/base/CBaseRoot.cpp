
/*
 *
 *  The guis main window, this handles input, drawing windows, tooltips and more.
 *  This is one of the most important parts of the gui.
 *
 */

#include "CBaseInfoBox.hpp"

#include "CBaseRoot.hpp"
#include "../../util/logging.hpp"

namespace gui { namespace base {

// An init for our catgui
CBaseRoot::CBaseRoot(std::string name, std::string tooltip) : CBaseParent(name, tooltip) {
	/*tooltip_widget = new CBaseInfoBox("tooltip", "some info", std::make_pair(200, 180));
	AddChild(tooltip_widget);*/
  	size = input::GetBounds();
}

void CBaseRoot::OnKeyPress(int key, bool repeat){
	CBaseParent::OnKeyPress(key, repeat);
}

void CBaseRoot::OnKeyRelease(int key){
	CBaseParent::OnKeyRelease(key);
}

}}
