
/*
 *
 * A base widget class
 *
 */

#include <functional> // std::pair
#include <string>

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
 	std::pair<int, int> offset;

 	// Naming
 	std::string name;

 	// General functions
 	virtual void Update() = 0;
 	virtual void Draw() = 0;

 	// User input functions
 	virtual void OnMouseEnter() = 0;
 	virtual void OnMouseLeave() = 0;
 	virtual void OnMousePress() = 0;
 	virtual void OnMouseRelease() = 0;
 	virtual void OnFocusGain() = 0;
 	virtual void OnFocusLose() = 0;
 	virtual bool OnKeyPress(int key, bool repeat) = 0;
 	virtual void OnKeyRelease(int key) = 0;

 	// Visibility
 	virtual void Show() = 0;
 	virtual void Hide() = 0;
 	inline bool IsVisible() { return always_visible || parent ? visible && parent->IsVisible() : visible; }

 	// Sizing
 	virtual std::pair<int, int> AbsolutePosition() = 0;

 	// Parental + children
 	inline IWidget* GetRoot() {
 		auto pParent = parent;
 		while (pParent)
 			pParent = pParent->parent;
 		return pParent;
 	}
private:
  std::pair<int, int> size;
};
