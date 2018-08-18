/*
  Created on 26.07.18.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>

namespace zerokernel
{

class ModalBehavior
{
public:
    explicit ModalBehavior(BaseMenuObject *object);

    bool shouldCloseOnEvent(SDL_Event *event);

    void close();

    bool close_on_escape{ true };
    bool close_on_outside_click{ true };

    bool closing_state{ false };

    BaseMenuObject *object;
};
} // namespace zerokernel