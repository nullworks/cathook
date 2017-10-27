
/*
 * 
 *	This is a base class for the gui to build apon.
 *
 *
 */

#pragma once

enum {
	ABSOLUTE,
	INLINE,
	FLOATING
};

class IWidget {
public:
	IWidget* parent;
	
	// values to replace keyvalues, they are self explanitory
	bool hover = false;
	bool press = false;
	bool focus = false;
	
	bool visible = true;
	bool always_visible = false;
	
	std::pair<int, int> offset(0, 0);
	std::pair<int, int> size(0, 0);
	std::pair<int, int> max(0, 0);
	int zindex = 0;
	
	std::string tooltip = "";
	
	int position_mode = 0;
	
	std::string name;
	
	CatVector4 bounds_color = CatVector4();
	
public:

	// General functions
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void DrawBounds() = 0;

	// User input functions
	virtual void OnMouseEnter() = 0;
	virtual void OnMouseLeave() = 0;
	virtual void OnMousePress() = 0;
	virtual void OnMouseRelease() = 0;
	virtual void OnKeyPress(int key, bool repeat) = 0;
	virtual void OnKeyRelease(int key) = 0;
	virtual bool ConsumesKey(int key) = 0;

	// visibility
	virtual void Show() = 0;
	virtual void Hide() = 0;
	virtual bool IsVisible() = 0;

	// General checking
	virtual bool IsHovered() = 0;
	virtual bool IsFocused() = 0;
	virtual bool IsPressed() = 0;

	// Sizing
	virtual void SetOffset(int x, int y) = 0;
	virtual void SetMaxSize(int x, int y) = 0;
	virtual void SetSize(int x, int y) = 0;
	virtual void SetZIndex(int idx) = 0;
	virtual std::pair<int, int> GetOffset() = 0;
	virtual std::pair<int, int> GetSize() = 0;
	virtual std::pair<int, int> GetMaxSize() = 0;
	virtual int GetZIndex() = 0;
	virtual std::pair<int, int> AbsolutePosition() = 0;

	// Hmm
	virtual std::string GetTooltip() = 0;

	// used to set position
	virtual int GetPositionMode() = 0;
	virtual void SetPositionMode(int mode) = 0;

	// Parental + children
	virtual IWidget* GetParent() = 0;
	virtual void SetParent(IWidget*) = 0;
	virtual std::string GetName() = 0;
};




