/*
  Created on 03.07.18.
*/

#pragma once

#include <menu/object/container/Container.hpp>

/*
 * "List":
 *  Vertical/Start/Stretch
 * "Tabs":
 *  Horizontal/-/Wrap/Fill
 */

namespace zerokernel
{

class SmartContentContainer : public Container
{
public:
    enum class Direction
    {
        HORIZONTAL,
        VERTICAL
    };
    enum class JustifyContent
    {
        START,
        END,
        CENTER
    };

public:
    ~SmartContentContainer() override = default;

    SmartContentContainer() : Container()
    {
    }

    void notifySize() override
    {
        Container::notifySize();
    }

    // Specific methods

    void reorderObjects()
    {
    }

    // Properties

    Direction direction{ Direction::HORIZONTAL };
};
} // namespace zerokernel