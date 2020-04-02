/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/Tooltip.hpp>

#include <menu/Menu.hpp>

namespace zerokernel_tooltip
{
static settings::RVariable<rgba_t> color_background{ "zk.style.tooltip.background", "1d2f40" };
static settings::RVariable<rgba_t> color_border{ "zk.style.tooltip.border", "446498ff" };
} // namespace zerokernel_tooltip
namespace zerokernel
{

Tooltip::Tooltip() : BaseMenuObject{}
{
    text.setParent(this);
    bb.width.mode  = BoundingBox::SizeMode::Mode::CONTENT;
    bb.height.mode = BoundingBox::SizeMode::Mode::CONTENT;
}

void Tooltip::render()
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    x += 9;
    move(x, y);

    text.renderForTooltip();

    BaseMenuObject::render();
}

void Tooltip::setText(std::string text)
{
    if (text == lastText)
        return;

    lastText = text;
    int lc;
    int width;
    text = utility::wrapString(text, resource::font::base, 420, &width, &lc);
    this->text.set(text);
}

void Tooltip::onMove()
{
    BaseMenuObject::onMove();

    text.onParentMove();
}
} // namespace zerokernel
