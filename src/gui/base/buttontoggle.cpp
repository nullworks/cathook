
/*
 *
 *	This is a button toggle, use it to change the child bool from a click in the gui!
 *
 *
 */

#include "../../framework/inputmgr.hpp"		// So we get input
#include "../../framework/drawing.hpp" // So we can draw

#include "buttontoggle.hpp"

namespace gui { namespace element {
	
void ButtonToggleDraw(CBaseWidget* base_widget) {
	if (!base_widget->visible) return;	// Draw name
	
	// Get string length + height so we can offset it from the button
	int length;
	draw::GetStringLength(base_widget->name.c_str(), base_widget->font, base_widget->font_size, length, base_widget->height);
	
	// Draw centered string
	draw::String(base_widget->name.c_str(),
							 base_widget->GetRealRoot().x + base_widget->height + (base_widget->height / 3), // offset from button
							 base_widget->GetRealRoot().y, 
							 base_widget->font, base_widget->font_size, rgba_t(255, 255, 255, base_widget->color.a));
	
	// if child bool is true, we notify the user with a white square
	if (base_widget->child_bool != nullptr && *base_widget->child_bool) {
		draw::RectFilled(base_widget->GetRealRoot().x + 3, 
							base_widget->GetRealRoot().y + 3,
							base_widget->height - 6, 
							base_widget->height - 6, rgba_t(200, 200, 200, base_widget->color.a));// Depressed look
	}
	
	// Draw our ourside rect, we use the string sizing to get scaling for the button.
	draw::Rect(base_widget->GetRealRoot().x,
				  base_widget->GetRealRoot().y,
				  base_widget->height,
				  base_widget->height, base_widget->GetColor());
}

bool ButtonToggleHandleUi(CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return false;
	// Check for m1
	if (CatUserInp.IsKeyPressed(CATKEY_MOUSE_1)) {	
		if (!base_widget->performed_last) {
			// Bounds checking
			if (!(CatUserInp.mousex > base_widget->GetRealRoot().x && 
				  CatUserInp.mousey > base_widget->GetRealRoot().y && 
				  CatUserInp.mousex < base_widget->GetRealRoot().x + base_widget->height && 
				  CatUserInp.mousey < base_widget->GetRealRoot().y + base_widget->height)) return false;
			base_widget->performed_last = true;
			PushOnTop(base_widget);
			if (base_widget->child_bool != nullptr) *base_widget->child_bool = !*base_widget->child_bool; // Inverse our child bool
		}
		return true;
	}
	base_widget->performed_last = false;
	return false;
}
	
CBaseWidget* ButtonToggleCreate(CBaseWidget* root_parent) {		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, ButtonToggleDraw);
	tmp->usrinput = ButtonToggleHandleUi;
	return tmp;
}
	
}}

