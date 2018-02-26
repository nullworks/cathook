/*
 * CMenuListEntry.cpp
 *
 *  Created on: Feb 3, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/drawing.hpp"
#include "CMenuList.h"
#include "GUI.h"

#include "CMenuListEntry.h"

CMenuListEntry::CMenuListEntry(std::string name, CMenuList* parent, std::string entry) : CBaseButton(name, parent, entry) {
	Props()->SetString("entry", entry.c_str());
}

void CMenuListEntry::SetMaxSize(int x, int y) {
	CBaseWidget::SetMaxSize(x, y);
	SetSize(x, y);
}

bool CMenuListEntry::IsSelected() {
	return (dynamic_cast<CMenuList*>(GetParent())->m_pSelected == this);
}

void CMenuListEntry::Draw(int x, int y) {
	auto texts = draw::GetStringLength(GetText().c_str(), draw::default_font, draw::default_font_size + 3);
	auto size = GetSize();
	if (IsSelected()) {
		draw::Line(x, y, size.first, 0, GUIColor());
		draw::Line(x, y + size.second, size.first, 0, GUIColor());
		draw::Line(x, y, 0, size.second, GUIColor());
	} else {
		draw::Rect(x, y, size.first, size.second, GUIColor());
	}
	if (IsHovered()) {
		draw::RectFilled(x, y, size.first, size.second, colors::Transparent(GUIColor(), 0.25));
	}
	draw::String(GetText().c_str(), x + (size.first - texts.first) / 2, y + (size.second - texts.second) / 2, draw::default_font, draw::default_font_size, IsSelected() ? colors::white : GUIColor());
}
