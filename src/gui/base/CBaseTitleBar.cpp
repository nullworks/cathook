
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
CTitleBar::CTitleBar(const char* _title) : title(_title), CBaseWidget("titlebar") {
	position_mode = ABSOLUTE;
}

void CTitleBar::Update() {
	if (!parent) return;

	// Set our size
	auto title_size = draw::GetStringLength(title.c_str(), 1, 9);
	size = std::make_pair(parent->size.first, title_size.second + 2);

	// If we arent depressed, reset dragging
	if (!press) {
		dragging = false;
		return;
	}
	// If we arent dragging already, we need to skip move so we can get a delta
	if (!dragging)
		dragging = true;
	// If we already have a delta, we can move here
	else {
		// Get the delta change
		auto delta = std::make_pair(input::mouse.first - last_mouse.first, input::mouse.second - last_mouse.second);
		// Apply the delta
		parent->offset = std::make_pair(offset.first + delta.first, offset.second + delta.second);
	}
	// Save the mouse location for delta use later
	last_mouse = input::mouse;
}

void CTitleBar::Draw() {
	// Draw a nice rect
	auto abs = AbsolutePosition();
	draw::Rect(abs.first, abs.second, size.first, size.second, colors::pink);
	// Draw our string
	auto title_size = draw::GetStringLength(title.c_str(), 1, 9);
	draw::String(title.c_str(), abs.first + (size.first - title_size.first) / 2, abs.second + (size.second - title_size.second) / 2, 1, 9, colors::white);
}

}}
