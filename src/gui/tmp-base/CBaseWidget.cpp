/*
 * CBaseWidget.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/drawing.hpp"
#include "GUI.h"

#include "CBaseWidget.h"

void CBaseWidget::DrawBounds(int x, int y) {
	if (bounds_color == Color()) {
		bounds_color = Color(rand() % 255, rand() % 255, rand() % 255, 255);
	}
	auto size = GetSize();
	draw::RectFilled(x, y, size.first, size.second, colors::Transparent(bounds_color, 0.25f));
	draw::Rect(x, y, size.first, size.second, bounds_color);
}

bool CBaseWidget::IsHovered() {
	return g_pGUI->Visible() && m_KeyValues.GetBool("hover");
}

bool CBaseWidget::IsFocused() {
	return g_pGUI->Visible() && m_KeyValues.GetBool("focus");
}

bool CBaseWidget::IsPressed() {
	return g_pGUI->Visible() && m_KeyValues.GetBool("press");
}

CBaseWidget::CBaseWidget(std::string name, IWidget* parent) {
	m_pParent = parent;
	Props()->SetString("name", name.c_str());
	SetPositionMode(INLINE);
	Show();
	SetMaxSize(-1, -1);
}

void CBaseWidget::Update() {
	if (IsHovered() && IsVisible() && !Props()->GetString("tooltip").empty()) {
		g_pGUI->ShowTooltip(Props()->GetString("tooltip"));
	}
}

std::pair<int, int> CBaseWidget::AbsolutePosition() {
	auto result = GetOffset();
	auto parent = GetParent();
	while (parent) {
		auto poffset = parent->GetOffset();
		result.first += poffset.first;
		result.second += poffset.second;
		parent = parent->GetParent();
	}
	return result;
}
