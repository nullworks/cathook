#include <menu/BaseMenuObject.hpp>
#include <menu/ModalBehavior.hpp>
#include <menu/Menu.hpp>
#include <menu/Message.hpp>

/*
  Created on 26.07.18.
*/

zerokernel::ModalBehavior::ModalBehavior(zerokernel::BaseMenuObject *object)
    : object(object)
{
}

bool zerokernel::ModalBehavior::shouldCloseOnEvent(SDL_Event *event)
{
    if (close_on_outside_click && event->type == SDL_MOUSEBUTTONDOWN &&
        !object->isHovered())
        return true;
    return close_on_escape && event->type == SDL_KEYDOWN &&
           event->key.keysym.scancode == SDL_SCANCODE_ESCAPE;
}

void zerokernel::ModalBehavior::close()
{
    if (closing_state)
        return;
    closing_state = true;
    object->markForDelete();
    printf("Requesting close: %p\n", this);
    Message msg{ "ModalClose" };
    object->emit(msg, false);
}
