
/*
 *
 *	This is a base widget for things like sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#include <cstring>
#include <algorithm> // std::sort

#include "../../framework/drawing.hpp" // Draw stuff
#include "../../framework/input.hpp" // Draw stuff
#include "../../util/colors.hpp" // Draw stuff

#include "CBaseWidget.hpp"

namespace gui { namespace base {

// Constructors
CBaseWidget::CBaseWidget(const char* _name) {
	name = _name;
	position_mode = INLINE;
	Show();
}

// General functions
void CBaseWidget::Update() {}
void CBaseWidget::Draw() {
	//TODO: Remove hover debug
	//if(hover) DrawBounds();
}
void CBaseWidget::DrawBounds() {
	//If the bounds color is unset
	if (bounds_color == CatVector4())
		// Set random bounds color
		bounds_color = CatVector4(rand() % 255, rand() % 255, rand() % 255, 255);
	auto abs_pos = AbsolutePosition();
	draw::RectFilled(abs_pos.first, abs_pos.second, size.first, size.second, colors::Transparent(bounds_color, 0.25f));
	draw::Rect(abs_pos.first, abs_pos.second, size.first, size.second, bounds_color);
}

// User input functions
void CBaseWidget::OnMouseEnter()	{ hover = true; }
void CBaseWidget::OnMouseLeave() 	 { hover = false; }
void CBaseWidget::OnMousePress(){
  auto abs = AbsolutePosition();
  //TODO: Mouse press
  //auto mf = input::mouse.first-abs.first;
  //auto ms = input::mouse.second-abs.second;
  //if(mf>0&&ms>0&&mf<size.first&&ms<size.second){
  //  press = true;
  //}
}
void CBaseWidget::OnMouseRelease() { press = false; }
bool CBaseWidget::TryFocusGain() 	 { focus = true; return true; }
void CBaseWidget::OnFocusLose() 	 { focus = false; }
void CBaseWidget::OnKeyPress(int key) {};
void CBaseWidget::OnKeyRelease(int key) {};
bool CBaseWidget::ConsumesKey(int key) { return false; }

// Visibility
void CBaseWidget::Show() { visible = true; }
void CBaseWidget::Hide() { visible = false; }

// Sizing
//TODO: Optimization: Have parent pass own absolute position to child at drawtime
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
const std::string& CBaseWidget::GetTooltip() { return tooltip; }

}}
