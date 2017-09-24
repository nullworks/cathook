 
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
void DrawMachine() {
	if (CBaseWidgetList.empty()) return; // Cant draw nothing
	
	// Recurse backwards through to find roots from the bottom up
	int list_size = CBaseWidgetList.size();
	for(int i = list_size; i >= 0; i--) {
		CBaseWidget* i_widget = CBaseWidgetList[i];
		if (i_widget == nullptr) continue;
		if (!i_widget->root) continue;	// We only want roots
		
		// Draw the root so we can put stuff on top of it
		i_widget->draw(i_widget, GUIColor());
		
		// We found a root, draw stuff owned by it
		for(CBaseWidget* ii_widget : CBaseWidgetList) {
			if (ii_widget == nullptr) continue;
			if (ii_widget->root || ii_widget->root_parent != i_widget) continue;	// dont draw roots and stuff not part of our root-tree
				ii_widget->draw(ii_widget, GUIColor());
		}
	}
}	
}