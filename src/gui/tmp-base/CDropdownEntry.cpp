/*
 * CDropdownEntry.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#include "../../framework/drawing.hpp"
#include "GUI.h"

#include "CDropdownEntry.h"

CDropdownEntry::CDropdownEntry(std::string name, CDropdownList* parent, std::string text, int value) : CBaseButton(name, parent, text) {
	Props()->SetInt("value", value);
	SetCallback([this](CBaseButton*) -> void {
		CDropdownList* parent = dynamic_cast<CDropdownList*>(GetParent());
		if (!parent) return;
		parent->SetValue(Props()->GetInt("value"));
	});
}

void CDropdownEntry::Draw(int x, int y) {
	auto ssize = draw::GetStringLength(GetText().c_str(), draw::default_font, draw::default_font_size);
	auto size = GetSize();
	draw::String(GetText().c_str(), x + (size.first - ssize.first) / 2, y + (size.second - ssize.second) / 2, draw::default_font, draw::default_font_size, GUIColor());
}

CDropdownEntry::~CDropdownEntry() {

}
