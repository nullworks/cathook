 
/*
 *
 *	This is the input machine that handles the user input of gui and its elements
 *
 */

#include "base/baseelement.hpp"
#include "../logging.h"

#include "inputmachine.hpp"

namespace gui {
	
// This should be called on draw. The init file requests this for us!
void InputMachine() {
	if (CBaseWidgetList.empty()) return; // Cant check nothing
	
	// Recurse backwards through to find roots from top down
	int list_size = CBaseWidgetList.size();
	logging::Info(format("Size: ", list_size).c_str());
	for(int i = list_size; i > 0; i--) {
		CBaseWidget* i_widget = CBaseWidgetList[i - 1];
		logging::Info(format("Inp: ", i).c_str());
		
		if (!i_widget) continue;		// To prevent crash
		if (!i_widget->root) continue;	// We only want roots
		if (!i_widget->visible) continue;	// root needs to be visible
		
		// We found a root, find something accepting user input
		for(CBaseWidget* ii_widget : CBaseWidgetList) {
			if (i_widget == nullptr) continue;
			if (ii_widget->root || ii_widget->root_parent != i_widget) continue;	 // Dont check any roots and stuff not part of our root-tree
			if (!i_widget->visible) continue;
			if (ii_widget->usrinput(ii_widget)) return;
		}
		
		// no elements of the root returned true so we check the root itself now
		if (i_widget->usrinput(i_widget)) return;
	}
}	
}