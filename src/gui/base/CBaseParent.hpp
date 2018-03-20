/*
 * 
 * Base class for widgets with children.
 * Examples include: Root, Window, and Inliner (prev known as container)
 * 
 */

#pragma once

#include <vector>

#include "CBaseWidget.hpp"

namespace gui { namespace base {

class CBaseParent : public CBaseWidget {
public:
    CBaseParent(std::string name="", std::string tooltip="");
	~CBaseParent();

	// General functions
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

	// Children
	std::vector<IWidget*> children;
	void AddChild(IWidget* child);

	// Get Child/info
	IWidget* ChildByIndex(int idx);
	IWidget* ChildByName(const char* name);

	// Child info related to the container
	bool TryFocusOn(int child);
	//void PressOn(int child);
	int hovered_child = -1;
	int focused_child = -1;
	//int pressed_child = -1;
	bool can_focus_on_nothing = false;// Not a guarantee
	bool hover_is_focus = false;
};

}}
