
/*
 * TitleBar.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/inputmgr.hpp" // We use mouse stuff here
#include "../../framework/drawing.hpp"  // We use mouse stuff here

#include "CTitleBar.hpp"

#define TITLEBAR_PADDING_W 2
#define TITLEBAR_PADDING_H 1

namespace gui { namespace base {

enum {
	EDRAG_START,
	EDRAG_CONT
}

// Constructor
CTitleBar::CTitleBar(std::string _title, IWidget* _parent) : CBaseWidget("titlebar", parent) {
	title = _title;
	m_iDraggingStage = 0;
	m_nLastX = 0;
	m_nLastY = 0;
	SetPositionMode(ABSOLUTE);
}

void CTitleBar::Update() {
	auto psize = GetParent()->GetSize();
	int l, h;
	draw::GetStringLength(title.c_str(), 1, 9, l, h);
	SetSize(psize.first, 2 * TITLEBAR_PADDING_H + h);
	if (!IsPressed()) {
		drag_stage = EDRAG_START;
		return;
	}
	if (drag_stage == EDRAG_START) {
		drag_stage = EDRAG_CONT;
	} else {
		int dx = input::mouse.first  - m_nLastX;
		int dy = input::mouse.second - m_nLastY;
		auto offset = GetParent()->GetOffset();
		GetParent()->SetOffset(offset.first + dx, offset.second + dy);
	}
	last_mouse = input::mouse;
}

void CTitleBar::Draw() {
	auto size = GetSize();
	draw::Rect(x, y, size.first, size.second, colors::pink);
	int l, h;
	draw::GetStringLength(title.c_str(), 1, 9, l, h);
	draw::String(title.c_str(), x + (size.first - l) / 2, y + TITLEBAR_PADDING_H, 1, 9, colors::white);
}

}}
