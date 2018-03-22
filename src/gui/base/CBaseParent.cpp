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
#include "../../util/logging.hpp"

#include "CBaseParent.hpp"

namespace gui { namespace base {

using namespace gui;

CBaseParent::CBaseParent(std::string name, std::string tooltip):CBaseWidget(name,tooltip){}
// Destructor
CBaseParent::~CBaseParent() { for (auto child : children) delete child; }

// Graphics
void CBaseParent::Draw() {
	//g_CatLogging.log("Drawing %s's children",name.c_str());
	for (auto child : children) {
		if (child->visible) {
			child->Draw();
		}
	}
	CBaseWidget::Draw();
}

void CBaseParent::UpdatePositioning(){
	CBaseWidget::UpdatePositioning();
	for( auto c : children ){
		c->UpdatePositioning();
	}
}

// User input functions
bool CBaseParent::OnMouse(std::pair<int,int> mouse_pos, bool hover_taken) {
	hover=false;
	for(int i = 0;i<children.size();i++){
		//the !hover_taken is a safety precaution
		if(children[i]->OnMouse(mouse_pos, hover_taken||hover)&&!hover_taken){
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
bool CBaseParent::TryFocusOn(int child) {
	if(focused_child!=child&&(child==-1||children[child]->TryFocusGain())){
		if(focused_child!=-1){
			children[focused_child]->OnFocusLose();
		}
		focused_child=child;
		return true;
	}
	//TODO: Should trying to focus on the focus return true or false?
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
		if(focused_child!=-1&&key==nextkey.value){
			g_CatLogging.log("%s's next child, from %d.. (%d children)",name.c_str(),focused_child, children.size());
			for(int i = focused_child;i<children.size();i++){
				g_CatLogging.log("Trying child %d..",i);
				if(i==-1){
					g_CatLogging.log("Bad! BAD! BAAAD!");
					continue;
				}
				if(children[i]->visible&&TryFocusOn(i)){
					g_CatLogging.log("My new daughter, %d..",i);
					//We found a new child
					break;
				}
			}
		}
		//Flip to previous child
		else if (focused_child!=-1&&key==prevkey.value){
			g_CatLogging.log("%s's previous child, from %d of %d children..",name.c_str(),focused_child, children.size());
			for(int i = focused_child;i>-1;i--){
				g_CatLogging.log("Trying child %d..",i);
				if(i==children.size()){
					g_CatLogging.log("Worse! WORSE! WOOOOOORSE!!");
					continue;
				}
				if(children[i]->visible&&TryFocusOn(i)){
					g_CatLogging.log("My new son, %d..", i);
					//We found a new child
					break;
				}
			}
		}
		//Escape
		else if (key==backkey.value){
			if(can_focus_on_nothing){
				g_CatLogging.log("%s's abandoning %d!", name.c_str(),focused_child);
				TryFocusOn(-1);
			}
		}
		else if (key==activatekey.value){
			if(focused_child==-1){
			g_CatLogging.log("%s's finding their first child..", name.c_str());
				for(int child=0;child<children.size();child++){
					if(children[child]->visible){
						if(TryFocusOn(child)) return;
					}
				}
				TryFocusOn(-1);
			}
		}
	}
    CBaseWidget::OnKeyPress(key,repeat);
}

void CBaseParent::OnKeyRelease(int key) {
	//Tempted to just fully hand these key release events on, but that'll just slow things down.
	if (focused_child!=-1) children[focused_child]->OnKeyRelease(key);
}
bool CBaseParent::ConsumesKey(int key) {
	//If our focused child consumes, we can't
	return (focused_child!=-1 && children[focused_child]->ConsumesKey(key))
	//If we can go to prev/next child, do so.
	|| (key==nextkey && focused_child!=(children.size()-1))
	|| (key==prevkey && focused_child!=0)
	//If we can focus on nothing, and are currently focusing on something
	//(otherwise our parent container gets the backkey and will defocus our entire container)
	|| (key==backkey && can_focus_on_nothing && focused_child != -1)
	//Or if we can `activate`
	|| (key==activatekey && focused_child==-1);
}

bool CBaseParent::OnBounds(std::pair<int,int> bounds){
	for(auto c : children){
		c->OnBounds(bounds);
	}
}

// Tooltips
const std::string& CBaseParent::GetTooltip() {
	if (hovered_child!=-1) return children[hovered_child]->GetTooltip();
	return tooltip;
}

// Children
void CBaseParent::AddChild(CBaseWidget* child) {
	children.push_back(child);
	child->parent = this;
	UpdatePositioning();
}

// Children util
CBaseWidget* CBaseParent::ChildByIndex(int idx) {
	if (idx < 0 || idx >= children.size()) return nullptr;
	return children.at(idx);
}
CBaseWidget* CBaseParent::ChildByName(const char* name) {
	for (auto child : children) {
		if (child->name == name) {
			return child;
		}
	}
	return nullptr;
}
}}
