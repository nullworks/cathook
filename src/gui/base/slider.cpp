 
/*
 *
 *	Devider to seperate the menu
 *
 */

#include <math.h>	 // floor()
#include <algorithm> // max() min()

#include "../../util/stringhelpers.hpp"
#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../managers/drawmodulemgr.hpp" // So we can draw
#include "../sidestrings/sidestrings.hpp"

#include "slider.hpp"

// TODO, make slider increase "scope" when mouse is moved up.

namespace gui { namespace element {
	
void SliderDraw(const CBaseWidget* base_widget) {	
	if (base_widget == nullptr || !base_widget->visible) return;
	
	// slider bar rail
	drawmgr::RectFilled(base_widget->GetRealRoot().x,	
				 		base_widget->GetRealRoot().y + (base_widget->height / 6),
				 		base_widget->width, 
				 	    base_widget->height / 6, rgba_t(205, 205, 205, 35));
	
	// Slider fill
	drawmgr::RectFilled(base_widget->GetRealRoot().x,	
				 		base_widget->GetRealRoot().y + (base_widget->height / 6),
				 		base_widget->position, 
				 	    base_widget->height / 6, rgba_t(205, 205, 205, 225));
	// Button infill
	drawmgr::RectFilled(base_widget->GetRealRoot().x + base_widget->position + 1,	
			      base_widget->GetRealRoot().y + (base_widget->height / 6) + 1,
			     (base_widget->height / 9) - 1,
			     (base_widget->height / 6) - 1, rgba_t(255, 255, 255, 255));
	
	// Button outline
	drawmgr::Rect(base_widget->GetRealRoot().x + base_widget->position,	
			      base_widget->GetRealRoot().y + (base_widget->height / 6),
			      base_widget->height / 9,
			      base_widget->height / 6, rgba_t(255, 0, 255, 255));
	
	// Get value our value so we can display it
	float value = 0;
	if (base_widget->child_int != nullptr) {
		value = *base_widget->child_int;
	} else if (base_widget->child_float != nullptr) {
		value = *base_widget->child_float;
	}
	
	// Get length so we can center our string
	int length, height;
	drawmgr::strings::GetStringLength(format(base_widget->name, ": ", value).c_str(), base_widget->extra_ints[0], base_widget->extra_ints[1], length, height);

	// Print centered string
	drawmgr::strings::String(format(base_widget->name, ": ", value).c_str(),
							 base_widget->GetRealRoot().x + ((base_widget->width - length) / 2), 
							 base_widget->GetRealRoot().y + (base_widget->height / 2), 
							 base_widget->extra_ints[0], base_widget->extra_ints[1], rgba_t(255, 255, 255, 250));
}

bool SliderHandleUi(CBaseWidget* base_widget) {
	// Only do stuff if we have a child value
	if (base_widget->child_int != nullptr || base_widget->child_float != nullptr) {	// Make sure we have a value to control and Check for m1
		
		// Update Position
		// Min and max are to prevent the slider from going off its "rails". First this gets the percentage of how much it is of the max and min combined then it scales that to the width.
		if (base_widget->child_int != nullptr)
			base_widget->position = std::max(std::min((*base_widget->child_int / (base_widget->min_position + base_widget->max_position)) * (base_widget->width - (base_widget->height / 9)), (float)base_widget->width), 0.0f);
		else if (base_widget->child_float != nullptr)
			base_widget->position = std::max(std::min((*base_widget->child_float / (base_widget->min_position + base_widget->max_position)) * (base_widget->width - (base_widget->height / 9)), (float)base_widget->width), 0.0f);
		
		// Check for m1
		if (CatUserInp.IsKeyPressed(CATKEY_MOUSE_1)) {
			if (!base_widget->performed_last) {
														   
				if (!(CatUserInp.mousex > base_widget->GetRealRoot().x + base_widget->position && //	Bounds checking
					  CatUserInp.mousey > base_widget->GetRealRoot().y + (base_widget->height / 6) && 
					  CatUserInp.mousex < base_widget->GetRealRoot().x + base_widget->position  + (base_widget->height / 9) && 
					  CatUserInp.mousey < base_widget->GetRealRoot().y + (base_widget->height / 6) + (base_widget->height / 6))) return false;

				base_widget->performed_last = true;
				PushOnTop(base_widget);
				widget_mouseoffsetx = CatUserInp.mousex;
			} else {
				
				// Update position
				float change = CatUserInp.mousex - widget_mouseoffsetx; // We update the position based on the change of the mouse 
				if (base_widget->child_int != nullptr) change += 1.5f;	// Ints are weird, this fixes some stuff
				base_widget->position = std::max(std::min(base_widget->position + change, (float)(base_widget->width - (base_widget->height / 9))), 0.f); // Min + max is out of bounds clamping
				widget_mouseoffsetx = CatUserInp.mousex;
				
				// Update child values
				if (base_widget->child_int != nullptr) 
					*base_widget->child_int = (int)floor(((base_widget->position / (base_widget->width - (base_widget->height / 9))) * (base_widget->min_position + base_widget->max_position)));
				else if (base_widget->child_float != nullptr) 
					*base_widget->child_float = (float)((base_widget->position / (base_widget->width - (base_widget->height / 9))) * (base_widget->min_position + base_widget->max_position));
			}
			return true;
		}
		
	}
	base_widget->performed_last = false;
	return false;
}

CBaseWidget* SliderCreate(CBaseWidget* root_parent) {		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, SliderDraw);
	tmp->usrinput = SliderHandleUi;
	return tmp;
}
	
}}