/*
  Copyright (c) 2020 nullworks. All rights reserved.
*/

#include <menu/object/container/Container.hpp>
#include <iostream>
#include <menu/ObjectFactory.hpp>
#include <menu/Menu.hpp>

namespace zerokernel
{

void Container::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);

    fillFromXml(data);
}

void Container::updateIsHovered()
{
    BaseMenuObject::updateIsHovered();

    for (auto it = objects.rbegin(); it != objects.rend(); ++it)
    {
        if ((*it)->isHidden())
            continue;
        if ((*it)->containsMouse())
        {
            (*it)->updateIsHovered();
            return;
        }
    }
}

void Container::emitSizeUpdate()
{
    BaseMenuObject::emitSizeUpdate();

    reorder_needed = true;

    for (auto &o : objects)
        o->onParentSizeUpdate();
}

void Container::recursiveSizeUpdate()
{
    BaseMenuObject::recursiveSizeUpdate();

    for (auto &o : objects)
        o->recursiveSizeUpdate();
}

void Container::fillFromXml(const tinyxml2::XMLElement *element)
{
    auto el = element->FirstChildElement();

    while (el != nullptr)
    {
        auto e = ObjectFactory::createObjectFromXml(el);
        if (e)
            addObject(std::move(e));
        el = el->NextSiblingElement();
    }
}

void Container::iterateObjects(std::function<void(BaseMenuObject *)> callback)
{
    for (auto &o : objects)
        callback(o.get());
}

BaseMenuObject *Container::findElement(const std::function<bool(BaseMenuObject *)> &search)
{
    auto result = BaseMenuObject::findElement(search);
    if (result)
        return result;

    for (auto &o : objects)
    {
        result = o->findElement(search);
        if (result)
            return result;
    }
    return nullptr;
}

bool Container::handleSdlEvent(SDL_Event *event)
{
    cleanupObjects();
    bool handled{ false };
    for (auto &object : objects)
    {
        if (!object->isHidden() && object->handleSdlEvent(event))
            handled = true;
    }
    return handled;
}

void Container::render()
{
    for (auto &object : objects)
    {
        if (!object->isHidden())
            object->render();
    }
    BaseMenuObject::render();
}

void Container::update()
{
    cleanupObjects();
    if (reorder_needed)
    {
        reorderElements();
        reorder_needed = false;
    }
    for (auto &object : objects)
        object->update();
}

void Container::addObject(std::unique_ptr<BaseMenuObject> &&object)
{
    object->setParent(this);
    objects.push_back(std::move(object));
    reorder_needed = true;
    recalculateSize();
}

void Container::cleanupObjects()
{
    for (auto it = objects.begin(); it != objects.end();)
    {
        if ((*it)->markedForDelete)
        {
            it             = objects.erase(it);
            reorder_needed = true;
        }
        else
        {
            ++it;
        }
    }
}

void Container::reset()
{
    objects.clear();
}

void Container::reorderElements()
{
}

void Container::onMove()
{
    BaseMenuObject::onMove();

    for (auto &o : objects)
        o->onParentMove();
}

void Container::onChildMove()
{
    bb.onChildSizeUpdate();
}

void Container::recalculateSize()
{
    bb.shrinkContent();
    bb.updateFillSize();

    for (auto &o : objects)
        bb.extend(o->getBoundingBox());
}

void Container::renderDebugOverlay()
{
    BaseMenuObject::renderDebugOverlay();

    for (auto &o : objects)
    {
        if (!o->isHidden())
            o->renderDebugOverlay();
    }
}
} // namespace zerokernel
