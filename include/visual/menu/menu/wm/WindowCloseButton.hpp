/*
  Created on 07.07.18.
*/

#pragma once

#include <glez/texture.hpp>
#include <settings/Registered.hpp>
#include <menu/BaseMenuObject.hpp>

namespace zerokernel
{

class WindowCloseButton : public BaseMenuObject
{
public:
    ~WindowCloseButton() override = default;

    WindowCloseButton();

    void render() override;

    bool onLeftMouseClick() override;
};
} // namespace zerokernel