/*
  Created on 02.07.18.
*/

#pragma once

#include <menu/object/input/TextInput.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>

namespace zerokernel
{

class SpinnerStyle
{
public:
    static settings::RVariable<int> default_width;
    static settings::RVariable<int> default_height;
};

template <typename T> class Spinner : public TextInput
{
public:
    ~Spinner() override = default;

    Spinner() : TextInput{}
    {
        bb.resize(*SpinnerStyle::default_width, *SpinnerStyle::default_height);
    }

    explicit Spinner(settings::ArithmeticVariable<T> &option)
        : TextInput{}, option(&option)
    {
        bb.resize(*SpinnerStyle::default_width, *SpinnerStyle::default_height);
    }

    bool handleSdlEvent(SDL_Event *event) override
    {
        if (option && event->type == SDL_MOUSEWHEEL)
        {
            if (isHovered() && !is_input_active)
            {
                auto mod = SDL_GetModState();
                T change = step;
                if (mod & KMOD_SHIFT)
                    change *= 10;
                if (mod & KMOD_CTRL)
                    change /= 10;
                if (change == 0)
                    change = 1;
                if (event->wheel.y < 0)
                    change = -change;
                option->set(**option + change);
                emitValueChange();
                return true;
            }
        }
        return TextInput::handleSdlEvent(event);
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
            data->QueryIntAttribute("step", &step);
            option = dynamic_cast<settings::Variable<int> *>(opt);
        }
        else if constexpr (std::is_same<float, T>::value)
        {
            data->QueryFloatAttribute("step", &step);
            option = dynamic_cast<settings::Variable<float> *>(opt);
        }
    }

    const std::string &getValue() override
    {
        if (option)
            return option->toString();
        return utility::empty_string;
    }

    void setValue(std::string value) override
    {
        if (option)
        {
            option->fromString(value);
            emitValueChange();
        }
    }

    // Properties
    T min{ 0 };
    T max{ 100 };
    T step{ 1 };

    // Spinner-specific

    settings::ArithmeticVariable<T> *option{ nullptr };
};
} // namespace zerokernel