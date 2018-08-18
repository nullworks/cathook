/*
  Created by Jenny White on 01.05.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <settings/Settings.hpp>
#include <menu/Menu.hpp>
#include <menu/Message.hpp>
#include <menu/BaseMenuObject.hpp>
#include <menu/object/ModalSpinner.hpp>

namespace zerokernel
{

// FIXME .cpp
class SliderStyle
{
public:
    static settings::RVariable<int> default_width;
    static settings::RVariable<int> default_height;

    static settings::RVariable<int> handle_width;
    static settings::RVariable<int> bar_width;

    static settings::RVariable<glez::rgba> handle_body;
    static settings::RVariable<glez::rgba> handle_border;
    static settings::RVariable<glez::rgba> bar_color;
};

template <typename T> class Slider : public BaseMenuObject
{
public:
    ~Slider() override = default;

    Slider() : BaseMenuObject{}
    {
        bb.resize(*SliderStyle::default_width, *SliderStyle::default_height);
    }

    explicit Slider(settings::ArithmeticVariable<T> &option)
        : BaseMenuObject{}, option(&option)
    {
        bb.resize(*SliderStyle::default_width, *SliderStyle::default_height);
    }

    bool handleSdlEvent(SDL_Event *event) override
    {
        if (option)
        {
            if (event->type == SDL_MOUSEBUTTONDOWN)
            {
                if (isHovered() && !modal)
                {
                    if (event->button.button == SDL_BUTTON_RIGHT)
                    {
                        openModalSpinner();
                        return true;
                    }
                    updateValue(vertical ? event->button.y : event->button.x);
                    if (event->button.button == SDL_BUTTON_LEFT)
                    {
                        grabbed = true;
                        return true;
                    }
                }
            }
            else if (event->type == SDL_MOUSEBUTTONUP)
                grabbed = false;
        }

        return BaseMenuObject::handleSdlEvent(event);
    }

    void render() override
    {
        if (modal)
            return;

        if (vertical)
        {
            // TODO
        }
        else
        {
            // Bar
            glez::draw::rect(
                bb.getBorderBox().left() + *SliderStyle::handle_width / 2,
                bb.getBorderBox().top() +
                    (bb.getBorderBox().height - *SliderStyle::bar_width) / 2,
                bb.getBorderBox().width - *SliderStyle::handle_width,
                *SliderStyle::bar_width,
                option ? *SliderStyle::bar_color : *style::colors::error);
            // Handle body
            auto offset = handleOffset() * (bb.getBorderBox().width -
                                            *SliderStyle::handle_width);
            glez::draw::rect(
                bb.getBorderBox().left() + offset, bb.getBorderBox().top(),
                *SliderStyle::handle_width, bb.getBorderBox().height,
                *SliderStyle::handle_body);
            // Handle outline
            glez::draw::rect_outline(
                bb.getBorderBox().left() + offset, bb.getBorderBox().top(),
                *SliderStyle::handle_width, bb.getBorderBox().height,
                *SliderStyle::handle_border, 1);
        }

        BaseMenuObject::render();
    }

    void loadFromXml(const tinyxml2::XMLElement *data) override
    {
        BaseMenuObject::loadFromXml(data);

        const char *target{ nullptr };
        settings::IVariable *opt{ nullptr };
        if (tinyxml2::XML_SUCCESS ==
            data->QueryStringAttribute("target", &target))
        {
            std::string str(target);
            opt = settings::Manager::instance().lookup(str);
            if (opt)
            {
                zerokernel::special::SettingsManagerList::markVariable(target);
            }
        }

        if constexpr (std::is_same<int, T>::value)
        {
            data->QueryIntAttribute("min", &min);
            data->QueryIntAttribute("max", &max);
            data->QueryIntAttribute("step", &step);
            option = dynamic_cast<settings::Variable<int> *>(opt);
        }
        else if constexpr (std::is_same<float, T>::value)
        {
            data->QueryFloatAttribute("min", &min);
            data->QueryFloatAttribute("max", &max);
            data->QueryFloatAttribute("step", &step);
            option = dynamic_cast<settings::Variable<float> *>(opt);
        }
    }

    void update() override
    {
        if (modal)
            return;

        if (option && isHovered())
        {
            Menu::instance->showTooltip(option->toString());
        }

        if (!grabbed)
            return;

        updateValue(vertical ? Menu::instance->mouseY : Menu::instance->mouseX);
    }

    void updateValue(int mouse)
    {
        if (!option)
            return;

        auto mouse_offset = mouse - (vertical ? bb.getBorderBox().top()
                                              : bb.getBorderBox().left());
        auto bar_length =
            (vertical ? bb.getBorderBox().height : bb.getBorderBox().width) -
            *SliderStyle::handle_width;
        auto fraction = mouse_offset / float(bar_length);
        fraction      = std::clamp(fraction, 0.f, 1.f);
        T value       = (max - min) * fraction + min;
        if (step != 0)
            value -= utility::mod(value, step);
        if (value != **option)
        {
            option->set(value);
            emitValueChange();
        }
    }

    void emitValueChange()
    {
        Message msg{ "ValueChange" };
        emit(msg, false);
    }

    float handleOffset() const
    {
        if (!option)
            return 0.5f;

        T value = **option;
        if (value <= min)
            return 0.f;
        if (value >= max)
            return 1.f;
        // cast to float to actually return the fraction instead of just 0
        return (value - min) / float(max - min);
    }

    void handleMessage(Message &msg, bool is_relayed) override
    {
        if (!is_relayed && msg.name == "ModalClose")
        {
            modal   = nullptr;
            grabbed = false;
        }
        BaseMenuObject::handleMessage(msg, is_relayed);
    }

    void openModalSpinner()
    {
        if (!option)
            return;
        grabbed      = false;
        auto spinner = std::make_unique<ModalSpinner<T>>(*option);
        modal        = spinner.get();
        // TODO style this
        spinner->resize(bb.getBorderBox().width, bb.getBorderBox().height);
        spinner->move(bb.getBorderBox().left(), bb.getBorderBox().top());
        spinner->addMessageHandler(*this);
        Menu::instance->addModalObject(std::move(spinner));
    }

    // Properties

    T min{ 0 };
    T max{ 100 };
    T step{ 0 };
    bool draw_text{ true };
    // TODO
    bool vertical{ false };

    // State

    bool grabbed{ false };
    ModalSpinner<T> *modal{ nullptr };

    settings::ArithmeticVariable<T> *option{ nullptr };
};
} // namespace zerokernel