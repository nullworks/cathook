 
/*
 *
 *	This is an organizer that organizes items based on their height.
 *
 *
 */

#include "../../logging.h"

#include "organizer.hpp"

namespace gui { namespace element {
	
void OrganizerDraw(CBaseWidget* base_widget) {	// We control the positions of our child buttons. We may take over drawing at times.
	if (!base_widget->visible) return;
	if (base_widget->child_widgets.empty())	return;	// We cant do anything without any children
	
	// Loop through our children
	const int divider = 3;		// Thing to seperate our menu by, TODO!!! Make this a variable
	int counter = 0;			// For when we count to our new line
	int current_max_height = 0; // Used to store our mex height from our row so we can add to the used
	int length_used = 0;
	for (CBaseWidget* widget : base_widget->child_widgets) {
		widget->rootx = base_widget->rootx + (counter * (base_widget->width / divider));
		widget->rooty = base_widget->rooty + length_used;
		
		// If we got a higher ent, then we compensate here
		if (widget->height > current_max_height) current_max_height = widget->height;
		
		// If we reached the end ouf our line, we recurse and go down some
		if (counter++ >= divider) {
			length_used += current_max_height + 5;
			counter = 0;
			current_max_height = 0;
		}
	}
}
	
CBaseWidget* OrganizerCreate(CBaseWidget* root_parent) {
	logging::Info("Making Organizer!");
	return new CBaseWidget(root_parent, OrganizerDraw);
}
	
}} 
