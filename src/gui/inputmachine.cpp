 
/*
 *
 *	This is the input machine that handles the user input of gui and its elements
 *
 */

#include "base/baseelement.hpp"

#include "inputmachine.hpp"

namespace gui {
	
bool InputFromTree(CBaseWidget* base_widget) {
	if (base_widget == nullptr) return false;
	if (!base_widget->visible) return false; // If it isnt visible, we dont check it or anything it owns
	
	// If there are children, we recurse
	if (!base_widget->child_widgets.empty()) {
		// Recurse throught the children check if they are accepting ui
		for(CBaseWidget* widget : base_widget->child_widgets) {
			if (widget == nullptr) continue;
			// check if something is using user input down the tree
			if (InputFromTree(widget)) return true;
		}
	}
	
	// If it wants user input, check if widget is accepting user input
	if (base_widget->usrinput != nullptr && base_widget->usrinput(base_widget)) return true;
	
	return false;
}
	
// This should be called on draw. The init file requests this for us!
void InputMachine() {
	if (CBaseWidgetRoots.empty()) return; // Cant check nothing
	
	// Recurse backwards through to find roots from top down
	int list_size = CBaseWidgetRoots.size();
	for (int i = list_size; i > 0; i--) {
		CBaseWidget* base_widget = CBaseWidgetRoots[i - 1];
		if (base_widget == nullptr) continue;	// To prevent crash
		
		// If something returns true, then ui is being used by an element
		if (InputFromTree(base_widget)) return;
	}
}	
}