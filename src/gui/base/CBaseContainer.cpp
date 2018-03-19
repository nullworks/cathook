/*
 * CBaseContainer.cpp
 *
 *  Created on: Jan 31, 2017
 *      Author: nullifiedcat
 */

#include <cstring> // strcmp()
#include <algorithm> // std::sort, min, max

#include "../../framework/drawing.hpp" // Draw stuff
#include "../../util/colors.hpp" // Draw stuff
#include "../../framework/input.hpp"
#include "../gui.hpp"

#include "CBaseContainer.hpp"

namespace gui { namespace base {

using namespace gui;

// Constructor & destructor
CBaseContainer::CBaseContainer(const char* _name) : CBaseWidget(_name) {}
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
	for (auto child : childrenZSort) {
		if (child->IsVisible()) {
			child->Draw();
		}
	}
	CBaseWidget::Draw();
}
void CBaseContainer::DrawBounds() {
	auto abs_pos = AbsolutePosition();
	draw::Rect(abs_pos.first, abs_pos.second, size.first, size.second, focus ? (focused_child==-1?colors::blue:colors::pink) : colors::gray);
	for (auto child : children) {
		if (child->IsVisible()) {
			child->DrawBounds();
		}
	}
	CBaseWidget::DrawBounds();
}

// User input functions
void CBaseContainer::OnMouseLeave() {
	HoverOn(-1);
	CBaseWidget::OnMouseLeave();
}
void CBaseContainer::OnMousePress() {
	//TODO: Fix mouse input
	//PressOn(ChildByPoint(input::mouse.first, input::mouse.second));
	for(auto child:children){
		child->OnMousePress();
	}
}
void CBaseContainer::OnMouseRelease() {
	for(auto child:children){
		child->OnMouseRelease();
	}
}
bool CBaseContainer::TryFocusGain() {
	if(can_focus_on_nothing) return true;
	//TODO: Move this into it's own function.
	for(int i=0;i<children.size();i++){
		if(children[i]->IsVisible()&&TryFocusOn(i)){
			return true;
		}
	}
	return false;
}
void CBaseContainer::OnFocusLose() {
	CBaseWidget::OnFocusLose();
	TryFocusOn(-1);
}
void CBaseContainer::OnKeyPress(int key) {
	if(focused_child!=-1&&children[focused_child]->ConsumesKey(key)){
		children[focused_child]->OnKeyPress(key);
	}else{
		//Flip to next child
		if(focused_child!=-1){
			if(key==nextkey.value){
				//TODO: Replace focused_child with the index of the selected child
				// so we don't have to spend time finding it here
				for(int i = focused_child+1;i<children.size();i++){
					if(children[i]->IsVisible()&&TryFocusOn(i)){
						//We found a new child
						break;
					}
				}
			}
			//Flip to previous child
			else if (key==prevkey.value){
				for(int i = ((focused_child==-1)?children.size():focused_child)-1;i>=0;i--){
					if(children[i]->IsVisible()&&TryFocusOn(i)){
						//We found a new child
						break;
					}
				}
			}
		}
		//Escape
		else if (key==backkey.value){
			if(can_focus_on_nothing){
				TryFocusOn(-1);
			}
		}
		else if (key==activatekey.value){
			if(focused_child==-1){
				for(int child=0;child<children.size();child++){
					if(children[child]->IsVisible()){
						if(TryFocusOn(child)) return;
					}
				}
				TryFocusOn(-1);
			}
		}
		
	}
}
void CBaseContainer::OnKeyRelease(int key) {
	//Tempted to just fully hand these key release events on, but that'll just slow things down.
	if (focused_child!=-1) children[focused_child]->OnKeyRelease(key);
}
bool CBaseContainer::ConsumesKey(int key) {
	//If our focused child consumes, we can't
	return (focused_child!=-1 && children[focused_child]->ConsumesKey(key))
	//If we can go to prev/next child, do so.
	|| (key==nextkey && focused_child!=children.size()-1)
	|| (key==prevkey && focused_child!=0)
	//If we can focus on nothing, and are currently focusing on something
	//(otherwise our parent container gets the backkey and will defocus our entire container)
	|| (key==backkey && focus && can_focus_on_nothing && focused_child)
	//Or if we can `activate`
	|| (key==activatekey && focus && focused_child==-1);
}

// Visiblity
void CBaseContainer::Hide() {
	CBaseWidget::Hide();
	if (hovered_child!=-1){
		children[hovered_child]->OnMouseLeave();
		hovered_child=-1;
	}
	if (focused_child!=-1){
		children[focused_child]->OnFocusLose();
		focused_child=-1;
	}
	if (pressed_child!=-1){
		children[pressed_child]->OnMouseRelease();
		pressed_child=-1;
	}
}

// Tooltips
const std::string& CBaseContainer::GetTooltip() {
	if (hovered_child!=-1) return children[hovered_child]->GetTooltip();
	return tooltip;
}

// Children
void CBaseContainer::AddChild(IWidget* child) {
	children.push_back(child);
	childrenZSort.push_back(child);
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
int CBaseContainer::ChildByPoint(int x, int y) { // Input a point in space to return a child within it
	int z=1337*69;
	int best=-1;
	for (int i = children.size() - 1; i >= 0; i--) {
		auto child = children[i];
		if (!child->visible) continue; // We dont care about always visible, we just want visible
		auto abs = child->AbsolutePosition();
		if (z>child->zindex && x >= abs.first && x <= abs.first + child->size.first &&
			y >= abs.second && y <= abs.second + child->size.second) {
			best=i;
			z=child->zindex;
		}
	}
	return best;
}

// Child related update functions
void CBaseContainer::SortByZIndex() {
	// Sort everything
	std::sort(childrenZSort.begin(), childrenZSort.end(), [](IWidget* a, IWidget* b) -> bool {
		return a->zindex < b->zindex;
	});
	// Make everything have a linear number... For delicious stacking
	for (int i = 0; i < childrenZSort.size(); i++)
		childrenZSort[i]->zindex = i;
}
void CBaseContainer::UpdateHovers() {
	//TODO: Hovering using input::mouse_event
	/*
	auto hovered = ChildByPoint(input::mouse.first, input::mouse.second);
	if (hovered != hovered_child)
		HoverOn(hovered);*/
}
void CBaseContainer::MoveChildren() {
	//TODO: Decipher MoveChildren, as written by nullcat back in 2017
	//minmax_size is being used as a minimum on this class
	// Used space
	std::pair<int, int> space = minmax_size;
	// Find a Bounding box around all of the absolutes
	for (auto c : children) {
		if (!c->IsVisible()) continue;
		// Check if not absolute
		if (c->position_mode != ABSOLUTE)
			continue;
		// If some our used space is less than the space taken by the widget, add it to used space.
		space.first = std::max(space.first, c->offset.first + c->size.first);
		space.second = std::max(space.second, c->offset.second + c->size.second);
	}
	
	// Organize our inlines
	int x = 0;
	auto bounds = input::GetBounds();
	int longest_column_height = 0;//Cannot exceed bounds.second
	int column_height = space.second;//init'd to abs' height
	int column=0;
	int columnFirstIDX=0;
	if(columnWidth.size()==0){
		columnWidth.push_back(defaultColumnWidth);
	}
	//for (auto c : children) {
	for(int idx=0;idx<children.size();idx++){
		auto c = children[idx];
		// Check if visible & inline
		if (!c->IsVisible()||c->position_mode != INLINE)
			continue;
		if(c->size.first>columnWidth[column]){
			columnWidth[column]=c->size.first;
			idx = columnFirstIDX-1;
			continue;//Increments idx
		}
		c->minmax_size.first=columnWidth[column];
		c->offset=std::make_pair(x, column_height);
		//Wrap to Next Column
		if(column_height+c->size.second>bounds.second){
			//Check for the longest column
			longest_column_height=std::max(column_height,longest_column_height);
			//Move right
			x+=columnWidth[column]+columnSpacing;
			//If this is the first time this column has existed
			if(++column>=columnWidth.size()){
				//Give it a default width
				columnWidth.push_back(defaultColumnWidth);
			}
			//Reset to top of column
			column_height=space.second;
			columnFirstIDX=idx;
		}
		//Add to column
		column_height+=c->size.second;
	}
	//Check for longest column
	longest_column_height = std::max(column_height,longest_column_height);
	//Move right
	x+=columnWidth[column]+columnSpacing;
	//Set own size
	size = std::make_pair(x,longest_column_height);
	//Ensure we don't go off the bottom of the screen
	auto abs_pos = AbsolutePosition();
	offset.second=std::min(offset.second,bounds.second+offset.second-abs_pos.second-longest_column_height);
}

// Child info related to the container
void CBaseContainer::HoverOn(int child) {
	if (hovered_child!=-1) children[hovered_child]->OnMouseLeave();
	if (hover_is_focus&&focus) {
		TryFocusOn(child);
	}
	if (child!=-1) children[child]->OnMouseEnter();
	hovered_child = child;
}
bool CBaseContainer::TryFocusOn(int child) {
	if (focused_child != child) {
		if (child!=-1){
			if(children[child]->TryFocusGain()){
				if (focused_child!=-1) children[focused_child]->OnFocusLose();
				focused_child = child;
				return true;
			}
		}else{
			if(can_focus_on_nothing||!focus){
				if (focused_child!=-1) children[focused_child]->OnFocusLose();
				focused_child = -1;
				//You can focus on nothing if you want to
				//(as proved in elementary school)
				return true;
			}
		}
		return false;
	}else{
		//We can focus something thats already focused..
		//By doing nothin'!
		return true;
	}
}
void CBaseContainer::PressOn(int child) {
	pressed_child = child;
	children[child]->press=true;
	if (focus) TryFocusOn(child);
}

}}
