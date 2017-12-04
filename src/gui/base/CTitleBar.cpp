
/*
 * TitleBar.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/input.hpp" // We use mouse stuff here
#include "../../framework/drawing.hpp" // Draw stuff
#include "../../util/colors.hpp" // Draw stuff

#include "CTitleBar.hpp"

#define TITLEBAR_PADDING_W 2
#define TITLEBAR_PADDING_H 1

namespace gui { namespace base {

enum {
	EDRAG_START,
	EDRAG_CONT
};

// Constructor
CTitleBar::CTitleBar(std::string _title, IWidget* _parent) : CBaseWidget("titlebar", parent), title(_title) {
	SetPositionMode(ABSOLUTE);
}

void CTitleBar::Update() {
	auto psize = GetParent()->GetSize();
	auto str_size = draw::GetStringLength(title.c_str(), 1, 9);
	SetSize(psize.first, 2 * TITLEBAR_PADDING_H + str_size.first);
	if (!IsPressed()) {
		drag_stage = EDRAG_START;
		return;
	}
	if (drag_stage == EDRAG_START) {
		drag_stage = EDRAG_CONT;
	} else {
		int dx = input::mouse.first - last_mouse.first;
		int dy = input::mouse.second - last_mouse.second;
		auto offset = GetParent()->GetOffset();
		GetParent()->SetOffset(offset.first + dx, offset.second + dy);
	}
	last_mouse = input::mouse;
}

void CTitleBar::Draw() {
	auto abs = AbsolutePosition();
	auto size = GetSize();
	draw::Rect(abs.first, abs.second, size.first, size.second, colors::pink);
	auto str_size = draw::GetStringLength(title.c_str(), 1, 9);
	draw::String(title.c_str(), abs.first + (size.first - str_size.first) / 2, abs.second + TITLEBAR_PADDING_H, 1, 9, colors::white);
}

}}
