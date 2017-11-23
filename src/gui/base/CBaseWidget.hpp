
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
	virtual void Draw() {};
	virtual void DrawBounds();

	// User input functions
	inline virtual void OnMouseEnter()	 { hover = true; }
	inline virtual void OnMouseLeave() 	 { hover = false; }
	inline virtual void OnMousePress() 	 { press = true; }
	inline virtual void OnMouseRelease() { press = false; }
	inline virtual void OnFocusGain() 	 { focus = true; }
	inline virtual void OnFocusLose() 	 { focus = false; }
	inline virtual void OnKeyPress(int key, bool repeat) {};
	inline virtual void OnKeyRelease(int key) {};
	inline virtual bool ConsumesKey(int key) { return false; }

	// Visibility
	inline virtual void Show() { visible = true; }
	inline virtual void Hide() { visible = false; }
	inline virtual bool IsVisible() { return ((GetParent()) ? (GetParent()->visible && visible) : visible) || always_visible; }

	// General checking
	inline virtual bool IsHovered() { return hover; }
	inline virtual bool IsFocused() { return focus; }
	inline virtual bool IsPressed() { return press; }

	// Sizing
	inline 		     void SetSize(int x, int y) 	 { if (x >= 0) size.first = x;   if (y >= 0) size.second = y; }
	inline virtual void SetOffset(int x, int y)  { if (x >= 0) offset.first = x; if (y >= 0) offset.second = y; }
	inline virtual void SetMaxSize(int x, int y) { if (x >= 0) max.first = x; 	 if (y >= 0) max.second = y; }
	inline virtual std::pair<int, int> GetOffset() { return offset; }
	inline virtual std::pair<int, int> GetSize() { return size; }
	inline virtual std::pair<int, int> GetMaxSize() { return max; }
	inline virtual int GetZIndex() { return zindex; }
	inline virtual void SetZIndex(int idx) { zindex = idx; }

	// hmm
	inline virtual std::string GetTooltip() { return tooltip; }

	// Used to get/set position
	inline virtual int GetPositionMode() { return position_mode; }
	inline virtual void SetPositionMode(int mode) { position_mode = mode; };

	// Parental + children
	inline virtual IWidget* GetParent() { return parent; }
	inline virtual void SetParent(IWidget* _parent) { parent = _parent; }
	inline virtual std::string GetName() { return name; }

	// Widget specific
	virtual std::pair<int, int> AbsolutePosition();
};

}}
