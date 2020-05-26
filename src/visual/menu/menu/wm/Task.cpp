#include <menu/BaseMenuObject.hpp>
#include <menu/wm/Task.hpp>
#include <menu/Menu.hpp>

/*
  Created on 06.07.18.
*/

// TODO FIXME Style?!

static settings::RVariable<rgba_t> color_closed{ "zk.style.task.color.text.closed", "888888" };
settings::RVariable<rgba_t> color_focused{ "zk.style.task.color.text.focused", "ffffff" };
settings::RVariable<rgba_t> color_open{ "zk.style.task.color.text.open", "c4d3e1" };

extern settings::RVariable<rgba_t> color_border;

void zerokernel::Task::render()
{
    if (isHovered())
        renderBackground(*color_border);
    renderBorder(*color_border);

    if (window.isFocused())
    {
        text.setColorText(&*color_focused);
    }
    else
    {
        if (window.isHidden())
            text.setColorText(&*color_closed);
        else
            text.setColorText(&*color_open);
    }
    text.render();
}

bool zerokernel::Task::onLeftMouseClick()
{
    if (window.isHidden())
    {
        window.wmOpenWindow();
        window.container.wmRequestFocus(window.uid);
    }
    else
    {
        if (window.isFocused())
            window.wmCloseWindow();
        else
            window.requestFocus();
    }

    return true;
}

zerokernel::Task::Task(zerokernel::WMWindow &window) : BaseMenuObject{}, window(window)
{
    bb.width.setContent();
    bb.setMargin(4, 4, 4, 4);
    // FIXME "fontHeight"
    resize(-1, 14);
    text.bb.setPadding(0, 0, 8, 8);
    text.bb.width.setContent();
    text.bb.height.setFill();
    text.setParent(this);
    text.set(window.short_name);
}

void zerokernel::Task::recalculateSize()
{
    BaseMenuObject::recalculateSize();

    bb.resize(-1, 14);
    bb.extend(text.getBoundingBox());
}

void zerokernel::Task::onMove()
{
    BaseMenuObject::onMove();

    text.onParentMove();
}

void zerokernel::Task::recursiveSizeUpdate()
{
    BaseMenuObject::recursiveSizeUpdate();

    text.recursiveSizeUpdate();
}

void zerokernel::Task::emitSizeUpdate()
{
    BaseMenuObject::emitSizeUpdate();

    text.onParentSizeUpdate();
}
