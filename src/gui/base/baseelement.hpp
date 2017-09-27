
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

#include "../../util/mathlib.hpp"
#include "../../util/colors.hpp"

class CBaseWidget {
public:
	CBaseWidget(CBaseWidget* root_parent, void(*draw)(CBaseWidget*)); 
	CBaseWidget();
	~CBaseWidget();
	
public:
	CBaseWidget* root_parent = nullptr;			// What root does this button belong to, if any at all
	std::vector<CBaseWidget*> child_widgets;	// Children of the widget
	
	int rootx = 0;
	int rooty = 0;
	int width = 0;
	int height = 0;
	
	CatVector GetRealRoot();
	bool root_offset_cached = false;
	int root_offsetx = 0;	// Please set to your parents root offset so we can keep track of where stuff is.
	int root_offsety = 0;
	
	rgba_t GetColor();
	bool color_cached = false;
	rgba_t color = rgba_t(0, 0, 0, 0);	// Controls the color of the object and opacity of everything in it.
	
	std::string name;					// Depends on if its used or not.
	std::string description;			// Sometimes we want to describe
	
	float position;						// Used to keep persice positions on some elements
	float min_position = 0;				// How far elements are allowed to set things.
	float max_position = 100;
	
	bool visible = true;				// Used to tell whether drawing and user input should work.
	bool performed_last = false; 		// Used by elements to tell if something happened.
	int extra_ints[4];					// Extra stuff to use just cuz
	
	
	// If a widget needs a pointer to a var for refrence
	bool* child_bool = nullptr;
	int* child_int = nullptr;
	float* child_float = nullptr;	
	std::string* child_string = nullptr;
	rgba_t* child_rgba = nullptr;
	
	void(*draw)(CBaseWidget*);					// We send draw requests to widgets
	bool(*usrinput)(CBaseWidget*) = nullptr;	// We send out request to the widget to see if its accepting userinp. it returns true if it is
};

// These carry color to children during draw and input
void CarryColor(CBaseWidget* base_widget);
void CarryRootOffset(CBaseWidget* base_widget);

void DeleteWidgetTree(CBaseWidget* base_widget);	// Im giving this to the menu for it to use
void PushOnTop(CBaseWidget* base_widget);			// Great for controling stacking
extern std::vector<CBaseWidget*> CBaseWidgetRoots;

extern int widget_mouseoffsetx;
extern int widget_mouseoffsety;

#endif
