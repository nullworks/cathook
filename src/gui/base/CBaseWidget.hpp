
/*
 *
 *	This is a base for widgets such as sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#pragma once
#include <functional> // Pair
#include <string>

#include "../../util/colors.hpp" // Colors

namespace gui { namespace base {

//Forward decl
class CBaseParent;

class CBaseWidget{
protected:
	std::string tooltip;
	CatColor bounds_color;
	std::pair<int, int> global_pos;
public:
	std::string name;
	CBaseParent *parent=nullptr;
	CBaseWidget(std::string name, std::string tooltip = "");
	virtual ~CBaseWidget(){} // needs to be virtual, to enable deleting
	// States
	//TODO: Add Mouse support to UI
	//bool press = false;
	bool hover = false;
	bool focus = false;


	// Positioning
	inline std::pair<int, int> GetGlobalPos(){ return global_pos; };
	std::pair<int, int> offset;
	std::pair<int, int> size;
	virtual void UpdatePositioning();

	// Graphics
	bool visible = true;
	bool draw_bounds = false;
	virtual void Draw();

	// Input
	virtual bool OnMouse(std::pair<int,int> mouse_pos, bool hover_taken);
	virtual void OnBounds(std::pair<int,int> bounds);
	virtual bool TryFocusGain();
	virtual void OnFocusLose();
	virtual void OnKeyPress(int key);
	virtual void OnKeyRelease(int key);
	virtual bool ConsumesKey(int key);

	// Naming
	virtual const std::string& GetTooltip();
};

}}
