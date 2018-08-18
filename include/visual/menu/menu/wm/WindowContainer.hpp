/*
  Created on 06.07.18.
*/

#pragma once

#include <menu/object/container/Container.hpp>
#include <menu/wm/WMWindow.hpp>

namespace zerokernel
{

class WindowManager;

/*
 *  Contains multiple windows
 *  Manages Z-Order of these windows
 *  Can bring a window to top
 */
class WindowContainer : public BaseMenuObject
{
public:
    ~WindowContainer() override = default;

    explicit WindowContainer(WindowManager &wm);

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void update() override;

    void recursiveSizeUpdate() override;

    void updateIsHovered() override;

    void emitSizeUpdate() override;

    void renderDebugOverlay() override;

    BaseMenuObject *
    findElement(const std::function<bool(BaseMenuObject *)> &search) override;

    void onMove() override;

    // Functions

    void moveWindowToTop(size_t uid);

    // WM Functions

    WMWindow &wmCreateWindow();
    WMWindow *wmFindWindow(size_t uid);
    WMWindow *wmGetActive();
    void wmRequestFocus(size_t uid);
    void wmResetFocus();

    // Sub-elements

    std::vector<std::unique_ptr<WMWindow>> windows{};
    std::vector<size_t> order{};

    WMWindow *active_window{ nullptr };
    size_t request_move_top{};

    WindowManager &wm;
};
} // namespace zerokernel