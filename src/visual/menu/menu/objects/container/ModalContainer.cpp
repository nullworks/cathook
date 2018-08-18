#include <SDL2/SDL_events.h>
#include <menu/BaseMenuObject.hpp>
#include <menu/tinyxml2.hpp>
#include <menu/object/container/ModalContainer.hpp>
#include <menu/ModalBehavior.hpp>
#include <menu/Menu.hpp>

/*
  Created on 26.07.18.
*/

static settings::RVariable<glez::rgba> color_border{
    "zk.style.modal-container.color.border", "079797"
};
static settings::RVariable<glez::rgba> color_background{
    "zk.style.modal-container.color.background", "1d2f40"
};

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
