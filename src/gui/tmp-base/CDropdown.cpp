/*
 * CDropdown.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/drawing.hpp"
#include "CDropdownList.h"
#include "RootWindow.h"
#include "GUI.h"

#include "CDropdown.h"

CDropdown::CDropdown(std::string name, IWidget* parent) : CBaseButton(name, parent) {
	list = new CDropdownList(name + "_list", this);
	g_pGUI->GetRootWindow()->AddChild(list);
	SetSize(80, 18);
	list->SetSize(80, 0);
	CBaseButton::SetCallback([this](CBaseButton*) -> void {
		ShowList();
	});
}

CDropdown::~CDropdown() {
	delete list;
}

void CDropdown::SetCallback(DropdownCallbackFn_t callback) {
	m_pDropdownCallback = callback;
}

void CDropdown::AddValue(std::string string) {
	list->AddEntry(string);
	m_values.push_back(string);
}

std::string CDropdown::ValueName(int idx) {
	if (idx < 0 || idx >= m_values.size()) return "unknown";
	return m_values.at(idx);
}

void CDropdown::Draw(int x, int y) {
	auto size = GetSize();
	auto ssize = draw::GetStringLength(ValueName(Value() - Props()->GetInt("offset")).c_str(), draw::default_font, draw::default_font_size);
	draw::RectFilled(x, y, size.first, size.second, colors::Transparent(colors::black));
	draw::Rect(x, y, size.first, size.second, GUIColor());
	draw::String(ValueName(Value() - Props()->GetInt("offset")).c_str(), x + (size.first - ssize.first) / 2, y + (size.second - ssize.second) / 2, draw::default_font, draw::default_font_size, GUIColor());
	auto asize = draw::GetStringLength(">", draw::default_font, draw::default_font_size);
	draw::String(">", x + size.first - asize.first - 2, y + (size.second - asize.second) / 2, draw::default_font, draw::default_font_size, GUIColor());
}

void CDropdown::OnFocusLose() {
	list->Hide();
}

void CDropdown::SetValueInternal(int value) {
	Props()->SetInt("value", value + Props()->GetInt("offset"));
	if (m_pDropdownCallback)
		m_pDropdownCallback(this, value + Props()->GetInt("offset"));
}

void CDropdown::SetValue(int value) {
	Props()->SetInt("value", value);
}

void CDropdown::ShowList() {
	auto pos = AbsolutePosition();
	auto size = GetSize();
	list->SetOffset(pos.first + size.first + 3, pos.second);
	list->Show();
}

int CDropdown::Value() {
	return Props()->GetInt("value");
}

int CDropdown::ValueCount() {
	return m_values.size();
}
