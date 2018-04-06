#include "catvar.hpp"
#include "../../framework/input.hpp"
#include "../../framework/drawing.hpp"
#include "../gui.hpp"
#include "../../util/strings.hpp" // sepstr()
#include <chrono>	// For time keeping
#include <cmath>	// For fmod
#include <ratio> // above

/*
A CatVar

*/

namespace gui { namespace menu {
    CCatVar::CCatVar(CatVar * cv):CBaseWidget(cv->name), cv(cv){
    }
    void CCatVar::Draw(){
        auto color=focus?(typing?colors::pink:colors::RainbowCurrent()):(hover?colors::lightgray:colors::gray);
    	draw::RectFilled(global_pos.first, global_pos.second, size.first, size.second, color.Transparent(0.3));
    	draw::Rect(global_pos.first, global_pos.second, size.first, size.second, color);
        auto textsize=draw::GetStringLength(cv->desc_short,draw::default_font.value,draw::default_font_size.value);
    	draw::RectFilled(global_pos.first, global_pos.second, textsize.first, size.second, colors::black.Transparent(0.3));
        draw::String(cv->desc_short,global_pos.first,global_pos.second+(size.second-textsize.second)/2,draw::default_font.value,draw::default_font_size.value,colors::white);
        textsize=draw::GetStringLength(content.c_str(),draw::default_font.value,draw::default_font_size.value);
        draw::String(content.c_str(),global_pos.first+size.first-textsize.first,global_pos.second+(size.second-textsize.second)/2,draw::default_font.value,draw::default_font_size.value,colors::white);
        if(typing){
	        std::chrono::duration<float, std::deca> curtime = std::chrono::steady_clock::now() - blink_start_time;
            if(std::fmod(curtime.count(), blink_period)<blink_vis_time){
                auto beep = global_pos.first-textsize.first+draw::GetStringLength(content.substr(0,cursor).c_str(),draw::default_font.value,draw::default_font_size.value).first;
                //colors::RainbowCurrent()
            }
        }
        CBaseWidget::Draw();
    }
    void CCatVar::UpdatePositioning(){
        int minwidth = size.first;
        if (!typing||!focus) content=cv->GetValue();
        auto label=draw::GetStringLength(cv->desc_short,draw::default_font.value,draw::default_font_size.value);
        size=draw::GetStringLength(content.c_str(),draw::default_font.value,draw::default_font_size.value);
        size.first=std::max(label.first+min_mid_padding+size.first, minwidth);
        size.second=std::max(label.second,size.second)+padding.second*2;
        CBaseWidget::UpdatePositioning();
    }
void CCatVar::OnFocusLose() {
    if(typing) cv->callback(sepstr(content));
    typing=false;
	CBaseWidget::OnFocusLose();
}
void CCatVar::OnKeyPress(int key) {
    if (key==activatekey.value&&focus&&!typing){
        typing=true;
        cursor=content.size();
        blink_start_time = std::chrono::steady_clock::now();
    }
    if(typing){
        blink_start_time = std::chrono::steady_clock::now();
        if (key==field_escapekey.value){
            cv->callback(sepstr(content));
            typing=false;
        }else if (key==field_leftkey.value){
            if(cursor>0){
                cursor--;
            }
        }else if (key==field_rightkey.value){
            if(cursor<content.size()){
                cursor++;
            }
        }else{
            //TODO: Text Input
        }
    }

}
void CCatVar::OnKeyRelease(int key) {
	//Tempted to just fully hand these key release events on, but that'll just slow things down.
}
bool CCatVar::ConsumesKey(int key) {
    //If we're typing, or we are about to start
	return (typing)	|| (focus && key==activatekey);
}
}}
