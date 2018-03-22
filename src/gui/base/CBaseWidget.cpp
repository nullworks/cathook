
/*
 *
 *	This is a base for widgets such as sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#include <cstring>
#include <algorithm> // std::sort

#include "../../framework/drawing.hpp" // Draw stuff
#include "../../framework/input.hpp" // Keyboard & Mouse
#include "../../util/colors.hpp" // Draw stuff
#include "../../util/logging.hpp"

#include "CBaseWidget.hpp"
#include "CBaseParent.hpp"

namespace gui { namespace base {

// Constructors
CBaseWidget::CBaseWidget(std::string name, std::string tooltip) : name(name), tooltip(tooltip){
	UpdatePositioning();
}

// General functions
void CBaseWidget::UpdatePositioning() {
	//g_CatLogging.log("Positioning %s",name.c_str());
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
		//g_CatLogging.log("Drawing %s",name.c_str());
	}
}

// User input functions
bool CBaseWidget::OnMouse(std::pair<int,int> mouse_pos, bool hover_taken){
	hover=(!hover_taken)
		&&mouse_pos.first>global_pos.first
		&&mouse_pos.second>global_pos.second
		&&mouse_pos.first<offset.first
		&&mouse_pos.second<offset.second;
}
bool CBaseWidget::OnBounds(std::pair<int,int> bounds){ }
bool CBaseWidget::TryFocusGain() 	 { focus = true; return true; }
void CBaseWidget::OnFocusLose() 	 { focus = false; }
void CBaseWidget::OnKeyPress(int key, bool repeat) {
	//g_CatLogging.log("Key %s in %s", input::key_names[key], name.c_str());
};
void CBaseWidget::OnKeyRelease(int key) {};
bool CBaseWidget::ConsumesKey(int key) { return false; }

// Get our tooltip
const std::string& CBaseWidget::GetTooltip() { return tooltip; }

}}
