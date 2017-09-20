/*
 * CKeyInput.cpp
 *
 *  Created on: Feb 1, 2017
 *      Author: nullifiedcat
 */

#include "CKeyInput.h"

#include "../common.h"
#include "../sdk.h"

CKeyInput::CKeyInput(std::string name, IWidget* parent) : CBaseWidget(name, parent) {
	Props()->SetInt("value", 0);
	Props()->SetBool("capturing", false);
}

ButtonCode_t CKeyInput::Value() {
	return (ButtonCode_t)Props()->GetInt("value");
}

void CKeyInput::SetValue(int value) {
	Props()->SetInt("value", value);
}

void CKeyInput::Draw(int x, int y) {
	std::string key = "";
	int color = colors::white;
	if (Props()->GetBool("capturing")) {
		key = "< PRESS >";
		color = GUIColor();
	} else {
		if (!Value()) {
			if (!IsFocused()) {
				key = "< CLICK >";
			}
		} else {
			key = g_IInputSystem->ButtonCodeToString(Value());
		}
	}
	auto size = GetSize();
	auto ss = draw::GetStringLength(fonts::MENU, key);
	draw::String(fonts::MENU, x + (size.first - ss.first) / 2, y + (size.second - ss.second) / 2, color, 1, key);
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

bool CKeyInput::ConsumesKey(ButtonCode_t key) {
	return key != MOUSE_LEFT && Props()->GetBool("capturing");
}

void CKeyInput::OnKeyPress(ButtonCode_t key, bool repeat) {
	if (Props()->GetBool("capturing")) {
		if (key == KEY_ESCAPE) key = (ButtonCode_t)0;
		SetValue(key);
		if (m_pCallback) m_pCallback(this, key);
		Props()->SetBool("capturing", false);
	}
}
