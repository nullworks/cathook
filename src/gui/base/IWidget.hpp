
/*
 *
 *	A base widget class
 *
 *
 */

#pragma once

#include <functional> // Pair
#include <string>

#include "../../util/mathlib.hpp" // Colors

//Forward decl
class CBaseParent;

class IWidget {
public:
	IWidget* parent;
	virtual ~IWidget() = 0;

	// States
	bool hover = false;
	//TODO: Add Mouse support to UI
	//bool press = false;
	bool focus = false;

	// Visibility
	bool visible = true;

	// Positioning
protected:
	std::pair<int, int> global_pos;
public:
	inline std::pair<int, int> GetGlobalPos(){ return global_pos; };
	std::pair<int, int> offset;
	std::pair<int, int> size;
	virtual void UpdatePositioning() = 0;

	// Naming
	std::string name;

	// Graphics
	bool draw_bounds = false;
	virtual void Draw() = 0;

	// User input functions
	//If true, this widget or a child has taken the hover
	virtual bool OnMouseMove(std::pair<int,int> mouse_pos, bool hover_taken) = 0;
	virtual bool TryFocusGain() = 0;
	virtual void OnFocusLose() = 0;
	virtual void OnKeyPress(int key, bool repeat) = 0;
	virtual void OnKeyRelease(int key) = 0;
	virtual bool ConsumesKey(int key) = 0;

	// Visibility
	bool IsVisible() { return visible && (!parent || parent->IsVisible()); }

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
