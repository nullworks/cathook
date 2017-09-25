 
/*
 *
 *	This is the draw machine that handles drawing the gui and its elements.
 *
 */

#include "base/baseelement.hpp"
#include "gui.hpp" // GUIColor()

#include "drawmachine.hpp"

namespace gui {
	
void DrawFromTree(CBaseWidget* base_widget) {
	if (!base_widget) return;
	if (!base_widget->visible) return; // If it isnt visible, we dont want to draw it or anything owned by it.
	
	base_widget->draw(base_widget);
	
	// If there are children, we recurse
	if (base_widget->child_widgets.empty()) return;
	
	// Recurse throught the children and draw them
	for(CBaseWidget* widget : base_widget->child_widgets) {
		if (!widget) continue;
		
		DrawFromTree(widget);
	}
}

// This should be called on draw. The init file requests this for us!
void DrawMachine() {
	if (CBaseWidgetRoots.empty()) return; // Cant draw nothing
	
	// Find roots on the bottom
	for(CBaseWidget* widget : CBaseWidgetRoots) {
		if (!widget) continue;
		
		// Recurse into the root and draw it and its children
		DrawFromTree(widget);		
	}
}	

	
}