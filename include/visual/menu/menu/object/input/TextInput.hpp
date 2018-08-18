/*
  Created on 02.07.18.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>
#include <glez/draw.hpp>
#include <menu/Menu.hpp>
#include <menu/Message.hpp>

namespace zerokernel
{

class TextInput : public BaseMenuObject
{
public:
    ~TextInput() override = default;

    TextInput();

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void onMove() override;

    // TextInput-specific methods

    void startInput();

    virtual void finishInput(bool accept);

    virtual const std::string &getValue();

    virtual void setValue(std::string value);

    void emitValueChange();

    // Properties

    size_t max_length{ std::numeric_limits<size_t>::max() };
    bool draw_border{ true };

    // State

    Text text_object{};
    bool is_input_active{};
    std::string current_text{};
    std::string text{};
};
} // namespace zerokernel