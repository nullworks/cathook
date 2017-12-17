/*
 * CBaseContainer.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#include <cstring> // strcmp()

#include "../../framework/input.hpp"

#include "CBaseContainer.hpp"

namespace gui { namespace base {

// Constructor & destructor
CBaseContainer::CBaseContainer(const char* _name, IWidget* _parent) : CBaseWidget(_name, _parent) {}
CBaseContainer::~CBaseContainer() { for (auto child : children) delete child; }

// General functions
void CBaseContainer::Update() {
	SortByZIndex();
	MoveChildren();
	UpdateHovers();
	for (auto child : children) {
		child->Update();
	}
	CBaseWidget::Update();
}
void CBaseContainer::Draw() {
	for (auto child : children) {
		if (child->IsVisible()) {
			child->Draw();
		}
	}
}
void CBaseContainer::DrawBounds() {
	for (auto child : children) {
		if (child->IsVisible()) {
			child->DrawBounds();
		}
	}
	CBaseWidget::DrawBounds();
}

// User input functions
void CBaseContainer::OnMouseLeave() {
	HoverOn(0);
	CBaseWidget::OnMouseLeave();
}
void CBaseContainer::OnMousePress() {
	PressOn(ChildByPoint(input::mouse.first, input::mouse.second));
}
void CBaseContainer::OnMouseRelease() {
	if (pressed_child) pressed_child->OnMouseRelease();
}
void CBaseContainer::OnFocusLose() {
	FocusOn(0);
	CBaseWidget::OnFocusLose();
}
void CBaseContainer::OnKeyPress(int key, bool repeat) {
	if (focused_child) focused_child->OnKeyPress(key, repeat);
}
void CBaseContainer::OnKeyRelease(int key) {
	if (focused_child) focused_child->OnKeyRelease(key);
}
bool CBaseContainer::ConsumesKey(int key) {
	if (focused_child) return focused_child->ConsumesKey(key);
	return false;
}

// Visiblity
void CBaseContainer::Hide() {
	CBaseWidget::Hide();
	if (hovered_child) hovered_child->OnMouseLeave();
	if (focused_child) focused_child->OnFocusLose();
	if (pressed_child) pressed_child->OnMouseRelease();
}

// Tooltips
const char* CBaseContainer::GetTooltip() {
	if (hovered_child) return hovered_child->GetTooltip();
	return nullptr;
}

// Children
void CBaseContainer::AddChild(IWidget* child) {
	children.push_back(child);
	child->parent = this;
}

// Children util
IWidget* CBaseContainer::ChildByIndex(int idx) {
	if (idx < 0 || idx >= children.size()) return nullptr;
	return children.at(idx);
}
IWidget* CBaseContainer::ChildByName(const char* name) {
	for (auto child : children) {
		if (!strcmp(child->name, name)) {
			return child;
		}
	}
	return nullptr;
}
IWidget* CBaseContainer::ChildByPoint(int x, int y) { // Input a point in space to return a child within it
	for (int i = children.size() - 1; i >= 0; i--) {
		auto child = ChildByIndex(i);
		if (!child->IsVisible()) continue;
		auto abs = child->AbsolutePosition();
		auto cs = child->size;
		if (x >= abs.first && x <= abs.first + cs.first &&
			y >= abs.second && y <= abs.second + cs.second) {
			return child;
		}
	}
	return nullptr;
}

// Child related update functions
void CBaseContainer::SortByZIndex() {
	std::sort(children.begin(), children.end(), [](IWidget* a, IWidget* b) -> bool {
		return a->zindex < b->zindex;
	});
}
void CBaseContainer::UpdateHovers() {
	auto hovered = ChildByPoint(input::mouse.first, input::mouse.second);
	if (hovered != hovered_child)
		HoverOn(hovered);
}
void CBaseContainer::MoveChildren() {
	// Used space
	int mx = 0, my = 0, mox = size.first, moy = size.second;
	// Get our absolutes down
	/*for (auto c : children) {
		// Check if not absolute
		if (c->position_mode != ABSOLUTE)
			return;
		// Add the amount of space it takes to our used amount
		std::pair<int, int> space_taken;
		space_taken.first = c->offset.second + c->size.second

			space_taken =  + 2
			if

			break;
		case ABSOLUTE_LEFT:
			mx -= c->offset.first + c->size.second + 2;
			break;
		}
	}
	// Organize our inlines
	for (auto c : children) {
		if (!c->IsVisible()) continue;
		auto off = c->GetOffset();
		auto size = c->GetSize();

		c->SetOffset(2, my);
		if (size.first > mx) mx = size.first;
		size.first += off.first;
		size.second += off.second;
		}
	}*/
}

// Child info related to the container
void CBaseContainer::HoverOn(IWidget* child) {
	if (hovered_child) hovered_child->OnMouseLeave();
	if (child) child->OnMouseEnter();
	hovered_child = child;
}
void CBaseContainer::FocusOn(IWidget* child) {
	if (focused_child != child) {
		if (focused_child) focused_child->OnFocusLose();
		if (child) child->OnFocusGain();
		focused_child = child;
	}
}
void CBaseContainer::PressOn(IWidget* child) {
	pressed_child = child;
	if (child) {
		FocusOn(child);
		child->OnMousePress();
	} else FocusOn(0);
}

}}
