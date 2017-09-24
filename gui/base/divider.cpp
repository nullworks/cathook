 
/*
 *
 *	Devider to seperate the menu
 *
 */

#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "divider.hpp"


namespace gui { namespace element {
	
void DividerDraw(const CBaseWidget* base_widget, rgba_t color) {
	if (base_widget == nullptr || !base_widget->visible) return;
	drawmgr::Line(base_widget->rootx, base_widget->rooty, base_widget->widthx, base_widget->widthy, color);
}

CBaseWidget* DividerCreate(CBaseWidget* root_parent, int layer) {	// Use when creating a devider. This returns a pointer to the widget
	return new CBaseWidget(root_parent, layer, DividerDraw);
}
	
}}