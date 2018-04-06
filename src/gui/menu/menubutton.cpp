#include "menubutton.hpp"
#include "../../framework/input.hpp"
#include "../../framework/drawing.hpp"
#include "../../util/logging.hpp"
#include "../gui.hpp"

/*
Child is open, displayed.

*/

namespace gui { namespace menu {
CMenuButton::CMenuButton(std::string str):CBaseParent(str){
}
void CMenuButton::Draw(){
    auto color = focus ? (focused_child == -1 ? colors::pink : colors::RainbowCurrent()) : (hover ? colors::lightgray : colors::gray);
    draw::RectFilled(global_pos.first, global_pos.second, size.first, size.second, color.Transparent(0.3));
    draw::Rect(global_pos.first, global_pos.second, size.first, size.second, color);
    auto textsize=draw::GetStringLength(name.c_str(), draw::default_font.value, draw::default_font_size.value);
    draw::String(name.c_str(), global_pos.first + (size.first - textsize.first) / 2, global_pos.second + (size.second - textsize.second) / 2, draw::default_font.value, draw::default_font_size.value, colors::white);
    CBaseParent::Draw();
}
void CMenuButton::UpdatePositioning(){
    int min_width = size.first;
    size = draw::GetStringLength(name.c_str(), draw::default_font.value, draw::default_font_size.value);
    size.first = std::max(size.first + padding.first * 2, min_width);
    size.second += padding.second * 2;
    //g_CatLogging.log("New size for menubutton '%s': %d %d", name.c_str(), size.first, size.second);
    CBaseParent::UpdatePositioning();
}
bool CMenuButton::TryFocusGain() {
    focused_child=-1;
    for(auto c : children)
        c->visible = true;
    UpdatePositioning();
	return CBaseParent::TryFocusGain();
}
void CMenuButton::OnFocusLose() {
    for(auto child : children)
      child->visible=false;
    TryFocusOn(-1);
	CBaseParent::OnFocusLose();
}
void CMenuButton::OnKeyPress(int key) {
  if (key == backkey.value){
    TryFocusOn(-1);
  } else if(focused_child != -1 && children[focused_child]->ConsumesKey(key)){
		children[focused_child]->OnKeyPress(key);
	} else if (focused_child == -1 && key == activatekey.value && children.size() > 0 &&focus){
    //&&focus is probs unnecessary
    TryFocusOn(0);
  }
  CBaseParent::OnKeyPress(key);
}
bool CMenuButton::ConsumesKey(int key) {
  //We need a child to consume anything.
	//If our child consumes, we can't. If we have a child who doesn't consume we can defocus them
	if (focused_child != -1 && (key == backkey || children[focused_child]->ConsumesKey(key)))
    return true;
  return key == activatekey && focus && focused_child == -1 && children.size() > 0;
}
void CMenuButton::AddChild(base::CBaseWidget* child) {
	children.push_back(child);
	child->parent = this;
  child->visible = false;
}
}}
