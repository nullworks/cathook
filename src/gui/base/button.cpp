 
/*
 *
 *	A simple button that runs a void function pointed to it when depressed.
 *	Please use extra_ints[0] for font and extra_ints[1] for font size of the buttons.
 *
 */

#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "button.hpp"

namespace gui { namespace element {
	
void ButtonDraw(const CBaseWidget* base_widget) {
	if (!base_widget->visible) return;	// Draw name
	
	// Get string length + height so we can center it
	int length, height;
	drawmgr::strings::GetStringLength(base_widget->name.c_str(), base_widget->extra_ints[0], base_widget->extra_ints[1], length, height);

	drawmgr::strings::String(base_widget->name.c_str(),
							 base_widget->GetRealRoot().x + ((base_widget->width - length) / 2), 
							 base_widget->GetRealRoot().y + ((base_widget->height - height) / 2), 
							 base_widget->extra_ints[0], base_widget->extra_ints[1], rgba_t(255, 255, 255, base_widget->color.a));
	
	if (base_widget->performed_last) {
		drawmgr::RectFilled(base_widget->GetRealRoot().x + 1, 
							base_widget->GetRealRoot().y + 1,
							base_widget->width - 1, base_widget->height - 1, rgba_t(25, 25, 25, base_widget->color.a * 0.75));// Depressed look
	}
	drawmgr::Rect(base_widget->GetRealRoot().x, 
				  base_widget->GetRealRoot().y, 
				  base_widget->width, 
				  base_widget->height, base_widget->GetColor());
}

bool ButtonHandleUi(CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return false;
	if (CatUserInp.IsKeyPressed(CATKEY_MOUSE_1)) {	// Check for m1
		if (!base_widget->performed_last) {
			if (!(CatUserInp.mousex > base_widget->GetRealRoot().x && //	Bounds checking
				  CatUserInp.mousey > base_widget->GetRealRoot().y && 
				  CatUserInp.mousex < base_widget->GetRealRoot().x + base_widget->width && 
				  CatUserInp.mousey < base_widget->GetRealRoot().y + base_widget->height)) return false;
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