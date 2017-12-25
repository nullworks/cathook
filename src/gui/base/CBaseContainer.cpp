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
		if (child->name == name) {
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
	std::pair<int, int> space = std::make_pair(-1, -1);
	// Get our absolutes down
	for (auto c : children) {
		if (!c->IsVisible()) continue;
		// Check if not absolute
		if (c->position_mode != ABSOLUTE)
			continue;
		// Add the amount of space it takes to our used amount
		std::pair<int, int> space_taken;
		space_taken.first = c->offset.first + c->size.first;
		space_taken.second = c->offset.second + c->size.second;

		// If some our used space is less than the space taken by the widget, add it to used space.
		if (space.first < space_taken.first)
			space.first = space_taken.first;
		if (space.second < space_taken.second)
			space.second = space_taken.second;
	}

	// Get our size for the container and set it
	std::pair<int, int> tmp_max = (max_size == std::make_pair(-1, -1)) ? space : max_size;
	size = tmp_max;

	// Organize our inlines
	std::pair<int, int> cur_pos = std::make_pair(2, 2);
	int lane_height = 0;
	for (auto c : children) {
		if (!c->IsVisible()) continue;

		// Check if inline
		if (c->position_mode != INLINE)
			continue;

		// Get whether widget width would overlap max size, make widget go in new lane if true
		if (cur_pos.first + c->size.first + 2 > tmp_max.first) {

			// Put the widget on a new line
			cur_pos = std::make_pair(2, cur_pos.second + lane_height + 2);
			lane_height = 0;
		}

		// If our widget height is more than the lane size, add to it
		if (c->size.second > lane_height)
			lane_height = c->size.second;

		// Set the inline widgets position
		c->offset = cur_pos;

		// Add to the length used
		cur_pos.first += c->size.first + 2;
	}
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
