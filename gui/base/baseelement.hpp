
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

namespace gui { namespace element {

class CBaseWidget {
public:
	CBaseWidget(CBaseWidget* root_parent, int layer, void(*draw)(const CBaseWidget*, rgba_t)); 
	CBaseWidget();
	~CBaseWidget();
		
public:
	CBaseWidget* root_parent;	// What root does this button belong to, if any at all
	int layer = 0;				// Makes it easier to keep track of where it is in the heierarchy
	int rootx = 0;
	int rooty = 0;
	int widthx = 0;
	int widthy = 0;
	std::string* name;   			// Depends on if its used or not. Be sure to check if its null
	bool visible = false;			// Keep it false untill someone makes it otherwise
	bool performed_last = false; 	// If the user action was performed last time.
	bool root = false;
	
	// If a widget needs a pointer to a var for refrence
	bool* child_bool;
	int* child_int;
	float* child_float;	
	std::string* child_string;
	rgba_t* child_rgba;	
	
	void(*draw)(const CBaseWidget*, rgba_t);	// We send draw requests to widgets
	bool(*usrinput)(CBaseWidget*);				// We send out request to the widget to see if its accepting userinp. it returns true if it is
};
	
extern std::vector<CBaseWidget*>& CBaseWidgetList();
}}

#endif
