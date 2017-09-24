 
/*
 *
 *	This is the draw machine that handles drawing the gui and its elements.
 *
 */

#include "base/baseelement.hpp"
#include "gui.hpp" // GUIColor()

#include "drawmachine.hpp"

namespace gui {
	
// This should be called on draw. The init file requests this for us!
void Draw() {
	
	// Recurse backwards through to find roots from the bottom up
	for(int i = element::CBaseWidgetList().size(); i >= 0; i--) {
		element::CBaseWidget* i_widget = element::CBaseWidgetList()[i];
		if (!i_widget->root) continue;	// We only want roots
		
		// We found a root, draw stuff owned by it
		for(element::CBaseWidget* ii_widget : element::CBaseWidgetList()) {
			if ((ii_widget->root && ii_widget != i_widget) || ii_widget->root_parent != i_widget) continue;	// dont draw stuff we dont have to yet
				ii_widget->draw(ii_widget, GUIColor());
		}
	}
}	
}