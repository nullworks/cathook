/*
  Created on 06.07.18.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>
#include <menu/object/container/Container.hpp>

namespace zerokernel
{

class WindowManager;
class WMWindow;

/*
 *  Shows all windows managed by WindowManager
 *  Shows hidden (closed) windows in different colors
 *  Used to open these windows
 */
class TaskBar : public Container
{
public:
    ~TaskBar() override = default;

    explicit TaskBar(WindowManager &wm);

    void reorderElements() override;

    bool isHidden() override;

    void render() override;

    //

    void addWindowButton(WMWindow &window);

    //

    WindowManager &wm;
};
} // namespace zerokernel