
/*
 *
 *	This is a base for widgets such as sliders, checkboxes, dropdowns, etc.
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
CBaseWidget::CBaseWidget(std::string name, std::string tooltip){
	this->name=name;
	this->tooltip=tooltip;
}

// General functions
void CBaseWidget::UpdatePositioning() {
	//BaseWidget is inline by default
	if(parent){
		auto pgpos = parent->GetGlobalPos();
		global_pos = std::make_pair(offset.first+pgpos.first, offset.second+pgpos.second);
	}else{
		//No parent
		global_pos = offset;
	}
}

void CBaseWidget::Draw() {
	if(draw_bounds){
		//If the bounds color is unset, set random bounds color
		if (bounds_color == CatVector4())
			bounds_color = colors::RandomColor();
		//Draw it
		draw::RectFilled(global_pos.first, global_pos.second, size.first, size.second, colors::Transparent(bounds_color, 0.25f));
		draw::Rect(global_pos.first, global_pos.second, size.first, size.second, bounds_color);
	}
}

// User input functions
bool CBaseWidget::OnMouseMove(std::pair<int,int> mouse_pos, bool hover_taken){ return false; }
bool CBaseWidget::TryFocusGain() 	 { focus = true; return true; }
void CBaseWidget::OnFocusLose() 	 { focus = false; }
void CBaseWidget::OnKeyPress(int key, bool repeat) {};
void CBaseWidget::OnKeyRelease(int key) {};
bool CBaseWidget::ConsumesKey(int key) { return false; }

// Get our tooltip
const std::string& CBaseWidget::GetTooltip() { return tooltip; }

}}
