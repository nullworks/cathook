
/*
 *
 *	This is for most objects on the gui.
 *	Most things use this
 *
 */

#include "CBaseWindow.h"

// General functions
void CBaseWindow::Draw() {
	auto abs = AbsolutePosition();
	auto size = GetSize();
	draw::RectFilled(abs.first, abs.second, size.first, size.second, colors::Transparent(colors::black, 0.9));
	draw::Rect(abs.first, abs.second, size.first, size.second, colors::pink);
	CBaseContainer::Draw();
}

// User Input
void CBaseWindow::OnFocusGain() {
	SetZIndex(GetZIndex() + 1);
	CBaseContainer::OnFocusGain();
}
void CBaseWindow::OnFocusLose() {
	SetZIndex(GetZIndex() - 1);
	CBaseContainer::OnFocusLose();
}

// Child related util
void CBaseWindow::MoveChildren() { // TODO, Reverse!!!
	int mx = 0, my = 2;// Book keepers
	for (auto c : children) {
		if (!c->IsVisible()) continue;
		auto off = c->GetOffset();
		auto size = c->GetSize();
		if (c->GetPositionMode() != ABSOLUTE && c->GetPositionMode() != FLOATING)
			c->SetOffset(2, my);
		else {
			size.first += off.first;
			size.second += off.second;
		}
		if (c->GetPositionMode() != FLOATING && c->GetPositionMode() != ABSOLUTE)
			if (size.first > mx) mx = size.first;
		if (c->GetPositionMode() != FLOATING)
			my += (size.second + 2);
	}
	if (GetParent()) {
		SetSize(mx + 4, my + 2);
	}
}




