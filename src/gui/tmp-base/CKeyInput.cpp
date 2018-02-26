/*
 * CKeyInput.cpp
 *
 *  Created on: Feb 1, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/drawing.hpp"
#include "GUI.h"

#include "CKeyInput.h"

CKeyInput::CKeyInput(std::string name, IWidget* parent) : CBaseWidget(name, parent) {
	Props()->SetInt("value", 0);
	Props()->SetBool("capturing", false);
}

int CKeyInput::Value() {
	return Props()->GetInt("value");
}

void CKeyInput::SetValue(int value) {
	Props()->SetInt("value", value);
}

void CKeyInput::Draw(int x, int y) {
	std::string key = "";
	auto color = colors::white;
	if (Props()->GetBool("capturing")) {
		key = "< PRESS >";
		color = GUIColor();
	} else {
		if (!Value()) {
			if (!IsFocused()) {
				key = "< CLICK >";
			}
		} else {
			key = input::key_names[Value()];
		}
	}
	auto size = GetSize();
	auto ss = draw::GetStringLength(key.c_str(), draw::default_font, draw::default_font_size);
	draw::String(key.c_str(), x + (size.first - ss.first) / 2, y + (size.second - ss.second) / 2, draw::default_font, draw::default_font_size, color);
}

void CKeyInput::SetCallback(KeyInputCallbackFn_t callback) {
	m_pCallback = callback;
}

void CKeyInput::OnMousePress() {
	if (!Props()->GetBool("capturing"))
		Props()->SetBool("capturing", true);
}

void CKeyInput::OnFocusLose() {
	Props()->SetBool("capturing", false);
}

bool CKeyInput::ConsumesKey(int key) {
	return key != CATKEY_MOUSE_1 && Props()->GetBool("capturing");
}

void CKeyInput::OnKeyPress(int key, bool repeat) {
	if (Props()->GetBool("capturing")) {
		if (key == CATKEY_ESCAPE) key = CATKEY_NONE;
		SetValue(key);
		if (m_pCallback) m_pCallback(this, key);
		Props()->SetBool("capturing", false);
	}
}
