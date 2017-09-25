 
/*
 *
 *	A base widget to build your other types around.
 *
 */

//#include "../stacking.hpp"	// So we can add new "roots" to the stack

#include "divider.hpp"

CBaseWidget::CBaseWidget(CBaseWidget* root_parent, int layer, void(*draw)(const CBaseWidget*, rgba_t)) 
	: root_parent(root_parent), layer(layer), draw(draw){ 
	root = false; // Its owned by something else. Its not a root
	CBaseWidget* tmp = this;
	CBaseWidgetList.push_back(tmp); // When creating this, i want to push it to the list
}

// This is for creating a root
CBaseWidget::CBaseWidget() { 
	root = true;
	CBaseWidget* tmp = this;
	CBaseWidgetList.push_back(tmp); // When creating this, i want to push it to the list
}

CBaseWidget::~CBaseWidget() { 	
	
	// If a root was deleted, we want to delete anything this owns.
	if (root) {
		for(int i = 0; i < CBaseWidgetList.size(); i++) {
			CBaseWidget* widget = CBaseWidgetList[i];
			if (widget->root) continue;			// dont delete other/our root/s
			if (widget->root_parent == this) {	// If the parent is our root, delete it as well as in the vector
				delete widget;
				CBaseWidgetList.erase(CBaseWidgetList.begin() + i);
			}
		}
		CBaseWidgetList.shrink_to_fit();
	}
}

// Pushes a root to the top of the stack
void PushOnTop(CBaseWidget* base_widget) {
	if (!base_widget || !base_widget->root) return; // Check if root
		
	// Attempt to find the root and remove it
	int list_size = CBaseWidgetList.size();
	for(int i = 0; i < list_size; i++) {
		CBaseWidget* find = CBaseWidgetList[i];
		if (find && base_widget == find) CBaseWidgetList.erase(CBaseWidgetList.begin() + i);
	}
	
	// Place root on top
	CBaseWidgetList.push_back(base_widget);
}

// This is used to clean the list of any unused or stray elements not used.
/*void CleanStack() {
	int list_size = CBaseWidgetList.size();
	for(int i = 0; i < list_size; i++) {
		CBaseWidget* find = CBaseWidgetList[i];
		if (!find) CBaseWidgetList.erase(c.begin() + i);
	}
}*/
	
std::vector<CBaseWidget*> CBaseWidgetList;