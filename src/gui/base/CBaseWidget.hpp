
/*
 *
 *	This is a base widget for things like sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#pragma once

#include <algorithm> // std::sort

#include "IWidget.hpp" // has a widget class that we build apon

namespace gui { namespace base {

class CBaseWidget : public IWidget {
public:
	CBaseWidget(const char* _name = "unnamed", IWidget* _parent = nullptr);

	// General functions
	virtual void Update();
	virtual void Draw();
	virtual void DrawBounds();

	// User input functions
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual void OnMousePress();
	virtual void OnMouseRelease();
	virtual void OnFocusGain();
	virtual void OnFocusLose();
	virtual void OnKeyPress(int key, bool repeat);
	virtual void OnKeyRelease(int key);
	virtual bool ConsumesKey(int key);

	// Visibility
	virtual void Show();
	virtual void Hide();

	// Sizing
	virtual std::pair<int, int> AbsolutePosition();

	// Naming
	virtual const char* GetTooltip();
};

}}
