/*
 * CCheckbox.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/drawing.hpp"
#include "GUI.h"

#include "CCheckbox.h"

CCheckbox::CCheckbox(std::string name, IWidget* parent, bool checked) : CBaseWidget(name, parent) {
	SetWidth(16);
	SetValue(checked);
}

void CCheckbox::SetWidth(int width) {
	Props()->SetInt("width", width);
	SetSize(width, width);
}

void CCheckbox::Draw(int x, int y) {
	auto size = GetSize();
	draw::Rect(x, y, size.first, size.second, GUIColor());
	if (Value()) {
		draw::RectFilled(x + 3, y + 3, size.first - 6, size.second - 6, GUIColor());
	}
}

void CCheckbox::OnMousePress() {
	SetValue(!Value());
	if (m_pCallback) {
		m_pCallback(this, Value());
	}
}

void CCheckbox::SetCallback(CheckboxCallbackFn_t callback) {
	m_pCallback = callback;
}
