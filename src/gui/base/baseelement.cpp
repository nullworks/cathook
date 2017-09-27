 
/*
 *
 *	A base widget to build your other types around.
 *
 */

#include "baseelement.hpp"

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
	
	// If this is a root, we attempt to find it and remove it from the main root list
	if (base_widget->root_parent == nullptr || !CBaseWidgetRoots.empty()) {

		// Attempt to find the root from the main list and remove it
		int list_size = CBaseWidgetRoots.size();
		for(int i = 0; i < list_size; i++) {
			CBaseWidget* find = CBaseWidgetRoots[i];
			if (find && base_widget == find) {
				CBaseWidgetRoots.erase(CBaseWidgetRoots.begin() + i);
			}
		}
	}
	
	// Check if we have to delete children
	if (base_widget->child_widgets.empty()) {
	
		// Recurse and delete children
		for(CBaseWidget* widget : base_widget->child_widgets) {
			if (!widget) continue;
			DeleteWidgetTree(widget);
		}
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
		PushOnTop(base_widget->root_parent);
		return;
	}
	
	// Attempt to find the root and remove it, then place it at the top
	int list_size = CBaseWidgetRoots.size();
	for(int i = 0; i < list_size; i++) {
		CBaseWidget* find = CBaseWidgetRoots[i];
		if (find && base_widget == find) {
			CBaseWidgetRoots.erase(CBaseWidgetRoots.begin() + i);
			CBaseWidgetRoots.push_back(base_widget);// Place root on top
		}
	}
}

// Carries the color from root and sets it onto your widget
rgba_t CBaseWidget::GetColor() {
	if (this->root_parent == nullptr) return this->color;
	
	// Check if we already have it
	if (!this->color_cached) { 
		this->root_parent->GetColor(); // Make the root parent get root
		this->color.r = this->root_parent->color.r;
		this->color.g = this->root_parent->color.g;
		this->color.b = this->root_parent->color.b;
		this->color_cached = true;
	}
	return this->color;
}

// Get real root
CatVector CBaseWidget::GetRealRoot() {
	if (this->root_parent == nullptr) return;
	
	// Check if we already have it
	if (!this->root_offset_cached) { 
		this->root_parent->GetRealRoot(); // Make the root parent get root
		this->root_offsetx = this->root_parent->root_offsetx + this->root_parent->rootx;
		this->root_offsety = this->root_parent->root_offsety + this->root_parent->rooty;
		this->root_offset_cached = true;
	}
	return CatVector(this->root_offsetx + this->rootx, this->root_offsety + this->rooty, 0);
}
std::vector<CBaseWidget*> CBaseWidgetRoots;


