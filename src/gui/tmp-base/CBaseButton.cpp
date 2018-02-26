/*
 * CBaseButton.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/drawing.hpp"
#include "GUI.h"

#include "CBaseButton.h"

CBaseButton::CBaseButton(std::string name, IWidget* parent, std::string text, ButtonCallbackFn_t callback) : CTextLabel(name, parent, text) {
	SetPadding(BUTTON_PADDING_W, BUTTON_PADDING_H);
	SetText(text);
	if (callback) SetCallback(callback);
}

void CBaseButton::SetCallback(ButtonCallbackFn_t callback) {
	m_pCallback = callback;
}

void CBaseButton::Draw(int x, int y) {
	auto textcolor = GUIColor();
	auto size = GetSize();
	if (IsPressed()) {
		draw::Rect(x, y, size.first, size.second, GUIColor());
		textcolor = colors::white;
	}
	draw::RectFilled(x, y, size.first, size.second, GUIColor());
	draw::String(GetText().c_str(), x + Props()->GetInt("padding_x"), y + Props()->GetInt("padding_y"), draw::default_font, draw::default_font_size, textcolor);
}

void CBaseButton::OnMousePress() {
	CBaseWidget::OnMousePress();
	if (m_pCallback) {
		m_pCallback(this);
	}
}
