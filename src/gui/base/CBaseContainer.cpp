/*
 * CBaseContainer.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#include "../../../framework/inputmgr.hpp"

#include "CBaseContainer.h"

// Constructor & destructor
CBaseContainer::CBaseContainer(std::string _name, IWidget* _parent) : CBaseWidget(_name, _parent) {}
CBaseContainer::~CBaseContainer() { for (auto child : children) delete child; }

// General functions
void CBaseContainer::Update() {
	SortByZIndex();
	MoveChildren();
	UpdateHovers();
	for (auto child : m_children) {
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
	if (GetPressedChild()) GetPressedChild()->OnMouseRelease();
}
void CBaseContainer::OnFocusLose() {
	FocusOn(0);
	CBaseWidget::OnFocusLose();
}
void CBaseContainer::OnKeyPress(int key, bool repeat) {
	if (GetFocusedChild()) GetFocusedChild()->OnKeyPress(key, repeat);
}
void CBaseContainer::OnKeyRelease(int key) {
	if (GetFocusedChild()) GetFocusedChild()->OnKeyRelease(key);
}
bool CBaseContainer::ConsumesKey(int key) { 
	if (GetFocusedChild()) return GetFocusedChild()->ConsumesKey(key); 
	return false; 
}

// Visiblity
void CBaseContainer::Hide() {
	CBaseWidget::Hide();
	if (GetHoveredChild()) GetHoveredChild()->OnMouseLeave();
	if (GetFocusedChild()) GetFocusedChild()->OnFocusLose();
	if (GetPressedChild()) GetPressedChild()->OnMouseRelease();
}

// Children
void CBaseContainer::AddChild(IWidget* child) {
	children.push_back(child);
	child->SetParent(this);
}

// Children util
IWidget* CBaseContainer::ChildByIndex(int idx) { 
	if (idx < 0 || idx >= ChildCount()) return nullptr; 
	return children.at(idx); 
}
IWidget* CBaseContainer::ChildByName(std::string name) {
	for (auto child : children) {
		if (!child->GetName().compare(name)) {
			return child;
		}
	}
	return nullptr;
}
IWidget* CBaseContainer::ChildByPoint(int x, int y) { // Input a point in space to return a child within it
	for (int i = ChildCount() - 1; i >= 0; i--) {
		auto child = ChildByIndex(i);
		if (!child->IsVisible()) continue;
		auto abs = child->AbsolutePosition();
		auto cs = child->GetSize();
		if (x >= abs.first && x <= abs.first + cs.first &&
			y >= abs.second && y <= abs.second + cs.second) {
			return child;
		}
	}
	return nullptr;
}
int CBaseContainer::ChildCount() {
	return children.size();
}

// Child related update functions
void CBaseContainer::SortByZIndex() { 
	std::sort(children.begin(), children.end(), [](IWidget* a, IWidget* b) -> bool { 
		return a->GetZIndex() < b->GetZIndex(); 
	}); 
}
void CBaseContainer::UpdateHovers() {
	auto hovered = ChildByPoint(input::mouse.first, input::mouse.second);
	if (hovered != GetHoveredChild()) 
		HoverOn(hovered); 
}
void CBaseContainer::MoveChildren() { 
	return; 
}

// Child info related to the container
IWidget* CBaseContainer::GetFocusedChild() { return focused_child; }
IWidget* CBaseContainer::GetHoveredChild() { return hovered_child; }
IWidget* CBaseContainer::GetPressedChild() { return pressed_child; }
void CBaseContainer::HoverOn(IWidget* child) {
	if (GetHoveredChild()) GetHoveredChild()->OnMouseLeave();
	if (child) child->OnMouseEnter();
	hovered_child = child;
}
void CBaseContainer::FocusOn(IWidget* child) {
	if (GetFocusedChild() != child) {
		if (GetFocusedChild()) GetFocusedChild()->OnFocusLose();
		if (child) child->OnFocusGain();
		focused_child = child;
	}
}
void CBaseContainer::PressOn(IWidget* child) {
	pressed_child = child;
	if (child) {
		child->OnMousePress();
		if (child->DoesStealFocus())
			FocusOn(child);
	} else FocusOn(0);
}


