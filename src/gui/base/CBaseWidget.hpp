
/*
 *
 *	This is a base widget for things like sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#pragma once

#include "IWidget.hpp" // has a widget class that we build apon

namespace gui { namespace base {

class CBaseWidget : virtual public IWidget {
public:
	CBaseWidget(const char* _name);

	// General functions
	virtual void Update();
	virtual void Draw();
	virtual void DrawBounds();

	// User input functions
	virtual void OnMouseEnter();
	virtual void OnMouseLeave();
	virtual void OnMousePress();
	virtual void OnMouseRelease();
	virtual bool TryFocusGain();
	virtual void OnFocusLose();
	virtual void OnKeyPress(int key);
	virtual void OnKeyRelease(int key);
	virtual bool ConsumesKey(int key);

	// Visibility
	virtual void Show();
	virtual void Hide();

	// Sizing
	virtual std::pair<int, int> AbsolutePosition();

	// Naming
	virtual const std::string& GetTooltip();
};

}}
