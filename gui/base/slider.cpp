 
/*
 *
 *	Devider to seperate the menu
 *
 */

#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "slider.hpp"


namespace gui { namespace element {
	
void SliderDraw(const CBaseWidget* base_widget, rgba_t color) {
	if (base_widget == nullptr || !base_widget->visible) return; // Use colors "205 205 205 255"
	drawmgr::Line(base_widget->rootx, base_widget->rooty, base_widget->widthx, base_widget->widthy, color);
}

bool SliderHandleUi(CBaseWidget* base_widget) {
	return false;	// TODO, make it functional
}

CBaseWidget* SliderCreate(CBaseWidget* root_parent, int layer){		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, layer, SliderDraw);
	tmp->usrinput = SliderHandleUi;
	return tmp;
}
	
}}