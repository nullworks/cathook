/*
 * TitleBar.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/drawing.hpp"
#include "GUI.h"

#include "CCTitleBar.h"



CTitleBar::CTitleBar(IWidget* parent, std::string title) : CBaseWidget("titlebar", parent) {
	m_strTitle = title;
	m_iDraggingStage = 0;
	m_nLastX = 0;
	m_nLastY = 0;
	SetPositionMode(ABSOLUTE);
}

void CTitleBar::Draw(int x, int y) {
	auto size = GetSize();
	draw::Rect(x, y, size.first, size.second, GUIColor());
	int l, h;
	draw::GetStringLength(m_strTitle.c_str(), draw::default_font, draw::default_font_size);
	draw::String(m_strTitle.c_str(), x + (size.first - l) / 2, y + TITLEBAR_PADDING_H, draw::default_font, draw::default_font_size, colors::white);
}

void CTitleBar::Update() {
	auto psize = GetParent()->GetSize();
	int l, h;
	draw::GetStringLength(m_strTitle.c_str(), draw::default_font, draw::default_font_size);
	SetSize(psize.first, 2 * TITLEBAR_PADDING_H + h);
	if (!IsPressed()) {
		m_iDraggingStage = 0;
		return;
	}
	if (m_iDraggingStage == 0) {
		m_iDraggingStage = 1;
	} else {
		int dx = g_pGUI->m_iMouseX - m_nLastX;
		int dy = g_pGUI->m_iMouseY - m_nLastY;
		auto offset = GetParent()->GetOffset();
		GetParent()->SetOffset(offset.first + dx, offset.second + dy);
	}
	m_nLastX = g_pGUI->m_iMouseX;
	m_nLastY = g_pGUI->m_iMouseY;
}
