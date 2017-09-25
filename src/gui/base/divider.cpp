 
/*
 *
 *	Devider to seperate the menu
 *
 */

#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "divider.hpp"


namespace gui { namespace element {
	
void DividerDraw(const CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return;
	drawmgr::Line(base_widget->rootx, base_widget->rooty, base_widget->width, base_widget->height, base_widget->color);
}

CBaseWidget* DividerCreate(CBaseWidget* root_parent) {	// Use when creating a devider. This returns a pointer to the widget
	return new CBaseWidget(root_parent, DividerDraw);
}
	
}}