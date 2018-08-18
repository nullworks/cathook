/*
  Created on 06.07.18.
*/

#include <menu/BaseMenuObject.hpp>
#include <menu/Message.hpp>
#include <menu/wm/WMWindow.hpp>
#include <menu/wm/WindowContainer.hpp>
#include <menu/ObjectFactory.hpp>
#include <menu/Menu.hpp>
#include <menu/object/input/Select.hpp>
#include <menu/object/input/Checkbox.hpp>

static settings::RVariable<glez::rgba> color_border{
    "zk.style.window.color.border", "079797"
};
static settings::RVariable<glez::rgba> color_background{
    "zk.style.window.color.background.active", "1d2f40"
};
static settings::RVariable<glez::rgba> color_background_inactive{
    "zk.style.window.color.background.inactive", "1d2f4088"
};

namespace zerokernel
{

bool WMWindow::handleSdlEvent(SDL_Event *event)
{
    if (isHidden())
        return false;

    if (event->type == SDL_MOUSEBUTTONDOWN &&
        event->button.button == SDL_BUTTON_RIGHT)
    {
        if (isHovered())
        {
            if (SDL_GetModState() & KMOD_SHIFT)
            {
                openSettingsModal();
                return true;
            }
        }
    }

    return Container::handleSdlEvent(event);
}

void WMWindow::render()
{
    if (isHidden())
        return;

    if (isFocused())
        renderBackground(*color_background);
    else
        renderBackground(*color_background_inactive);
    renderBorder(*color_border);

    Container::render();
}

void WMWindow::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);

    contents->bb.padding = bb.padding;
    bb.setPadding(0, 0, 0, 0);

    contents->fillFromXml(data);

    contents->resize(bb.border_box.width, bb.border_box.height);
    contents->bb.width  = bb.width;
    contents->bb.height = bb.height;

    bb.width.setContent();
    bb.height.setContent();
}

void WMWindow::handleMessage(Message &msg, bool is_relayed)
{
    BaseMenuObject::handleMessage(msg, is_relayed);
}

void WMWindow::wmFocusGain()
{
    focused = true;
}

void WMWindow::wmFocusLose()
{
    focused = false;
}

void WMWindow::wmCloseWindow()
{
    hidden = true;
}

void WMWindow::wmOpenWindow()
{
    hidden = false;
}

void WMWindow::requestFocus()
{
    container.wmRequestFocus(uid);
}

void WMWindow::requestClose()
{
    wmCloseWindow();
}

void WMWindow::moveObjects()
{
    switch (location)
    {
    case HeaderLocation::TOP:
        header->move(0, 0);
        contents->move(0, header->getBoundingBox().getFullBox().height);
        break;
    case HeaderLocation::BOTTOM:
        contents->move(0, 0);
        header->move(0, contents->getBoundingBox().getFullBox().height);
        break;
    case HeaderLocation::HIDDEN:
        contents->move(0, 0);
        break;
    }
}

WMWindow::WMWindow(WindowContainer &container)
    : BaseMenuObject{}, container(container)
{
    setParent(&container);

    auto header_obj    = std::make_unique<WindowHeader>(*this);
    auto container_obj = std::make_unique<Container>();

    header   = header_obj.get();
    contents = container_obj.get();

    addObject(std::move(header_obj));
    addObject(std::move(container_obj));

    header_location.installChangeCallback(
        [this](settings::VariableBase<int> &var, int after) {
            location = static_cast<HeaderLocation>(after);
            moveObjects();
            recalculateSize();
        });
}

void WMWindow::openSettingsModal()
{
    printf("Opening settings modal...\n");
    auto window = ObjectFactory::createFromPrefab("window-settings");
    if (!window)
    {
        printf("WARNING: WMWindow::openSettingsModal: window == NULL\n");
        return;
    }
    auto select_header_location =
        dynamic_cast<Select *>(window->getElementById("header-location"));
    auto checkbox_show_in_game =
        dynamic_cast<Checkbox *>(window->getElementById("show-in-game"));
    if (select_header_location)
        select_header_location->variable = &header_location;
    else
        printf(
            "WARNING: WMWindow::openSettingsModal: header-location == NULL\n");
    if (checkbox_show_in_game)
        checkbox_show_in_game->setVariable(should_render_in_game);
    else
        printf("WARNING: WMWindow::openSettingsModal: show-in-game == NULL\n");
    window->move(Menu::instance->mouseX, Menu::instance->mouseY);
    Menu::instance->addModalObject(std::move(window));
}

bool WMWindow::shouldDrawHeader()
{
    return location != HeaderLocation::HIDDEN;
}

void WMWindow::reorderElements()
{
    moveObjects();
}

bool WMWindow::isHidden()
{
    return BaseMenuObject::isHidden() ||
           (Menu::instance->isInGame() && !should_render_in_game);
}

bool WMWindow::isFocused()
{
    return focused && !Menu::instance->isInGame();
}
} // namespace zerokernel