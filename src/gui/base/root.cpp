
/*
 *
 *	A Root of a gui
 *
 */

#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "root.hpp"

// TODO, make root as an base element

namespace gui { namespace element {
	

void RootDraw(CBaseWidget* base_widget, rgba_t& color) {
	if (base_widget == nullptr || !base_widget->visible) return;
	drawmgr::RectFilled(base_widget->rootx - 1, base_widget->rooty - 1, base_widget->widthx - 1, base_widget->widthy - 1, rgba_t(25, 25, 25, 192));// Nice transparent dark gray inner box, 75% opacity
	drawmgr::Rect(base_widget->rootx, base_widget->rooty, base_widget->widthx, base_widget->widthy, color);
}
	
bool RootHandleUi(CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return false;
	return false; // TODO, make happen
}
	
CBaseWidget* RootCreate() {
	CBaseWidget* tmp = new CBaseWidget();
	tmp->usrinput = RootHandleUi;
	tmp->draw = RootDraw;
	return tmp;
}
}}