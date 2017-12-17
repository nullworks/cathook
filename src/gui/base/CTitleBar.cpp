
/*
 * TitleBar.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include <cstring>

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
CTitleBar::CTitleBar(const char* _title, IWidget* _parent) : CBaseWidget("titlebar", parent) {
	strcpy(title, _title);
	position_mode = ABSOLUTE;
}

void CTitleBar::Update() {
	auto psize = parent->size;
	auto str_size = draw::GetStringLength(title, 1, 9);
	size = std::make_pair(psize.first, 2 * TITLEBAR_PADDING_H + str_size.first);
	if (!press) {
		drag_stage = EDRAG_START;
		return;
	}
	if (drag_stage == EDRAG_START) {
		drag_stage = EDRAG_CONT;
	} else {
		int dx = input::mouse.first - last_mouse.first;
		int dy = input::mouse.second - last_mouse.second;
		auto offset = parent->offset;
		parent->offset = std::make_pair(offset.first + dx, offset.second + dy);
	}
	last_mouse = input::mouse;
}

void CTitleBar::Draw() {
	auto abs = AbsolutePosition();
	draw::Rect(abs.first, abs.second, size.first, size.second, colors::pink);
	auto str_size = draw::GetStringLength(title, 1, 9);
	draw::String(title, abs.first + (size.first - str_size.first) / 2, abs.second + TITLEBAR_PADDING_H, 1, 9, colors::white);
}

}}
