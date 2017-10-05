 
/*
 *
 *	Devider to seperate the menu
 *
 */

#include "../../framework/drawing.hpp" // So we can draw

#include "divider.hpp"


namespace gui { namespace element {
	
void DividerDraw(CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return;
	draw::Line(base_widget->GetRealRoot().x, base_widget->GetRealRoot().y, base_widget->width, base_widget->height, base_widget->GetColor());
}

CBaseWidget* DividerCreate(CBaseWidget* root_parent) {	// Use when creating a devider. This returns a pointer to the widget
	return new CBaseWidget(root_parent, DividerDraw);
}
	
}}