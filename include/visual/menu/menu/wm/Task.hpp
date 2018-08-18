/*
  Created on 06.07.18.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>
#include <menu/object/Text.hpp>

namespace zerokernel
{

class WMWindow;

/*
 *  Shows or hides a window when pressed on
 *  Managed by TaskBar/WM
 */
class Task : public BaseMenuObject
{
public:
    explicit Task(WMWindow &window);

    //

    ~Task() override = default;

    void render() override;

    bool onLeftMouseClick() override;

    void recalculateSize() override;

    void onMove() override;

    void recursiveSizeUpdate() override;

    void emitSizeUpdate() override;

    //

    WMWindow &window;
    Text text{};
};
} // namespace zerokernel