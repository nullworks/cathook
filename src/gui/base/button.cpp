 
/*
 *
 *	A simple button that runs a void function pointed to it when depressed.
 *	Please use extra_ints[0] for font and extra_ints[1] for font size of the buttons.
 *
 */

#include <string>

#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "button.hpp"

namespace gui { namespace element {
	
void ButtonDraw(const CBaseWidget* base_widget) {
	if (!base_widget || !base_widget->visible) return;	// Draw name
	if (base_widget->child_bool) {
		drawmgr::RectFilled(base_widget->rootx - 2, base_widget->rooty - 2, base_widget->width - 1, base_widget->height - 1, rgba_t(25, 25, 25, base_widget->color.a * 0.75));// Depressed look
		drawmgr::Rect(base_widget->rootx - 1, base_widget->rooty - 1, base_widget->width - 1, base_widget->height - 1, base_widget->color);
	} else
		drawmgr::Rect(base_widget->rootx, base_widget->rooty, base_widget->width, base_widget->height, base_widget->color);
}

bool ButtonHandleUi(CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return false;
	if (CatUserInp.IsKeyPressed(CATKEY_MOUSE_1)) {	// Check for m1
		if (!base_widget->performed_last) {
			if (!(CatUserInp.mousex > base_widget->rootx && CatUserInp.mousey > base_widget->rooty && CatUserInp.mousex < base_widget->rootx + base_widget->width && CatUserInp.mousey < base_widget->rooty + base_widget->height)) return false;
			base_widget->performed_last = true;
			PushOnTop(base_widget);
			if (base_widget->action) base_widget->action(); // Check if the button has a function to go with it, run it if you can.
		}
		return true;
	}
	base_widget->performed_last = false;
	return false;
}
	
CBaseWidget* ButtonCreate(CBaseWidget* root_parent){		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, ButtonDraw);
	tmp->usrinput = ButtonHandleUi;
	return tmp;
}

}}