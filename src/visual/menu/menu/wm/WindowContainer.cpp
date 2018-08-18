/*
  Created on 06.07.18.
*/

#include <menu/BaseMenuObject.hpp>
#include <menu/wm/WindowContainer.hpp>

bool zerokernel::WindowContainer::handleSdlEvent(SDL_Event *event)
{
    if (isHovered())
    {
        if (event->type == SDL_MOUSEBUTTONDOWN)
        {
            bool found{ false };
            for (auto i = order.rbegin(); i != order.rend(); ++i)
            {
                if (windows[*i]->isHidden())
                    continue;
                if (windows[*i]->containsMouse())
                {
                    found = true;
                    wmRequestFocus(windows[*i]->uid);
                    break;
                }
            }
            if (!found)
                wmResetFocus();
        }
        for (auto i = order.rbegin(); i != order.rend(); ++i)
            if (!windows[*i]->isHidden() && windows[*i]->handleSdlEvent(event))
                return true;
    }

    return BaseMenuObject::handleSdlEvent(event);
}

void zerokernel::WindowContainer::render()
{
    for (auto i : order)
    {
        if (!windows[i]->isHidden())
            windows[i]->render();
    }

    BaseMenuObject::render();
}

void zerokernel::WindowContainer::update()
{
    if (request_move_top)
    {
        moveWindowToTop(request_move_top);
        request_move_top = 0;
    }

    for (auto i : order)
    {
        if (!windows[i]->isHidden())
            windows[i]->update();
    }

    BaseMenuObject::update();
}

zerokernel::WindowContainer::WindowContainer(zerokernel::WindowManager &wm)
    : BaseMenuObject{}, wm(wm)
{
    bb.width.setFill();
    bb.height.setFill();
}

zerokernel::WMWindow &zerokernel::WindowContainer::wmCreateWindow()
{
    // TODO
    auto window = std::make_unique<WMWindow>(*this);
    auto w      = window.get();
    windows.push_back(std::move(window));
    order.push_back(windows.size() - 1);
    return *w;
}

zerokernel::WMWindow *zerokernel::WindowContainer::wmFindWindow(size_t uid)
{
    for (auto &w : windows)
        if (w->uid == uid)
            return w.get();
    return nullptr;
}

zerokernel::WMWindow *zerokernel::WindowContainer::wmGetActive()
{
    return active_window;
}

void zerokernel::WindowContainer::wmRequestFocus(size_t uid)
{
    auto next_window = wmFindWindow(uid);
    if (active_window == next_window)
        return;
    if (active_window)
    {
        active_window->wmFocusLose();
        active_window = nullptr;
    }
    active_window = next_window;
    if (active_window)
    {
        active_window->wmFocusGain();
        request_move_top = uid;
    }
}

void zerokernel::WindowContainer::wmResetFocus()
{
    if (active_window)
    {
        active_window->wmFocusLose();
        active_window = nullptr;
    }
}

void zerokernel::WindowContainer::recursiveSizeUpdate()
{
    BaseMenuObject::recursiveSizeUpdate();

    for (auto &w : windows)
        w->recursiveSizeUpdate();
}

void zerokernel::WindowContainer::updateIsHovered()
{
    BaseMenuObject::updateIsHovered();

    for (auto i = order.rbegin(); i != order.rend(); ++i)
    {
        if (windows[*i]->isHidden())
            continue;
        if (windows[*i]->containsMouse())
        {
            windows[*i]->updateIsHovered();
            return;
        }
    }
}

void zerokernel::WindowContainer::emitSizeUpdate()
{
    BaseMenuObject::emitSizeUpdate();

    for (auto &w : windows)
        w->onParentSizeUpdate();
}

void zerokernel::WindowContainer::moveWindowToTop(size_t uid)
{
    for (auto it = order.begin(); it != order.end(); ++it)
    {
        if (windows[*it]->uid == uid)
        {
            printf("Found window\n");
            size_t index = *it;
            order.erase(it);
            order.push_back(index);
            return;
        }
    }
}

zerokernel::BaseMenuObject *zerokernel::WindowContainer::findElement(
    const std::function<bool(BaseMenuObject *)> &search)
{
    auto result = BaseMenuObject::findElement(search);
    if (result)
        return result;

    for (auto &w : windows)
    {
        result = w->findElement(search);
        if (result)
            return result;
    }
    return nullptr;
}

void zerokernel::WindowContainer::onMove()
{
    BaseMenuObject::onMove();

    printf("WindowContainer::onMove()\n");

    for (auto &w : windows)
        w->onParentMove();
}

void zerokernel::WindowContainer::renderDebugOverlay()
{
    BaseMenuObject::renderDebugOverlay();

    for (auto i : order)
    {
        if (!windows[i]->isHidden())
            windows[i]->renderDebugOverlay();
    }
}
