 
/*
 *
 *	Button 
 *
 */
#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "button.hpp"


namespace gui { namespace element {
	
void ButtonDraw(const CBaseWidget* base_widget, rgba_t color) {
	if (base_widget == nullptr || !base_widget->visible) return;	// Draw name
	if (base_widget->child_bool != nullptr && base_widget->child_bool) {
		drawmgr::RectFilled(base_widget->rootx - 1, base_widget->rooty - 1, base_widget->widthx - 1, base_widget->widthy - 1, rgba_t(25, 25, 25, 192));// Depressed look
		drawmgr::Rect(base_widget->rootx - 1, base_widget->rooty - 1, base_widget->widthx - 1, base_widget->widthy - 1, color);
	} else
		drawmgr::Rect(base_widget->rootx, base_widget->rooty, base_widget->widthx, base_widget->widthy, color);
}

bool ButtonHandleUi(CBaseWidget* base_widget) {
	return false;	// TODO, make it functional
}
	
CBaseWidget* ButtonCreate(CBaseWidget* root_parent, int layer){		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, layer, ButtonDraw);
	tmp->usrinput = ButtonHandleUi;
	return tmp;
}

}}