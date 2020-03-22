/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/Menu.hpp>
#include <menu/object/container/Box.hpp>
#include <menu/object/input/Checkbox.hpp>
#include <menu/object/container/TabSelection.hpp>
#include <menu/object/container/TabContainer.hpp>
#include <menu/object/input/InputKey.hpp>
#include <menu/object/input/Slider.hpp>
#include <menu/object/input/ColorSelector.hpp>
#include <menu/object/input/Select.hpp>
#include <menu/special/SettingsManagerList.hpp>
#include <menu/special/PlayerListController.hpp>
#include <menu/object/container/ScrollableList.hpp>
#include <fstream>
#include <iostream>
#include <menu/menu/Menu.hpp>
#include <config.h>
#include <core/logging.hpp>
#include "pathio.hpp"

static void recursiveXmlResolveIncludes(const std::string &directory, tinyxml2::XMLElement *element)
{
    printf("recursively resolving includes for element %s\n", element->Name());
    auto c = element->FirstChildElement();
    while (c)
    {
        if (!strcmp("Include", c->Name()))
        {
            printf("Include element found\n");
            const char *path{ nullptr };
            if (tinyxml2::XML_SUCCESS == c->QueryStringAttribute("path", &path))
            {
                printf("Trying to include: path='%s'\n", path);
                std::string fp = directory + "/" + std::string(path);
                printf("Full path: %s\n", fp.c_str());

                tinyxml2::XMLDocument document{};
                if (tinyxml2::XML_SUCCESS == document.LoadFile(fp.c_str()))
                {
                    printf("File loaded\n");
                    auto content = document.RootElement()->DeepClone(c->GetDocument());
                    element->InsertAfterChild(c, content);
                    element->DeleteChild(c);
                    c = content->ToElement();
                    continue;
                }
            }
        }
        if (!strcmp("ElementGroup", c->Name()))
        {
            auto it   = c->FirstChild();
            auto prev = c;
            while (it)
            {
                auto next = it->NextSibling();
                element->InsertAfterChild(prev, it);
                printf("GroupedElement: %s\n", it->ToElement()->Name());
                it = next;
            }
            auto n = c->NextSiblingElement();
            element->DeleteChild(c);
            c = n;
            continue;
        }
        recursiveXmlResolveIncludes(directory, c);
        c = c->NextSiblingElement();
    }
}

namespace zerokernel
{

Menu *Menu::instance{ nullptr };

namespace resource::font
{
// FIXME dynamic font change..
#if ENABLE_IMGUI_DRAWING
fonts::font base{ paths::getDataPath("/menu/Verdana.ttf"), 12 };
fonts::font bold{ paths::getDataPath("/menu/VerdanaBold.ttf"), 11 };
#else
fonts::font base{ paths::getDataPath("/menu/Verdana.ttf"), 10 };
fonts::font bold{ paths::getDataPath("/menu/VerdanaBold.ttf"), 9 };
#endif
} // namespace resource::font

namespace style::colors
{

color_type text{ "zk.style.menu.color.text", "c4d3e1" };
color_type text_shadow{ "zk.style.menu.color.text_shadow", "000000" };

color_type error{ "zk.style.menu.color.error", "ff0000" };
} // namespace style::colors

Menu::Menu(int w, int h) : tooltip{}, wm{ nullptr }
{
    screen_x = w;
    screen_y = h;
}

void Menu::setup()
{
}

void Menu::showTooltip(std::string tooltip)
{
    if (!ready)
        return;
    this->tooltip.shown = true;
    this->tooltip.setText(tooltip);
}

bool Menu::handleSdlEvent(SDL_Event *event)
{
    if (!ready)
        return false;

    if (!modal_stack.empty())
        return modal_stack.back()->handleSdlEvent(event);

    if (wm)
        return wm->handleSdlEvent(event);
    return false;
}

void Menu::render()
{
    if (!ready)
        return;
    wm->render();
    wm->renderDebugOverlay();

    // TODO maybe move these into WM...
    for (auto &m : modal_stack)
    {
        m->render();
    }

    if (tooltip.shown)
        tooltip.render();
    tooltip.shown = false;
}

void Menu::update()
{
    if (!ready)
        return;
    ++frame;
    updateHovered();
    if (modal_close_next_frame > 0)
    {
        while (!modal_stack.empty() && modal_close_next_frame)
        {
            modal_stack.pop_back();
            --modal_close_next_frame;
        }
        if (modal_close_next_frame != 0)
            printf("WARNING: %d queued modal close events, modal stack empty\n", modal_close_next_frame);
    }
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    dx     = mx - mouseX;
    dy     = my - mouseY;
    mouseX = mx;
    mouseY = my;

    wm->update();

    for (auto &m : modal_stack)
    {
        m->update();
    }

    if (tooltip.shown)
        tooltip.update();
}

void Menu::init(int w, int h)
{
    instance = new Menu(w, h);
    instance->setup();
}

void Menu::destroy()
{
    delete instance;
}

void Menu::reset()
{
    special::SettingsManagerList::resetMarks();
    wm = std::make_unique<WindowManager>();
    wm->init();
    ready = true;
}

void Menu::loadFromXml(tinyxml2::XMLElement *element)
{
    reset();

    tinyxml2::XMLElement *prefab = element->FirstChildElement("Prefab");
    while (prefab)
    {
        const char *id;
        if (!prefab->QueryStringAttribute("id", &id))
        {
            prefabs[id] = prefab;
        }
        prefab = prefab->NextSiblingElement("Prefab");
    }

    wm->resize(screen_x, screen_y);
    wm->loadFromXml(element);
    wm->move(0, 0);
    wm->recursiveSizeUpdate();
}

void Menu::addModalObject(std::unique_ptr<BaseMenuObject> &&object)
{
    object->addMessageHandler(*this);
    printf("Adding %p[%u] to modal stack\n", object.get(), object->uid);
    modal_stack.push_back(std::move(object));
}

void Menu::handleMessage(Message &msg, bool is_relayed)
{
    if (is_relayed)
        return;

    if (msg.name == "ModalClose")
    {
        if (!modal_stack.empty())
        {
            printf("%p[%u] requested close, top %p\n", msg.sender, msg.sender->uid, modal_stack.back().get());
            if (modal_stack.back().get() == msg.sender)
                ++modal_close_next_frame;
        }
        // Warning
    }
}

void Menu::updateHovered()
{
    for (auto i = 0; i < modal_stack.size(); ++i)
    {
        auto &m = modal_stack[modal_stack.size() - 1 - i];
        if (m->containsMouse())
        {
            m->updateIsHovered();
            return;
        }
    }
    wm->updateIsHovered();
}

void Menu::resize(int x, int y)
{
    screen_x = x;
    screen_y = y;
    wm->onParentSizeUpdate();
}

tinyxml2::XMLElement *Menu::getPrefab(const std::string &name)
{
    auto it = prefabs.find(name);
    if (it == prefabs.end())
        return nullptr;
    return it->second;
}

BoundingBox &Menu::wmRootBoundingBox()
{
    return wm->getBoundingBox();
}

bool Menu::isInGame()
{
    return in_game;
}

void Menu::setInGame(bool flag)
{
    in_game = flag;
}

void Menu::loadFromFile(std::string directory, std::string path)
{
    xml_source.LoadFile((directory + "/" + path).c_str());
    recursiveXmlResolveIncludes(directory, xml_source.RootElement());
    loadFromXml(xml_source.RootElement());
}
} // namespace zerokernel
