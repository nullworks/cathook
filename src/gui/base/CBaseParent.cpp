/*
 * 
 * Base class for widgets with children.
 * Examples include: Root, Window, and Inliner (prev known as Container)
 * 
 */

#include <cstring> // strcmp()
#include <algorithm> // std::sort, min, max

#include "../../framework/drawing.hpp" // Draw stuff
#include "../../util/colors.hpp" // Draw stuff
#include "../../framework/input.hpp"
#include "../gui.hpp"

#include "CBaseParent.hpp"

namespace gui { namespace base {

using namespace gui;

CBaseParent::CBaseParent(std::string name, std::string tooltip):CBaseWidget(name,tooltip){}
// Destructor
CBaseParent::~CBaseParent() { for (auto child : children) delete child; }

// Graphics
void CBaseParent::Draw() {
	for (auto child : children) {
		if (child->visible) {
			child->Draw();
		}
	}
	CBaseWidget::Draw();
}

// User input functions
bool CBaseParent::OnMouseMove(std::pair<int,int> mouse_pos, bool hover_taken) {
	hover=false;
	for(int i = 0;i<children.size();i++){
		//the !hover_taken is a safety precaution
		if(children[i]->OnMouseMove(mouse_pos, hover_taken||hover)&&!hover_taken){
			hover=true;
			hovered_child = i;
		}
	}
	hover|=(!hover_taken)
		&&mouse_pos.first>global_pos.first
		&&mouse_pos.second>global_pos.second
		&&mouse_pos.first<offset.first
		&&mouse_pos.second<offset.second;
	return hover;
}
bool CBaseParent::TryFocusGain() {
	if(can_focus_on_nothing) return true;
	for(int i = focused_child+1;i<children.size();i++){
		auto child = children[i];
		if(child->visible&&TryFocusOn(i)){
			return true;
		}
	}
	return false;
}
void CBaseParent::OnFocusLose() {
	CBaseWidget::OnFocusLose();
	TryFocusOn(-1);
}
void CBaseParent::OnKeyPress(int key, bool repeat) {
	if(focused_child!=-1&&children[focused_child]->ConsumesKey(key)){
		children[focused_child]->OnKeyPress(key, repeat);
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
void CBaseParent::OnKeyRelease(int key) {
	//Tempted to just fully hand these key release events on, but that'll just slow things down.
	if (focused_child!=-1) children[focused_child]->OnKeyRelease(key);
}
bool CBaseParent::ConsumesKey(int key) {
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

// Tooltips
const std::string& CBaseParent::GetTooltip() {
	if (hovered_child!=-1) return children[hovered_child]->GetTooltip();
	return tooltip;
}

// Children
void CBaseParent::AddChild(IWidget* child) {
	children.push_back(child);
	child->parent = this;
	UpdatePositioning();
}

// Children util
IWidget* CBaseParent::ChildByIndex(int idx) {
	if (idx < 0 || idx >= children.size()) return nullptr;
	return children.at(idx);
}
IWidget* CBaseParent::ChildByName(const char* name) {
	for (auto child : children) {
		if (child->name == name) {
			return child;
		}
	}
	return nullptr;
}
}}
