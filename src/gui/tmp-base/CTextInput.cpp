/*
 * CTextInput.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: nullifiedcat
 */

#include "CTextInput.h"

#include "GUI.h"

CTextInput::CTextInput(std::string name, IWidget* parent) : CBaseWidget(name, parent) {
	Props()->SetString("value", "");
}

bool CTextInput::ConsumesKey(int key) {
	return strlen(input::key_names[key]) == 1 || key == CATKEY_BACKSPACE;
}

void CTextInput::SetMaxWidth(int width) {
	auto length = draw::GetStringLength("W", draw::default_font, draw::default_font_size);
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
	auto wsize = draw::GetStringLength("W", draw::default_font, draw::default_font_size);
	auto size = GetSize();
	auto color = Color(0, 0, 0, 80);
	if (IsFocused()) color = colors::Transparent(GUIColor(), 0.25);
	draw::RectFilled(x, y, size.first, size.second, color);
	draw::Rect(x, y, size.first, size.second, GUIColor());
	int ml = 0;
	int md = 0;
	auto dotssize = draw::GetStringLength("...", draw::default_font, draw::default_font_size); // TODO static?
	std::string value = Value();
	for (int i = 0; i < value.length(); i++) {
		auto strsize = draw::GetStringLength(value.substr(i).c_str(), draw::default_font, draw::default_font_size);
		if (strsize.first + 10 + dotssize.first >= size.first) md = i;
		if (strsize.first + 8 > size.first) ml = i;
	}
	if (ml) {
		draw::String((std::string("...") + value.substr(md)).c_str(), x + 2, y + 2, draw::default_font, draw::default_font_size, colors::white);
	} else {
		draw::String(value.c_str(), x + 2, y + 2, draw::default_font, draw::default_font_size, colors::white); // TODO recalc on update
	}
}

void CTextInput::SetCallback(TextInputCallbackFn_t callback) {
	m_pCallback = callback;
}

void CTextInput::PutChar(char ch) {
	SetValue(Value() + std::string(1, ch));
}

void CTextInput::OnKeyPress(int key, bool repeat) {
	if (key == CATKEY_BACKSPACE) {
		std::string val = Value();
		if (val.length() > 0) {
			SetValue(val.substr(0, val.length() - 1));
		}
		return;
	} else if (key == CATKEY_SPACE) {
		PutChar(' ');
		return;
	} else {
		char ch = 0;
		if (g_pGUI->pressed_state[CATKEY_LSHIFT] || g_pGUI->pressed_state[CATKEY_RSHIFT]) {
			ch = toupper((int)*input::key_names[key]);
		} else {
			ch = *input::key_names[key];
		}
		if (ch) {
			PutChar(ch);
		}
	}
}
