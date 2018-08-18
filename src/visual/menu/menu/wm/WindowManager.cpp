/*
  Created on 06.07.18.
*/

#include <menu/BaseMenuObject.hpp>
#include <menu/wm/WindowManager.hpp>

void zerokernel::WindowManager::loadFromXml(const tinyxml2::XMLElement *data)
{
    Container::loadFromXml(data);

    auto window = data->FirstChildElement("Window");
    while (window)
    {
        loadWindowFromXml(window);
        window = window->NextSiblingElement("Window");
    }
}

zerokernel::WindowManager::WindowManager() : BaseMenuObject{}
{
    bb.width.setFill();
    bb.height.setFill();
}

void zerokernel::WindowManager::loadWindowFromXml(
    const tinyxml2::XMLElement *data)
{
    const char *title{ nullptr };
    const char *title_short{ nullptr };

    if (data->QueryStringAttribute("name", &title))
    {
        printf("ERROR: Window must have a name\n");
        return;
    }
    if (data->QueryStringAttribute("short_name", &title_short))
        title_short = title;

    auto &win = container->wmCreateWindow();
    win.loadFromXml(data);
    win.name = title;
    if (title_short)
        win.short_name = title_short;
    else
        win.short_name = title;
    win.header->updateTitle();
    bar->addWindowButton(win);
}

zerokernel::WMWindow *zerokernel::WindowManager::findWindow(size_t uid)
{
    return container->wmFindWindow(uid);
}

zerokernel::WMWindow *zerokernel::WindowManager::getActiveWindow()
{
    return container->wmGetActive();
}

void zerokernel::WindowManager::focusOnWindow(size_t uid)
{
    container->wmRequestFocus(uid);
}

void zerokernel::WindowManager::init()
{
    auto container  = std::make_unique<WindowContainer>(*this);
    this->container = container.get();
    container->move(0, 0);
    addObject(std::move(container));

    auto bar  = std::make_unique<TaskBar>(*this);
    this->bar = bar.get();
    bar->move(0, 0);
    addObject(std::move(bar));
}
