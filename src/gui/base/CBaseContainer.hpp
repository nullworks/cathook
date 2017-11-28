
/*
 *
 *	An IWidget varient base designed to house child widgets
 *
 */

#pragma once

#include <vector>
#include <string>

#include "CBaseWidget.hpp"

namespace gui { namespace base {

class CBaseContainer : public CBaseWidget {
public:
	CBaseContainer(std::string _name = "unnamed", IWidget* _parent = nullptr);
	virtual ~CBaseContainer(); // required to be virtual to prevent destructor override

	virtual void Update();
	virtual void Draw();
	virtual void DrawBounds();

	// User input functions
	virtual void OnMouseLeave();
	virtual void OnMousePress();
	virtual void OnMouseRelease();
	virtual void OnFocusLose();
	virtual void OnKeyPress(int key, bool repeat);
	virtual void OnKeyRelease(int key);
	virtual bool ConsumesKey(int key);

	// Visibility
	virtual void Hide();

	// Children
	std::vector<IWidget*> children;
	void AddChild(IWidget* child);

	// Get Child/info
	IWidget* ChildByIndex(int idx);
	IWidget* ChildByName(std::string name);
	IWidget* ChildByPoint(int x, int y);
	int ChildCount();

	// Child related util
	virtual void SortByZIndex();
	void UpdateHovers();
	virtual void MoveChildren();

	// Child info related to the container
	IWidget* GetHoveredChild();
	IWidget* GetFocusedChild();
	IWidget* GetPressedChild();
	void HoverOn(IWidget* child);
	void FocusOn(IWidget* child);
	void PressOn(IWidget* child);
	IWidget* hovered_child = nullptr;
	IWidget* focused_child = nullptr;
	IWidget* pressed_child = nullptr;
};

}}
