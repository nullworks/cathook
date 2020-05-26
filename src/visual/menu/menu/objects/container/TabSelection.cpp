/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/object/container/TabSelection.hpp>
#include <menu/object/TabButton.hpp>

extern settings::RVariable<rgba_t> color_border;

namespace zerokernel
{

void TabSelection::render()
{
    Container::render();
    draw::Line(bb.getBorderBox().left(), bb.getBorderBox().bottom() - 1, bb.getBorderBox().width, 0, *color_border, 1);
}

void TabSelection::add(const std::string &option)
{
    options.push_back(option);
    std::unique_ptr<TabButton> button = std::make_unique<TabButton>(*this, options.size() - 1);
    addObject(std::move(button));
}

TabSelection::TabSelection(TabContainer &parent) : Container{}, container(parent)
{
    setParent(&parent);
    bb.width.setFill();
    bb.height.setContent();
}

void TabSelection::selectTab(size_t id)
{
    active = id;
}

void TabSelection::reorderElements()
{
    // TODO
    int acc{ 0 };
    for (auto &o : objects)
    {
        o->move(acc + o->getBoundingBox().margin.left, 0);
        acc += o->getBoundingBox().getFullBox().width;
    }
}
} // namespace zerokernel
