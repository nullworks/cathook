
/*
 * TitleBar.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/input.hpp" // We use mouse stuff here
#include "../../framework/drawing.hpp" // Draw stuff
#include "../../util/colors.hpp" // Draw stuff

#include "CBaseTitleBar.hpp"

namespace gui { namespace base {

// Constructor
CBaseTitleBar::CBaseTitleBar(const char* _title) : title(_title), CBaseWidget("titlebar") {
}

/*void CBaseTitleBar::Update() {
	if (!parent) return;

	// Set our size
	auto title_size = draw::GetStringLength(title.c_str(), 1, 9);
	size = std::make_pair(parent->size.first, title_size.second + 2);

	// If we arent depressed, return
	if (!press) return;

	//TODO: mouse movement
	// Move our parent according to mouse movement
	// parent->offset = std::make_pair(input::mouse.first + last_mouse.first, input::mouse.second + last_mouse.second);

	// Save the mouse location for delta use later
	//last_mouse = input::mouse;
}

void CBaseTitleBar::OnMousePress() {
	// Save the mouse location for delta use later	
	//last_mouse = std::make_pair(parent->offset.first-input::mouse.first,parent->offset.second-input::mouse.second);
	CBaseWidget::OnMousePress();
}

*/
void CBaseTitleBar::Draw() {
	// Draw a nice rect
	draw::Rect(global_pos.first, global_pos.second, size.first, size.second, colors::Transparent(colors::pink,hover?0.7f:1.0f));
	// Draw our string
	auto title_size = draw::GetStringLength(title.c_str(), 1, 9);
	draw::String(title.c_str(), global_pos.first + (size.first - title_size.first) / 2, global_pos.second + (size.second - title_size.second) / 2, 1, 9, colors::white);
}

}}
