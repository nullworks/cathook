 
/*
 *
 *	This is an element to contain button elements in a small space.
 *	This element autosizes the buttons given to it based on the height and width parameters given to it.
 *	Please use extra_ints[0] for font and extra_ints[1] for font size of the buttons.
 *	Change extra_ints[3] to a 1 and it will stretch the buttons if it doesnt need to scroll
 *	Please only give it buttons or related
 *
 */

#include <algorithm> // max()

#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "buttonbar.hpp"

namespace gui { namespace element {
	
void ButtonBarDraw(CBaseWidget* base_widget) {	// We control the positions of our child buttons. We may take over drawing at times.
	if (!base_widget->visible) return;
	if (base_widget->child_widgets.empty())	return;	// We cant do anything without any children
	
	// First we need to get how long our buttons are
	base_widget->extra_ints[2] = 0;
	for (CBaseWidget* widget : base_widget->child_widgets) {
		// Get string length + height
		int length, height;
		drawmgr::strings::GetStringLength(widget->name.c_str(), widget->extra_ints[0], widget->extra_ints[1], length, height);
		// Get side distance from height and and add to total distance
		int side_distance = std::max(base_widget->height - height, 3);
		base_widget->extra_ints[2] += length + side_distance;
	}
	
	// We go and size our stuff now
	int length_used = 0;
	if (base_widget->extra_ints[2] <= base_widget->width) length_used += (base_widget->width - base_widget->extra_ints[2]) / 2; // Add to our used length if we are centering it, this is what centers them
	for (CBaseWidget* widget : base_widget->child_widgets) {
		// Make visible
		widget->visible = true; // Max
		widget->color.a = 255; // Max
		
		// Get string length + height
		int length, height;
		drawmgr::strings::GetStringLength(widget->name.c_str(), widget->extra_ints[0], widget->extra_ints[1], length, height);
		// Get side distance from height
		int side_distance = std::max(base_widget->height - height, 3);
		
		// if we dont need to allow scrolling, center everything
		if (base_widget->extra_ints[2] <= base_widget->width) {
			
			// Set the widgets size
			widget->rootx = base_widget->rootx + length_used;
			widget->rooty = base_widget->rooty;
			widget->width = length + side_distance;
			widget->height = base_widget->height;
			
			// Add to the total length used
			length_used += length + side_distance;
		// We need to do some special logic for scrolling here
		} else {
			int out_scroll_dist = base_widget->extra_ints[2] - base_widget->width;	// This is how much room we can scroll out of bounds
			
			widget->rootx = base_widget->rootx + length_used + ((base_widget->position / 0.01) * out_scroll_dist); // Position can be from 0-100 so we ake it scale the percentage of our our bounds
			widget->rooty = base_widget->rooty;
			widget->width = length + side_distance;
			widget->height = base_widget->height;
			
			// Check if the box is out of bounds
			if (widget->rootx + widget->width < base_widget->rootx || widget->rootx > base_widget->rootx + base_widget->width) {
				widget->visible = false;
				continue;
			}
			// Clamp if needed, also set opacity lower
			if (widget->rootx < base_widget->rootx) {
				int width_offset = base_widget->rootx - widget->rootx;	// The amount of how much its out of bounds
				widget->color = Transparent(widget->color, (widget->width - width_offset) / widget->width);	// I would like to reduce the opacity by how much percent the offset is of the width
				widget->width -= width_offset;	// Shrink the width by the amount its over the bounds
				widget->rootx = base_widget->rootx;
			}
			if (widget->rootx + widget->width < base_widget->rootx + base_widget->width) {
				int width_offset = (widget->rootx + widget->width) - (base_widget->rootx + base_widget->width);	// The amount of how much its out of bounds
				widget->color = Transparent(widget->color, (widget->width - width_offset) / widget->width);	// I would like to reduce the opacity by how much percent the offset is of the width
				widget->width -= width_offset;	// Shrink the width by the amount its over the bounds
			}
		}
	}
}

bool ButtonBarHandleUi(CBaseWidget* base_widget) {
	if (!base_widget->visible) return false;
	if (base_widget->child_widgets.empty())	return false;	// We cant do anything without any children
	if (base_widget->extra_ints[2] <= base_widget->width) return false;	// If total length is less than our width, then we can fit everything in. Else, we detect mouse for positioning
	if (CatUserInp.mousex > base_widget->rootx && CatUserInp.mousey > base_widget->rooty && CatUserInp.mousex < base_widget->rootx + base_widget->width && CatUserInp.mousey < base_widget->rooty + base_widget->height) {// check for bounds
		base_widget->position = CatUserInp.mousex / (base_widget->rootx + base_widget->width); // Find the percnentage of our mouse to scale and move the position
	}
	return false;
}
	
CBaseWidget* ButtonBarCreate(CBaseWidget* root_parent) {		// Use when creating a slider. This returns a pointer to the widget
	CBaseWidget* tmp = new CBaseWidget(root_parent, ButtonBarDraw);
	tmp->usrinput = ButtonBarHandleUi;
	return tmp;
}
	
}}