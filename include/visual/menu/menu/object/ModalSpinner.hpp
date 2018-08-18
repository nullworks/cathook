/*
  Created on 02.07.18.
*/

#pragma once

#include <settings/Settings.hpp>
#include <zconf.h>
#include <menu/ModalBehavior.hpp>

#include <menu/object/input/Spinner.hpp>

namespace zerokernel
{

template <typename T> class ModalSpinner : public Spinner<T>
{
public:
    ~ModalSpinner() override = default;

    explicit ModalSpinner(settings::ArithmeticVariable<T> &option)
        : Spinner<T>(option), modal(this)
    {
        this->startInput();
    }

    bool handleSdlEvent(SDL_Event *event) override
    {
        if (modal.shouldCloseOnEvent(event))
        {
            printf("Modal spinner should close\n");
            finishInput(false);
            return true;
        }
        return Spinner<T>::handleSdlEvent(event);
    }

    void finishInput(bool accept) override
    {
        TextInput::finishInput(accept);
        modal.close();
    }

    ModalBehavior modal;
};
} // namespace zerokernel