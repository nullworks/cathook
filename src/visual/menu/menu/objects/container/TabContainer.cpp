/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/object/container/TabContainer.hpp>

namespace zerokernel
{

void TabContainer::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);

    auto el = data->FirstChildElement("Tab");
    while (el != nullptr)
    {
        const char *name = "Unnamed";
        el->QueryStringAttribute("name", &name);
        addTab(name);
        containers.at(containers.size() - 1)->loadFromXml(el);
        el = el->NextSiblingElement("Tab");
    }
}

void TabContainer::updateIsHovered()
{
    BaseMenuObject::updateIsHovered();

    if (selection.containsMouse())
        selection.updateIsHovered();
    else if (!containers.empty())
    {
        if (containers.at(selection.active)->containsMouse())
            containers.at(selection.active)->updateIsHovered();
    }
}

void TabContainer::emitSizeUpdate()
{
    BaseMenuObject::emitSizeUpdate();

    selection.onParentSizeUpdate();
    for (auto &c : containers)
        c->onParentSizeUpdate();
}

void TabContainer::recalculateSize()
{
    BaseMenuObject::recalculateSize();

    // TODO fixme
}

void TabContainer::recursiveSizeUpdate()
{
    BaseMenuObject::recursiveSizeUpdate();

    selection.recursiveSizeUpdate();
    for (auto &c : containers)
        c->recursiveSizeUpdate();
}

BaseMenuObject *
TabContainer::findElement(const std::function<bool(BaseMenuObject *)> &search)
{
    auto result = BaseMenuObject::findElement(search);
    if (result)
        return result;

    for (auto &c : containers)
    {
        result = c->findElement(search);
        if (result)
            return result;
    }
    return nullptr;
}

bool TabContainer::handleSdlEvent(SDL_Event *event)
{
    selection.handleSdlEvent(event);
    auto active = getActiveContainer();
    if (active)
        if (active->handleSdlEvent(event))
            return true;

    return BaseMenuObject::handleSdlEvent(event);
}

void TabContainer::render()
{
    selection.render();
    auto active = getActiveContainer();
    if (active)
        active->render();
}

void TabContainer::update()
{
    selection.update();
    auto active = getActiveContainer();
    if (active)
        active->update();
}

void TabContainer::addTab(std::string title)
{
    selection.add(title);

    auto container = std::make_unique<Container>();
    container->setParent(this);
    container->move(0, selection.getBoundingBox().getFullBox().height);
    container->getBoundingBox().width.mode  = BoundingBox::SizeMode::Mode::FILL;
    container->getBoundingBox().height.mode = BoundingBox::SizeMode::Mode::FILL;

    containers.push_back(std::move(container));
}

Container *TabContainer::getTab(std::string title)
{
    for (auto i = 0u; i < selection.options.size(); ++i)
    {
        if (selection.options.at(i) == title)
        {
            return containers.at(i).get();
        }
    }
    return nullptr;
}

void TabContainer::onMove()
{
    BaseMenuObject::onMove();

    selection.onParentMove();
    for (auto &c : containers)
        c->onParentMove();
}

void TabContainer::renderDebugOverlay()
{
    BaseMenuObject::renderDebugOverlay();

    selection.renderDebugOverlay();

    auto active = getActiveContainer();
    if (active)
        active->renderDebugOverlay();
}

TabContainer::TabContainer() : BaseMenuObject{}, selection{ *this }
{
}

Container *TabContainer::getActiveContainer()
{
    if (containers.empty())
        return nullptr;
    if (selection.active >= containers.size())
    {
        printf("WARNING: TabContainer: selection.active >= container.size()\n");
        return nullptr;
    }
    return containers.at(selection.active).get();
}
} // namespace zerokernel