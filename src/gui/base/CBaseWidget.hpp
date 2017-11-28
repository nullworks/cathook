
/*
 *
 *	This is a base widget for things like sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#pragma once

#include "IWidget.hpp" // has a widget class that we build apon

namespace gui { namespace base {

class CBaseWidget : public virtual IWidget {
public:
	CBaseWidget(std::string _name = "unnamed", IWidget* _parent = nullptr);

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
	virtual bool IsVisible();

	// General checking
	virtual bool IsHovered();
	virtual bool IsFocused();
	virtual bool IsPressed();

	// Sizing
	virtual void SetSize(int x, int y);
	virtual void SetOffset(int x, int y);
	virtual void SetMaxSize(int x, int y);
	virtual std::pair<int, int> GetOffset();
	virtual std::pair<int, int> GetSize();
	virtual std::pair<int, int> GetMaxSize();
	virtual int GetZIndex();
	virtual void SetZIndex(int idx);
	virtual std::pair<int, int> AbsolutePosition();

	// hmm
	virtual std::string GetTooltip();

	// Used to get/set position
	virtual int GetPositionMode();
	virtual void SetPositionMode(int mode);

	// Parental + children
	virtual IWidget* GetParent();
	virtual void SetParent(IWidget* _parent);
	virtual IWidget* GetRootParent();
	virtual std::string GetName();
};

}}
