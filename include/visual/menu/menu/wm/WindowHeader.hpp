/*
  Created on 06.07.18.
*/

#pragma once

#include <menu/object/Text.hpp>
#include <menu/wm/WindowCloseButton.hpp>
#include <menu/object/container/Container.hpp>

namespace zerokernel
{

class WMWindow;

/*
 *  Shows window name
 *  Allows closing (hiding) windows
 *  Allows dragging windows around
 */
class WindowHeader : public Container
{
public:
    explicit WindowHeader(WMWindow &window);

    ~WindowHeader() override = default;

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void update() override;

    bool isHidden() override;

    void handleMessage(Message &msg, bool is_relayed) override;

    void reorderElements() override;

    //

    void updateTitle();

    //

    WMWindow &window;
    bool dragged{ false };
    WindowCloseButton *close{ nullptr };
    Text *title{ nullptr };
};
} // namespace zerokernel