
/*
 *
 *	This is a base for widgets such as sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#pragma once

#include "IWidget.hpp" // has a widget class that we build apon

namespace gui { namespace base {

class CBaseWidget : virtual public IWidget {
protected:
	std::string tooltip;
	CatVector4 bounds_color;
public:
	CBaseWidget(std::string name, std::string tooltip = "");

	// General functions
	virtual void UpdatePositioning();
	virtual void Draw();

	// User input functions
	virtual bool OnMouseMove(std::pair<int,int> mouse_pos, bool hover_taken);
	virtual bool TryFocusGain();
	virtual void OnFocusLose();
	virtual void OnKeyPress(int key, bool repeat);
	virtual void OnKeyRelease(int key);
	virtual bool ConsumesKey(int key);

	// Naming
	virtual const std::string& GetTooltip();
};

}}
