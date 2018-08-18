/*
  Created on 06.07.18.
*/

#pragma once

#include <menu/object/container/Container.hpp>
#include <settings/Bool.hpp>
#include <menu/wm/WindowHeader.hpp>

namespace zerokernel
{

class WindowContainer;

class WMWindow : public Container
{
public:
    enum class HeaderLocation
    {
        TOP,
        BOTTOM,
        HIDDEN
    };

    ~WMWindow() override = default;

    explicit WMWindow(WindowContainer &container);

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void reorderElements() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    void handleMessage(Message &msg, bool is_relayed) override;

    bool isHidden() override;

    bool isFocused();

    // Functions

    void moveObjects();

    void openSettingsModal();

    bool shouldDrawHeader();

    // WM functions

    void wmCloseWindow();
    void wmOpenWindow();
    void wmFocusGain();
    void wmFocusLose();

    void requestFocus();
    void requestClose();

    // WM info

    WindowContainer &container;
    std::string name{};
    std::string short_name{};
    bool focused{ false };

    HeaderLocation location{ HeaderLocation::TOP };
    settings::Variable<int> header_location{};
    settings::Variable<bool> should_render_in_game{ false };

    // Sub-elements

    WindowHeader *header{ nullptr };
    Container *contents{ nullptr };
};
} // namespace zerokernel