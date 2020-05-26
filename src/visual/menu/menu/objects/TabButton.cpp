/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/object/TabButton.hpp>

extern settings::RVariable<rgba_t> color_border;
extern settings::RVariable<rgba_t> color_background;
extern settings::RVariable<rgba_t> color_focused;
extern settings::RVariable<rgba_t> color_open;

namespace zerokernel
{

TabButton::TabButton(TabSelection &parent, size_t id) : BaseMenuObject{}, parent(parent), id(id)
{
    setParent(&parent);
    // FIXME height
    bb.resize(-1, 14);
    text.setParent(this);
    text.move(0, 0);
    text.bb.setPadding(2, 0, 6, 6);
    text.bb.width.setContent();
    text.bb.height.setFill();
    text.set(parent.options.at(id));
    bb.width.setContent();
}

void TabButton::render()
{
    bool selected = (parent.active == id);
    if (selected)
    {
        renderBackground(*color_border);
    }
    else if (isHovered())
    {
        draw::Line(bb.getBorderBox().x + 6, bb.getBorderBox().bottom() - 3, bb.getBorderBox().width - 12, 0, *color_border, 1);
    }

    draw::Line(bb.getBorderBox().right(), bb.getBorderBox().top(), 0, bb.getBorderBox().height, *color_border, 1);

    text.setColorText(selected ? &*color_focused : &*color_open);
    text.render();
}

bool TabButton::onLeftMouseClick()
{
    parent.selectTab(id);
    BaseMenuObject::onLeftMouseClick();
    return true;
}

void TabButton::recalculateSize()
{
    BaseMenuObject::recalculateSize();

    bb.shrinkContent();
    bb.extend(text.getBoundingBox());
}

void TabButton::onMove()
{
    BaseMenuObject::onMove();

    text.onParentMove();
}

void TabButton::recursiveSizeUpdate()
{
    BaseMenuObject::recursiveSizeUpdate();

    text.recursiveSizeUpdate();
}
} // namespace zerokernel
