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
	virtual ~CBaseParent();

	// General functions
	virtual void Draw();
	virtual void UpdatePositioning();

	// User input functions
	virtual bool OnMouse(std::pair<int,int> mouse_pos, bool hover_taken);
	virtual void OnBounds(std::pair<int,int> bounds);
	virtual bool TryFocusGain();
	virtual void OnFocusLose();
	virtual void OnKeyPress(int key);
	virtual void OnKeyRelease(int key);
	virtual bool ConsumesKey(int key);

	// Naming
	virtual const std::string& GetTooltip();

	// Children
	std::vector<CBaseWidget*> children;
	void AddChild(CBaseWidget* child);

	// Get Child/info
	CBaseWidget* ChildByIndex(int idx);
	CBaseWidget* ChildByName(const char* name);

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
