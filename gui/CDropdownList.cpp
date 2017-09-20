/*
 * CDropdownList.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#include "CDropdownList.h"
#include "CDropdownEntry.h"
#include "CDropdown.h"

#include "../common.h"
#include "../sdk.h"

CDropdownList::CDropdownList(std::string name, CDropdown* menu, int offset) : CBaseContainer(name, nullptr) {
	m_pMenu = menu;
	Hide();
	SetZIndex(5);
}

CDropdownList::~CDropdownList() {
	for (auto entry : m_entries) {
		delete entry;
	}
}

void CDropdownList::AddEntry(std::string name) {
	CDropdownEntry* entry = new CDropdownEntry("entry", this, name, m_entries.size());
	auto size = GetSize();
	entry->SetSize(size.first, 18);
	AddChild(entry);
	m_entries.push_back(entry);
	SetSize(size.first, m_entries.size() * 18);
}

void CDropdownList::SetValue(int value) {
	m_pMenu->SetValueInternal(value);
	Hide();
}

void CDropdownList::Draw(int x, int y) {
	auto size = GetSize();
	draw::DrawRect(x, y, size.first, size.second, colors::Transparent(colors::black, 0.85));
	draw::OutlineRect(x, y, size.first, size.second, GUIColor());
	CBaseContainer::Draw(x, y);
}

void CDropdownList::MoveChildren() {
	for (int i = 0; i < ChildCount(); i++) {
		auto child = ChildByIndex(i);
		child->SetOffset(0, i * 18);
	}
}
