/*
  Created on 06.07.18.
*/
#include <menu/object/container/Container.hpp>
#include <menu/wm/TaskBar.hpp>
#include <menu/wm/WMWindow.hpp>
#include <menu/wm/Task.hpp>
#include <menu/Menu.hpp>

extern settings::RVariable<rgba_t> color_border;
extern settings::RVariable<rgba_t> color_background;

void zerokernel::TaskBar::reorderElements()
{
    int acc{ 0 };
    for (auto &i : objects)
    {
        acc += i->getBoundingBox().margin.left;
        i->move(acc, i->getBoundingBox().margin.top);
        acc += i->getBoundingBox().getFullBox().width - i->getBoundingBox().margin.left;
    }
}

zerokernel::TaskBar::TaskBar(zerokernel::WindowManager &wm) : BaseMenuObject{}, wm(wm)
{
    bb.width.setFill();
    bb.height.setContent();
}

bool zerokernel::TaskBar::isHidden()
{
    return BaseMenuObject::isHidden() || Menu::instance->isInGame();
}

void zerokernel::TaskBar::addWindowButton(zerokernel::WMWindow &window)
{
    addObject(std::make_unique<Task>(window));
}

void zerokernel::TaskBar::render()
{
    renderBackground(*color_background);
    renderBorder(*color_border);

    Container::render();
}
