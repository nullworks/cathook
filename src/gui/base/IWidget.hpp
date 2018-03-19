
/*
 *
 *	This is a base class for the gui to build apon.
 *
 *
 */

#pragma once

#include <utility> // Pair
#include <string>

#include "../../util/mathlib.hpp" // Colors

namespace gui { namespace base {

enum {
	FLOATING, // Widget handles own positions
	ABSOLUTE, // Absolutes are positions that are static, and depends on the side they are on. Absobutes are for things supposed to be at the top
	INLINE // Automatic management of widget requested
};

class IWidget {
public:
	IWidget* parent;
	virtual ~IWidget() = 0;

	// States
	bool hover = false;
	bool press = false;
	bool focus = false;

	// Visibility
	bool visible = true;
	bool always_visible = false;

	// Positioning
	int position_mode = FLOATING;
	std::pair<int, int> offset;
	std::pair<int, int> size = std::make_pair(0, 0);
	std::pair<int, int> minmax_size = std::make_pair(-1, -1); //Max size on things like tooltip or infobox, min size when dealing with containers.

	int zindex = 0;

	// Naming
	std::string name;
	std::string tooltip;

	// For use in show bounds
	CatVector4 bounds_color;

	// General functions
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void DrawBounds() = 0;

	// User input functions
	virtual void OnMouseEnter() = 0;
	virtual void OnMouseLeave() = 0;
	virtual void OnMousePress() = 0;
	virtual void OnMouseRelease() = 0;
	virtual bool TryFocusGain() = 0;
	virtual void OnFocusLose() = 0;
	//TODO: Text input repeat
	//virtual void OnKeyPress(int key, bool repeat) = 0;
	virtual void OnKeyPress(int key) = 0;
	virtual void OnKeyRelease(int key) = 0;
	virtual bool ConsumesKey(int key) = 0;

	// Visibility
	virtual void Show() = 0;
	virtual void Hide() = 0;
	//TODO: Consider using parent->IsVisible()
	bool IsVisible() { return always_visible || parent ? parent->visible && visible : visible; }

	// Sizing
	virtual std::pair<int, int> AbsolutePosition() = 0;

	// Naming
	virtual const std::string& GetTooltip() = 0;

	// Parental + children
	IWidget* GetRoot() {
		auto pParent = parent;
		while (pParent)
			pParent = pParent->parent;
		return pParent;
	}
};

}}
