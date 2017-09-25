 
/*
 *
 *	Devider to seperate the menu
 *
 */

#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "slider.hpp"


namespace gui { namespace element {
	
void SliderDraw(const CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return; // Use colors "205 205 205 255"
	drawmgr::Line(base_widget->rootx, base_widget->rooty, base_widget->width, base_widget->height, base_widget->color);
}

bool SliderHandleUi(CBaseWidget* base_widget) {
	return false;	// TODO, make it functional
}

CBaseWidget* SliderCreate(CBaseWidget* root_parent){		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, SliderDraw);
	tmp->usrinput = SliderHandleUi;
	return tmp;
}
	
}}