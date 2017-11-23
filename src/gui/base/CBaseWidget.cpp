
/*
 *
 *	This is a base widget for things like sliders, checkboxes, dropdowns, etc.
 *
 *
 */

#include "CBaseWidget.hpp"

namespace gui { namespace base {

CBaseWidget::CBaseWidget(std::string _name, IWidget* _parent) {
	parent = _parent;
	name = _name;
	SetPositionMode(INLINE);
	Show();
	SetMaxSize(-1, -1);
}

void CBaseWidget::Update() {
	// Shows our tooltip
	if (IsHovered() && IsVisible() && tooltip != "") {
		auto pRoot = GetRootParent()
		pRoot->tooltip = tooltip;
	}
}

void CBaseWidget::DrawBounds() {
	if (!bounds_color) {
		bounds_color = CatVector4(rand() % 255, rand() % 255, rand() % 255, 255);
	}
	auto pRoot = AbsolutePosition();
	draw::RectFilled(pRoot.first, pRoot.second, size.first, size.second, colors::Transparent(bounds_color, 0.25f));
	draw::Rect(pRoot.first, pRoot.second, size.first, size.second, bounds_color);
}

std::pair<int, int> CBaseWidget::AbsolutePosition() {
	auto result = GetOffset();
	auto pParent = GetParent();
	while (pParent) {
		auto poffset = pParent->GetOffset();
		result.first += poffset.first;
		result.second += poffset.second;
		pParent = pParent->GetParent();
	}
	return result;
}

IWidget* GetRootParent() {
	auto pParent = GetParent();
	while (pParent)
		pParent = pParent->GetParent();
	return pParent;
}

}}
