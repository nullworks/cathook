/*
  Copyright (c) 2020 nullworks. All rights reserved.
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

Tooltip::Tooltip() : Container{}
{
    bb.width.mode  = BoundingBox::SizeMode::Mode::CONTENT;
    bb.height.mode = BoundingBox::SizeMode::Mode::CONTENT;
}

void Tooltip::createText()
{
    auto text = std::make_unique<Text>();
    text->setParent(this);
    this->text = text.get();
    bb.setPadding(5, 4, 5, 5);
    text->bb.width.setContent();
    text->bb.height.setContent();
    addObject(std::move(text));
}

void Tooltip::render()
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    x += 10;
    move(x, y);

    renderBackground(*zerokernel_tooltip::color_background);
    renderBorder(*zerokernel_tooltip::color_border);

    Container::render();
}

void Tooltip::setText(std::string text)
{
    if (text == lastText)
        return;

    lastText = text;
    int lc;
    int width;
    text = utility::wrapString(text, resource::font::base, 300, &width, &lc);
    if (!this->text)
        createText();
    this->text->set(text);
}
} // namespace zerokernel
