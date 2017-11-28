
/*
 *
 *	This is a base widget for things like sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#include "CBaseWidget.hpp"

namespace gui { namespace base {

// Constructors
CBaseWidget::CBaseWidget(std::string _name, IWidget* _parent) {
	parent = _parent;
	name = _name;
	SetPositionMode(INLINE);
	Show();
	SetMaxSize(-1, -1);
}

// General functions
void CBaseWidget::Update() {
	// Shows our tooltip
	if (IsHovered() && IsVisible() && tooltip != "") {
		auto pRoot = GetRootParent()
		pRoot->tooltip = tooltip;
	}
}
void CBaseWidget::Draw() {}
void CBaseWidget::DrawBounds() {
	if (!bounds_color) {
		bounds_color = CatVector4(rand() % 255, rand() % 255, rand() % 255, 255);
	}
	auto pRoot = AbsolutePosition();
	draw::RectFilled(pRoot.first, pRoot.second, size.first, size.second, colors::Transparent(bounds_color, 0.25f));
	draw::Rect(pRoot.first, pRoot.second, size.first, size.second, bounds_color);
}

// User input functions
void CBaseWidget::OnMouseEnter()	{ hover = true; }
void CBaseWidget::OnMouseLeave() 	 { hover = false; }
void CBaseWidget::OnMousePress() 	 { press = true; }
void CBaseWidget::OnMouseRelease() { press = false; }
void CBaseWidget::OnFocusGain() 	 { focus = true; }
void CBaseWidget::OnFocusLose() 	 { focus = false; }
void CBaseWidget::OnKeyPress(int key, bool repeat) {};
void CBaseWidget::OnKeyRelease(int key) {};
bool CBaseWidget::ConsumesKey(int key) { return false; }

// Visibility
void CBaseWidget::Show() { visible = true; }
void CBaseWidget::Hide() { visible = false; }
bool CBaseWidget::IsVisible() { return ((GetParent()) ? (GetParent()->visible && visible) : visible) || always_visible; }

// General checking
bool CBaseWidget::IsHovered() { return hover; }
bool CBaseWidget::IsFocused() { return focus; }
bool CBaseWidget::IsPressed() { return press; }

// Sizing
void CBaseWidget::SetSize(int x, int y) 	 { if (x >= 0) size.first = x;   if (y >= 0) size.second = y; }
void CBaseWidget::SetOffset(int x, int y)  { if (x >= 0) offset.first = x; if (y >= 0) offset.second = y; }
void CBaseWidget::SetMaxSize(int x, int y) { if (x >= 0) max.first = x; 	 if (y >= 0) max.second = y; }
std::pair<int, int> CBaseWidget::GetOffset() { return offset; }
std::pair<int, int> CBaseWidget::GetSize() { return size; }
std::pair<int, int> CBaseWidget::GetMaxSize() { return max; }
int CBaseWidget::GetZIndex() { return zindex; }
void CBaseWidget::SetZIndex(int idx) { zindex = idx; }
std::pair<int, int> CBaseWidget::AbsolutePosition() {
	auto result = GetOffset();
	auto pParent = GetParent();
	while (pParent) {
		auto poffset = pParent->GetOffset();
		result.first += poffset.first;
		result.second += poffset.second;
		pParent = pParent->GetParent();
	}
	return result;
}

// hmm
std::string CBaseWidget::GetTooltip() { return tooltip; }

// Used to get/set position
int CBaseWidget::GetPositionMode() { return position_mode; }
void CBaseWidget::SetPositionMode(int mode) { position_mode = mode; };

// Parental + children
IWidget* CBaseWidget::GetParent() { return parent; }
void CBaseWidget::SetParent(IWidget* _parent) { parent = _parent; }
IWidget* CBaseWidget::GetRootParent() {
	auto pParent = GetParent();
	while (pParent)
		pParent = pParent->GetParent();
	return pParent;
}
std::string CBaseWidget::GetName() { return name; }

}}
