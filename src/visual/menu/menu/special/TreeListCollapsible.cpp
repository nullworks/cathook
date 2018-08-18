#include <SDL2/SDL_events.h>
#include <menu/special/TreeListBaseEntry.hpp>
#include <menu/special/TreeListCollapsible.hpp>
#include <menu/object/container/Container.hpp>
#include <menu/Menu.hpp>

/*
  Created on 26.07.18.
*/

bool zerokernel::TreeListCollapsible::handleSdlEvent(SDL_Event *event)
{
    if (event->type == SDL_MOUSEBUTTONDOWN)
    {
        if (isHovered())
        {
            printf("collapse: %d\n", is_collapsed);
            // Collapse/uncollapse stuff
            if (!is_collapsed)
                listCollapse();
            else
                listUncollapse();
            is_collapsed = !is_collapsed;
            setText(name);
        }
    }

    return TreeListBaseEntry::handleSdlEvent(event);
}

void zerokernel::TreeListCollapsible::render()
{
    TreeListBaseEntry::render();

    text.render();
}

void zerokernel::TreeListCollapsible::listCollapse()
{
    auto container = dynamic_cast<Container *>(parent);
    if (!container)
    {
        printf(
            "TreeListCollapsible: can't collapse: Container(parent) == NULL\n");
        return;
    }
    bool flag = false;
    for (auto &o : container->objects)
    {
        auto item = dynamic_cast<TreeListBaseEntry *>(o.get());
        if (item)
        {
            if (flag)
            {
                if (item->depth == depth + 1)
                {
                    item->onCollapse();
                }
                if (item->depth == depth)
                    break;
            }
            if (item == dynamic_cast<TreeListBaseEntry *>(this))
            {
                flag = true;
            }
        }
    }
    container->reorder_needed = true;
}

void zerokernel::TreeListCollapsible::listUncollapse()
{
    auto container = dynamic_cast<Container *>(parent);
    if (!container)
    {
        printf("TreeListCollapsible: can't uncollapse: Container(parent) == "
               "NULL\n");
        return;
    }
    bool flag = false;
    for (auto &o : container->objects)
    {
        auto item = dynamic_cast<TreeListBaseEntry *>(o.get());
        if (item)
        {
            if (flag)
            {
                if (item->depth == depth + 1)
                {
                    item->onRestore();
                }
                else if (item->depth <= depth)
                    break;
            }
            if (item == dynamic_cast<TreeListBaseEntry *>(this))
            {
                flag = true;
            }
        }
    }
    container->reorder_needed = true;
}

void zerokernel::TreeListCollapsible::onCollapse()
{
    TreeListBaseEntry::onCollapse();

    listCollapse();
}

void zerokernel::TreeListCollapsible::onRestore()
{
    TreeListBaseEntry::onRestore();

    if (!is_collapsed)
        listUncollapse();
}

void zerokernel::TreeListCollapsible::setText(std::string data)
{
    name = data;
    text.set((is_collapsed ? "[+] " : "[-] ") + data);
    text.setParent(this);
}

void zerokernel::TreeListCollapsible::onMove()
{
    BaseMenuObject::onMove();

    text.onParentMove();
}

void zerokernel::TreeListCollapsible::recalculateSize()
{
    BaseMenuObject::recalculateSize();

    bb.shrinkContent();
    bb.extend(text.getBoundingBox());
}

zerokernel::TreeListCollapsible::TreeListCollapsible()
{
    bb.width.setFill();
    bb.height.setContent();
    bb.height.min = 12;
    bb.normalizeSize();
    text.setParent(this);
    text.font = &resource::font::bold;
    text.bb.height.setFill();
}

void zerokernel::TreeListCollapsible::setDepth(int depth)
{
    TreeListBaseEntry::setDepth(depth);

    text.move(depth * 5 + 4, 0);
}
