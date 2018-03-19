#include "menubutton.hpp"
#include "../../framework/input.hpp"
#include "../../framework/drawing.hpp"
#include "../gui.hpp"

/*
Child is open, displayed.

*/

namespace gui { namespace menu {
    CMenuButton::CMenuButton(const char * str):CBaseWidget(str){
    }
    void CMenuButton::Draw(){
        auto abs = AbsolutePosition();
        auto color=focus?(child_focused?colors::pink:colors::RainbowCurrent()):colors::gray;
    	draw::RectFilled(abs.first, abs.second, size.first, size.second, colors::Transparent(color, 0.3));
    	draw::Rect(abs.first, abs.second, size.first, size.second, color);
        auto textsize=draw::GetStringLength(name.c_str(),draw::default_font.value,draw::default_font_size.value);
        draw::String(name.c_str(),abs.first+(size.first-textsize.first)/2,abs.second+(size.second-textsize.second)/2,draw::default_font.value,draw::default_font_size.value,colors::white);
	    CBaseWidget::Draw();
        if(child&&child->IsVisible()) child->Draw();
    }
    void CMenuButton::Update(){
        size=draw::GetStringLength(name.c_str(),draw::default_font.value,draw::default_font_size.value);
        size.first=std::max(size.first+padding.first*2,minmax_size.first);
        size.second+=padding.second*2;
        if(child) child->Update();
    }
bool CMenuButton::TryFocusGain() {
    child_focused=false;
    if(child) child->Show();
	return CBaseWidget::TryFocusGain();
}
void CMenuButton::OnFocusLose() {
    if(child&&child_focused) child->OnFocusLose();
    if(child) child->Hide();
	child_focused=false;
	CBaseWidget::OnFocusLose();
}
void CMenuButton::OnKeyPress(int key) {
    if(child){
        if (child_focused){
            if(!child->ConsumesKey(key)){
                if (key==backkey.value){
                    if(child_focused) child->OnFocusLose();
                    child_focused=false;
                }
            }
            child->OnKeyPress(key);
        }else if (key==activatekey.value&&focus){
            //Only runs if child_focused==false
            child_focused|=child->TryFocusGain();
        }
    }
}
void CMenuButton::OnKeyRelease(int key) {
	//Tempted to just fully hand these key release events on, but that'll just slow things down.
	if (child && child_focused) child->OnKeyRelease(key);
}
bool CMenuButton::ConsumesKey(int key) {
    //We need a child to consume anything.
	//If our child consumes, we can't. If we have a child who doesn't consume we can defocus them
	return child &&(child_focused && (child->ConsumesKey(key) || key==backkey)
	//If we aren't focusing on our child we can focus on our child.
	|| (child && (!child_focused) && focus && key==activatekey));
}


    // Visiblity
    void CMenuButton::Hide() {
        CBaseWidget::Hide();
        if(child) child->Hide();
    }
    // Children
    void CMenuButton::SetChild(IWidget* child) {
        this->child=child;
    	child->parent = this;
        child->Hide();
    }
}}