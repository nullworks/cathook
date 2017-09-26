 
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
	if (!base_widget->visible) return;	// Draw name
	
	drawmgr::strings::String(base_widget->name.c_str(), base_widget->root_offsetx + base_widget->rootx, base_widget->root_offsety + base_widget->rooty, base_widget->extra_ints[0], base_widget->extra_ints[1], rgba_t(255, 255, 255, base_widget->color.a));
		
	if (base_widget->performed_last) {
		drawmgr::RectFilled(base_widget->root_offsetx + base_widget->rootx - 2, 
							base_widget->root_offsety + base_widget->rooty - 2, 
							base_widget->width - 1, base_widget->height - 1, rgba_t(25, 25, 25, base_widget->color.a * 0.75));// Depressed look
	}
	drawmgr::Rect(base_widget->root_offsetx + base_widget->rootx, 
		base_widget->root_offsety + base_widget->rooty, 
		base_widget->width, base_widget->height, base_widget->color);
}

bool ButtonHandleUi(CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return false;
	if (CatUserInp.IsKeyPressed(CATKEY_MOUSE_1)) {	// Check for m1
		if (!base_widget->performed_last) {
			if (!(CatUserInp.mousex > base_widget->root_offsetx + base_widget->rootx && //	Bounds checking
				  CatUserInp.mousey > base_widget->root_offsety + base_widget->rooty && 
				  CatUserInp.mousex < base_widget->root_offsetx + base_widget->rootx + base_widget->width && 
				  CatUserInp.mousey < base_widget->root_offsety + base_widget->rooty + base_widget->height)) return false;
			base_widget->performed_last = true;
			PushOnTop(base_widget);
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