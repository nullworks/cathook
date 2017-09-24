
/*
 *
 *	This is the stack manager, it keeps windows stacked in an order. 
 *	
 *
 */

#include <algorithm> //std::find()

#include "stacking.hpp"

namespace gui { namespace stack {
/*
// Pushes a root to the top of the stack
void PushOnTop(CBaseWidget* base_widget) {
	if (!base_widget->root) return; // Check if root
		
	// Attempt to find the root and remove it
	auto item = std::find(CBaseWidgetList.begin(), CBaseWidgetList.end(), base_widget);
	if (item != CBaseWidgetList.end()) 
		CBaseWidgetList.erase(item);
	
	// Place root on top
	CBaseWidgetList.push_front(base_widget);
}

// Pushes a root to the bottom of the stack
void PushToBottom(CBaseWidget* base_widget) {
	if (!base_widget->root) return; // Check if root
		
	// Attempt to find the root and remove it
	auto item = std::find(CBaseWidgetList.begin(), CBaseWidgetList.end(), base_widget);
	if (item != CBaseWidgetList.end()) 
		CBaseWidgetList.erase(item);
	
	// Place root on top
	CBaseWidgetList.push_back(base_widget);
}

// Use to find if its in the stack
bool IsInStack(CBaseWidget* base_widget) {
	if (!base_widget->root) return false; // Check if root
	
	auto item = std::find(CBaseWidgetList.begin(), CBaseWidgetList.end(), base_widget);
	if (item != CBaseWidgetList.end()) return false;
	return true;
}

// This is used to clean base elements not part of the stack.
void CleanStack() {
	for(CBaseWidget* n : CBaseWidgetList) {
		
	}
}*/

}}