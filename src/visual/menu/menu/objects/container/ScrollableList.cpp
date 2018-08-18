#include <SDL2/SDL_events.h>
#include <menu/object/container/Container.hpp>
#include <menu/object/container/ScrollableList.hpp>

/*
  Created on 27.07.18.
*/

bool zerokernel::ScrollableList::handleSdlEvent(SDL_Event *event)
{
    cleanupObjects();
    bool handled{ false };
    for (auto i = start_index; i < objects.size() && i <= end_index; ++i)
    {
        if (!objects[i]->isHidden() && objects[i]->handleSdlEvent(event))
            handled = true;
    }
    if (handled)
        return true;

    if (event->type == SDL_MOUSEWHEEL)
    {
        if (isHovered())
        {
            if (event->wheel.y > 0)
                scrollUp();
            else
                scrollDown();
            return true;
        }
    }

    return false;
}

void zerokernel::ScrollableList::updateScroll()
{
    auto height = bb.getContentBox().height;
    int acc{ 0 };
    end_index = 0;

    for (auto i = start_index; i < objects.size(); ++i)
    {
        if (objects[i]->isHidden())
            continue;
        int oh = objects[i]->getBoundingBox().getFullBox().height;
        if (oh + acc <= height)
        {
            objects[i]->move(0, acc);
            end_index = i;
        }
        acc += oh;
    }
}

void zerokernel::ScrollableList::scrollUp()
{
    if (start_index == 0)
        return;

    auto delta = 1;

    if (SDL_GetModState() & KMOD_SHIFT)
        delta = 4;

    if (delta > start_index)
        delta = start_index;

    start_index -= delta;
    updateScroll();
}

void zerokernel::ScrollableList::scrollDown()
{
    // FIXME
    /*if (end_index >= objects.size() - 1)
        return;*/

    auto delta = 1;

    if (SDL_GetModState() & KMOD_SHIFT)
        delta = 4;

    start_index += delta;

    updateScroll();
}

void zerokernel::ScrollableList::reorderElements()
{
    updateScroll();
    printf("SI/EI %u/%u\n", start_index, end_index);
}

void zerokernel::ScrollableList::render()
{
    for (auto i = start_index; i < objects.size() && i <= end_index; ++i)
    {
        if (!objects[i]->isHidden())
            objects[i]->render();
    }
}

void zerokernel::ScrollableList::update()
{
    for (auto i = start_index; i < objects.size() && i <= end_index; ++i)
    {
        if (!objects[i]->isHidden())
            objects[i]->update();
    }

    if (reorder_needed)
    {
        reorderElements();
        reorder_needed = false;
    }

    BaseMenuObject::update();
}

void zerokernel::ScrollableList::updateIsHovered()
{
    for (auto i = start_index; i < objects.size() && i <= end_index; ++i)
    {
        if (!objects[i]->isHidden() && objects[i]->containsMouse())
        {
            objects[i]->updateIsHovered();
            break;
        }
    }

    BaseMenuObject::updateIsHovered();
}

void zerokernel::ScrollableList::renderDebugOverlay()
{
    BaseMenuObject::renderDebugOverlay();

    for (auto i = start_index; i < objects.size() && i <= end_index; ++i)
    {
        if (!objects[i]->isHidden())
            objects[i]->renderDebugOverlay();
    }
}
