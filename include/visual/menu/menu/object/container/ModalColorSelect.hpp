/*
  Created on 02.07.18.
*/

#pragma once

#include <settings/Int.hpp>
#include <menu/object/input/Slider.hpp>
#include <menu/object/input/Spinner.hpp>
#include <menu/object/input/StringInput.hpp>
#include <menu/ModalBehavior.hpp>
#include <menu/object/container/ModalContainer.hpp>

namespace zerokernel
{

class ModalColorSelect : public ModalContainer
{
public:
    ~ModalColorSelect() override = default;

    explicit ModalColorSelect(settings::Variable<glez::rgba> &option);

    void render() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    void handleMessage(Message &msg, bool is_relayed) override;

    //

    void updateSlidersFromColor();

    void updateColorFromSliders();

    //

    bool preview_enabled{ false };
    int preview_x{ 0 };
    int preview_y{ 0 };
    int preview_size{ 48 };

    // State

    settings::Variable<int> red{};
    settings::Variable<int> green{};
    settings::Variable<int> blue{};
    settings::Variable<int> alpha{};

    settings::Variable<glez::rgba> &option;
    size_t hex_input_uid{ 0 };

    ModalBehavior modal;
};
} // namespace zerokernel