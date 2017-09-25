 
/*
 *
 *	A base widget to build your other types around.
 *
 */

//#include "../stacking.hpp"	// So we can add new "roots" to the stack

#include "divider.hpp"

// Used by elements for mouse actions
static int widget_mouseoffsetx;
static int widget_mouseoffsety;

CBaseWidget::CBaseWidget(CBaseWidget* root_parent, void(*draw)(CBaseWidget*)) 
	: root_parent(root_parent), draw(draw){ 
	CBaseWidget* tmp = this;
	root_parent->child_widgets.push_back(tmp); // When creating this, i want to push it to the parent
}

// This is for creating a root
CBaseWidget::CBaseWidget() { 
	CBaseWidget* tmp = this;
	CBaseWidgetRoots.push_back(tmp); // When creating this, i want to push it to the root list
}

// Deletes a widget as well as any children
void DeleteWidgetTree(CBaseWidget* base_widget) {
	if (!base_widget) return;
	
	// If there are no children, we our var and return, else we recurse
	if (base_widget->child_widgets.empty()) {
		delete base_widget;
		return;
	}
	
	// Recurse and delete children
	for(CBaseWidget* widget : base_widget->child_widgets) {
		if (!widget) continue;
		DeleteWidgetTree(widget);
	}
	// We are done deleting children, suicide is only option
	delete base_widget;
}

CBaseWidget::~CBaseWidget() { 	
	
	// If something has children, we want to delete them... Recursivly... Fuck children lol
	if (!child_widgets.empty()) {
		
		// Delete everything in the tree
		DeleteWidgetTree(this);
	}
}

// Pushes a root to the top of the stack
void PushOnTop(CBaseWidget* base_widget) {
	if (!base_widget) return;
	
	// If this has a parent, we send the parent through this function instead.
	if (base_widget->root_parent != nullptr) {
		PushOnTop(base_widget);
		return;
	}
	
	// Attempt to find the root and remove it
	int list_size = CBaseWidgetRoots.size();
	for(int i = 0; i < list_size; i++) {
		CBaseWidget* find = CBaseWidgetRoots[i];
		if (find && base_widget == find) {
			CBaseWidgetRoots.erase(CBaseWidgetRoots.begin() + i);
			CBaseWidgetRoots.push_back(base_widget);// Place root on top
		}
	}
}
	
std::vector<CBaseWidget*> CBaseWidgetRoots;


