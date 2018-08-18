/*
  Created on 26.07.18.
*/

#pragma once

#include <menu/object/container/Container.hpp>
#include <menu/ModalBehavior.hpp>

namespace zerokernel
{

class ModalContainer : public Container
{
public:
    ModalContainer();

    ~ModalContainer() override = default;

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    ModalBehavior modal;
};
} // namespace zerokernel