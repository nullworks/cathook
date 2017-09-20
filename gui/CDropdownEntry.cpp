/*
 * CDropdownEntry.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#include "CDropdownEntry.h"

#include "../common.h"
#include "../sdk.h"

CDropdownEntry::CDropdownEntry(std::string name, CDropdownList* parent, std::string text, int value) : CBaseButton(name, parent, text) {
	Props()->SetInt("value", value);
	SetCallback([this](CBaseButton*) -> void {
		CDropdownList* parent = dynamic_cast<CDropdownList*>(GetParent());
		if (!parent) return;
		parent->SetValue(Props()->GetInt("value"));
	});
}

void CDropdownEntry::Draw(int x, int y) {
	auto ssize = draw::GetStringLength(fonts::MENU, GetText());
	auto size = GetSize();
	draw::String(fonts::MENU, x + (size.first - ssize.first) / 2, y + (size.second - ssize.second) / 2, GUIColor(), 1, GetText());
}

CDropdownEntry::~CDropdownEntry() {

}
