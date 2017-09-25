
/*
 *
 *	This is a base widget. 
 *	While making your own widget, pass your draw and userinput to it.
 *
 */

#ifndef BASEWIDGETBASEGUI
#define BASEWIDGETBASEGUI

#include <string>
#include <vector>

#include "../../util/colors.hpp"

class CBaseWidget {
public:
	CBaseWidget(CBaseWidget* root_parent, void(*draw)(const CBaseWidget*)); 
	CBaseWidget();
	~CBaseWidget();
	
public:
	CBaseWidget* root_parent;	// What root does this button belong to, if any at all
	int rootx = 0;
	int rooty = 0;
	int width = 0;
	int height = 0;
	std::string* name;   			// Depends on if its used or not. Be sure to check if its null
	rgba_t color = rgba_t(0, 0, 0, 0);	// Controls the color of the object and opacity of everything in it.
	bool performed_last = false; 	// Used by elements to tell if something happened.
	bool visible = true;			// Used to tell whether drawing and user input should work.
	
	// If a widget needs a pointer to a var for refrence
	bool* child_bool;
	int* child_int;
	float* child_float;	
	std::string* child_string;
	rgba_t* child_rgba;	
	
	std::vector<CBaseWidget*> child_widgets;	// Children of the widget
	
	void(*draw)(const CBaseWidget*);	// We send draw requests to widgets
	bool(*usrinput)(CBaseWidget*);				// We send out request to the widget to see if its accepting userinp. it returns true if it is
};

void PushOnTop(CBaseWidget* base_widget);
extern std::vector<CBaseWidget*> CBaseWidgetRoots;

#endif
