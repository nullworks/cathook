#include <SDL2/SDL_events.h>
#include <menu/BaseMenuObject.hpp>
#include <menu/tinyxml2.hpp>
#include <menu/object/container/ModalContainer.hpp>
#include <menu/ModalBehavior.hpp>
#include <menu/Menu.hpp>

/*
  Created on 26.07.18.
*/

extern settings::RVariable<rgba_t> color_border;
extern settings::RVariable<rgba_t> color_background;

bool zerokernel::ModalContainer::handleSdlEvent(SDL_Event *event)
{
    if (modal.shouldCloseOnEvent(event))
        modal.close();

    return Container::handleSdlEvent(event);
}

void zerokernel::ModalContainer::render()
{
    renderBackground(*color_background);
    renderBorder(*color_border);

    Container::render();
}

zerokernel::ModalContainer::ModalContainer() : modal(this)
{
}
