#include <menu/BaseMenuObject.hpp>
#include <menu/wm/WindowCloseButton.hpp>

#include <menu/wm/WMWindow.hpp>
#include "pathio.hpp"
#include "drawing.hpp"

/*
  Created on 07.07.18.
*/

namespace zerokernel_windowclosebutton
{
static draw::Texture cross{ paths::getDataPath( "/menu/cross.png") };
static settings::RVariable<rgba_t> background_hover{ "zk.style.window-close-button.color.background-hover", "ff0000" };
static settings::RVariable<rgba_t> color_border{ "zk.style.window-close-button.color.border", "446498ff" };
} // namespace zerokernel_windowclosebutton

void zerokernel::WindowCloseButton::render()
{
    auto cb = bb.getBorderBox();
    if (isHovered())
        draw::Rectangle(cb.left(), cb.top(), cb.width, cb.height - 1, *zerokernel_windowclosebutton::background_hover);
    // glez::draw::line(cb.left(), cb.top(), 0, cb.height, *color_border, 1);
    renderBorder(*zerokernel_windowclosebutton::color_border);
    draw::RectangleTextured(cb.x + 1, cb.y, cb.width, cb.height, colors::white, zerokernel_windowclosebutton::cross, 0, 0, 14, 14, 0);
}

zerokernel::WindowCloseButton::WindowCloseButton() : BaseMenuObject{}
{
    bb.resize(16, 16);
}

bool zerokernel::WindowCloseButton::onLeftMouseClick()
{
    BaseMenuObject::onLeftMouseClick();

    return true;
}
