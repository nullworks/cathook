
/*
 *
 *	This is a base widget for things like sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#include <cstring>

#include "../../framework/drawing.hpp" // Draw stuff
#include "../../util/colors.hpp" // Draw stuff

#include "CBaseWidget.hpp"

namespace gui { namespace base {

// Constructors
CBaseWidget::CBaseWidget(const char* _name, IWidget* _parent) {
	strcpy(name, _name);
	parent = _parent;
	position_mode = INLINE;
	Show();
}

// General functions
void CBaseWidget::Update() {}
void CBaseWidget::Draw() {}
void CBaseWidget::DrawBounds() {
	// Set random bounds color
	if (bounds_color == CatVector4())
		bounds_color = CatVector4(rand() % 255, rand() % 255, rand() % 255, 255);
	auto abs_pos = AbsolutePosition();
	draw::RectFilled(abs_pos.first, abs_pos.second, size.first, size.second, colors::Transparent(bounds_color, 0.25f));
	draw::Rect(abs_pos.first, abs_pos.second, size.first, size.second, bounds_color);
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

// Sizing
std::pair<int, int> CBaseWidget::AbsolutePosition() {
	auto result = offset;
	auto pParent = parent;
	while (pParent) {
		auto poffset = pParent->offset;
		result.first += poffset.first;
		result.second += poffset.second;
		pParent = pParent->parent;
	}
	return result;
}

// Get our tooltip
const char* CBaseWidget::GetTooltip() { return tooltip; }

}}
