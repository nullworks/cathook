/*
 * CTextInput.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: nullifiedcat
 */

#include "CTextInput.h"

#include "../common.h"
#include "../sdk.h"
#include "GUI.h"

CTextInput::CTextInput(std::string name, IWidget* parent) : CBaseWidget(name, parent) {
	Props()->SetString("value", "");
}

bool CTextInput::ConsumesKey(ButtonCode_t key) {
	return key >= ButtonCode_t::KEY_FIRST && key <= ButtonCode_t::KEY_BACKSPACE;
}

void CTextInput::SetMaxWidth(int width) {
	auto length = draw::GetStringLength(fonts::MENU, "W");
	SetSize(length.first * width + 4, length.second + 4); // TODO PADDING
}

std::string CTextInput::Value() {
	return std::string(Props()->GetString("value"));
}

void CTextInput::SetValue(std::string value) {
	std::string oldv = Value();
	if (m_pCallback)
		m_pCallback(this, oldv, value);
	Props()->SetString("value", value.c_str());
}

void CTextInput::Draw(int x, int y) {
	auto wsize = draw::GetStringLength(fonts::MENU, "W");
	auto size = GetSize();
	int color = colors::Create(0, 0, 0, 80);
	if (IsFocused()) color = colors::Transparent(GUIColor(), 0.25);
	draw::DrawRect(x, y, size.first, size.second, color);
	draw::OutlineRect(x, y, size.first, size.second, GUIColor());
	int ml = 0;
	int md = 0;
	auto dotssize = draw::GetStringLength(fonts::MENU, "..."); // TODO static?
	std::string value = Value();
	for (int i = 0; i < value.length(); i++) {
		auto strsize = draw::GetStringLength(fonts::MENU, value.substr(i));
		if (strsize.first + 10 + dotssize.first >= size.first) md = i;
		if (strsize.first + 8 > size.first) ml = i;
	}
	if (ml) {
		draw::FString(fonts::MENU, x + 2, y + 2, colors::white, 1, "...%s", value.substr(md).c_str());
	} else {
		draw::String(fonts::MENU, x + 2, y + 2, colors::white, 1, value); // TODO recalc on update
	}
}

void CTextInput::SetCallback(TextInputCallbackFn_t callback) {
	m_pCallback = callback;
}

void CTextInput::PutChar(char ch) {
	SetValue(Value() + std::string(1, ch));
}

void CTextInput::OnKeyPress(ButtonCode_t key, bool repeat) {
	if (key == ButtonCode_t::KEY_BACKSPACE) {
		std::string val = Value();
		if (val.length() > 0) {
			SetValue(val.substr(0, val.length() - 1));
		}
		return;
	} else if (key == ButtonCode_t::KEY_SPACE) {
		PutChar(' ');
		return;
	} else {
		char ch = 0;
		if (g_pGUI->m_bPressedState[ButtonCode_t::KEY_LSHIFT] || g_pGUI->m_bPressedState[ButtonCode_t::KEY_RSHIFT]) {
			ch = GetUpperChar(key);
		} else {
			ch = GetChar(key);
		}
		if (ch) {
			PutChar(ch);
		}
	}
}

