#include <SDL2/SDL_events.h>
#include <menu/BaseMenuObject.hpp>
#include <menu/special/TreeListBaseEntry.hpp>
#include <glez/draw.hpp>
#include <menu/Menu.hpp>
#include <settings/Registered.hpp>

/*
  Created on 26.07.18.
*/

static settings::RVariable<glez::rgba> background_hover{
    "zk.style.tree-list-entry.color.hover", "38b28f66"
};
static settings::RVariable<glez::rgba> lines{
    "zk.style.tree-list-entry.color.lines", "38b28f"
};

bool zerokernel::TreeListBaseEntry::handleSdlEvent(SDL_Event *event)
{
    return BaseMenuObject::handleSdlEvent(event);
}

void zerokernel::TreeListBaseEntry::render()
{
    if (isHovered())
    {
        renderBackground(*background_hover);
    }

    for (int i = 0; i <= int(depth) - 1; ++i)
    {
        glez::draw::line(bb.getBorderBox().left() + 2 + i * 5,
                         bb.getBorderBox().top() - 1, 0,
                         bb.getBorderBox().height + 3, *lines, 1);
    }
    if (depth)
    {
        glez::draw::line(bb.getBorderBox().left() + getRenderOffset() - 8,
                         bb.getBorderBox().top() + bb.getBorderBox().height / 2,
                         4, 0, *lines, 1);
    }

    BaseMenuObject::render();
}

void zerokernel::TreeListBaseEntry::onCollapse()
{
    hidden = true;
}

void zerokernel::TreeListBaseEntry::onRestore()
{
    hidden = false;
}

int zerokernel::TreeListBaseEntry::getRenderOffset()
{
    return depth * 5 + (depth ? 6 : 0);
}

zerokernel::TreeListBaseEntry::TreeListBaseEntry() : BaseMenuObject()
{
    bb.width.mode = BoundingBox::SizeMode::Mode::FILL;
}

void zerokernel::TreeListBaseEntry::setDepth(int depth)
{
    this->depth = depth;
}
