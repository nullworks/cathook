 
/*
 *
 *	Button 
 *
 */
#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "button.hpp"


namespace gui { namespace element {
	
void ButtonDraw(const CBaseWidget* base_widget) {
	if (!base_widget || !base_widget->visible) return;	// Draw name
	if (base_widget->child_bool) {
		drawmgr::RectFilled(base_widget->rootx - 1, base_widget->rooty - 1, base_widget->width - 1, base_widget->height - 1, rgba_t(25, 25, 25, base_widget->color.a * 0.75));// Depressed look
		drawmgr::Rect(base_widget->rootx - 1, base_widget->rooty - 1, base_widget->width - 1, base_widget->height - 1, base_widget->color);
	} else
		drawmgr::Rect(base_widget->rootx, base_widget->rooty, base_widget->width, base_widget->height, base_widget->color);
}

bool ButtonHandleUi(CBaseWidget* base_widget) {
	return false;	// TODO, make it functional
}
	
CBaseWidget* ButtonCreate(CBaseWidget* root_parent){		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, ButtonDraw);
	tmp->usrinput = ButtonHandleUi;
	return tmp;
}

}}