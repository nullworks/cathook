
/*
 *
 *	This element allows you to pick your color from a color wheel.
 *	Please pass it a rgba_t pointer to allow it to contol its color
 *
 */

#include <algorithm> // max() min()

#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "colorpicker.hpp"

namespace gui { namespace element {
	
void ColorPickerDraw(CBaseWidget* base_widget) {
	if (!base_widget->visible) return;	// Draw name
	
	// the offset we use to take it away from the edges of our bounds
	int offcenter = base_widget->width * 0.13; 
	int height_used = base_widget->height / 6; // To know how much height was already used. We start with a slight amount used.
	
	// Draw initial color presentor
	drawmgr::Rect(base_widget->GetRealRoot().x + offcenter, 
				  base_widget->GetRealRoot().y + height_used, 
				  base_widget->width - (offcenter * 2), 
				  base_widget->height / 5, base_widget->GetColor());
	// add to our height
	height_used += base_widget->height / 5;
	
	// Get string length + height so we can know how much to reserve for it
	int length, height;
	drawmgr::strings::GetStringLength(base_widget->name.c_str(), base_widget->font, base_widget->font_size, length, height);	
	
	// Push everything to a dialouge element
	/*if (
	// Draw our box to contain everything
	drawmgr::Rect(base_widget->GetRealRoot().x + offcenter,
				  base_widget->GetRealRoot().y + height_used, 
				  base_widget->width - offcenter * 2, 
				  base_widget->width - offcenter * 2 + length, base_widget->GetColor());
	
	int circlex = base_widget->GetRealRoot().x + (base_widget->width * 0.5);
	int circley = base_widget->GetRealRoot().y + ((base_widget->height - height) / 2) + height + (height / 2) + radius;
	int radius = (base_widget->width - (offcenter * 2)) * 0.47;
	drawmgr::Circle(circlex,	// We want the x to be centered
					circley,
					radius,	// Doesnt quite use all the space so it looks cool
					70,
				   	base_widget->GetColor());
	
	// Draw string over stuff.
	drawmgr::strings::String(base_widget->name.c_str(),
							 base_widget->GetRealRoot().x + ((base_widget->width - length) / 2), 
							 base_widget->GetRealRoot().y + ((base_widget->height - height) / 2), 
							 base_widget->font, base_widget->font_size, rgba_t(255, 255, 255, base_widget->color.a));*/
}

bool ColorPickerHandleUi(CBaseWidget* base_widget) {
	
	return false; // TODO, make
}
	
CBaseWidget* ColorPickerCreate(CBaseWidget* root_parent){		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, ColorPickerDraw);
	tmp->usrinput = ColorPickerHandleUi;
	return tmp;
}

}}