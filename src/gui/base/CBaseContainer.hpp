
/*
 *
 *	An IWidget varient base designed to house child widgets
 *
 */

#pragma once

#include <vector>

#include "CBaseWidget.hpp"

namespace gui { namespace base {

class CBaseContainer : public CBaseWidget {
public:
	CBaseContainer(const char* _name);
	~CBaseContainer();

	virtual void Update();
	virtual void Draw();
	virtual void DrawBounds();

	// User input functions
	virtual void OnMouseLeave();
	virtual void OnMousePress();
	virtual void OnMouseRelease();
	virtual bool TryFocusGain();
	virtual void OnFocusLose();
	virtual void OnKeyPress(int key);
	virtual void OnKeyRelease(int key);
	virtual bool ConsumesKey(int key);

	// Visibility
	virtual void Hide();

	// Tooltips
	virtual const std::string& GetTooltip();

	// Children
	std::vector<IWidget*> children;
	std::vector<IWidget*> childrenZSort;
	void AddChild(IWidget* child);

	// Get Child/info
	IWidget* ChildByIndex(int idx);
	IWidget* ChildByName(const char* name);
	int ChildByPoint(int x, int y);

	// Child related util
	virtual void SortByZIndex();
	void UpdateHovers();
	virtual void MoveChildren();

	// Child info related to the container
	void HoverOn(int child);
	bool TryFocusOn(int child);
	void PressOn(int child);
	int hovered_child = -1;
	int focused_child = -1;
	int pressed_child = -1;
	bool can_focus_on_nothing = false;// Not a guarantee
	bool hover_is_focus = false;

	//Column things
	int columnSpacing = 4;
	int defaultColumnWidth=100;//TODO: CatVar?
	std::vector<int> columnWidth;
};

}}
