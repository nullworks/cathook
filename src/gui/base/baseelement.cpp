 
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
	//PushOnTop(this);	// Put new root on top
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
	
std::vector<CBaseWidget*> CBaseWidgetList;