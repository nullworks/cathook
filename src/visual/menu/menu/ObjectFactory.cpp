/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <iostream>
#include <menu/ObjectFactory.hpp>
#include <menu/object/input/Checkbox.hpp>
#include <menu/object/input/ColorSelector.hpp>
#include <menu/object/input/Select.hpp>
#include <menu/object/input/Slider.hpp>
#include <menu/object/input/InputKey.hpp>
#include <menu/object/input/StringInput.hpp>
#include <menu/object/container/Box.hpp>
#include <menu/object/container/TabContainer.hpp>
#include <menu/object/container/LabeledObject.hpp>
#include <menu/object/container/Box.hpp>
#include <menu/object/container/Table.hpp>
#include <menu/object/container/TData.hpp>
#include <menu/object/container/ScrollableList.hpp>
#include <menu/object/container/ModalContainer.hpp>

namespace zerokernel
{

std::unique_ptr<BaseMenuObject> ObjectFactory::createObjectFromXml(const tinyxml2::XMLElement *element)
{
    if (element == nullptr)
        return nullptr;

    std::unique_ptr<BaseMenuObject> result{ nullptr };
    std::string type = element->Name();

    if (type == "TabContainer")
        result = std::make_unique<TabContainer>();
    else if (type == "Container")
        result = std::make_unique<Container>();
    else if (type == "ModalContainer")
        result = std::make_unique<ModalContainer>();
    else if (type == "Box")
        result = std::make_unique<Box>();
    else if (type == "AutoVariable")
        result = createAutoVariable(element);
    else if (type == "Select")
        result = std::make_unique<Select>();
    else if (type == "Table")
        result = std::make_unique<Table>();
    else if (type == "TRow")
        result = std::make_unique<TRow>();
    else if (type == "TData")
        result = std::make_unique<TData>();
    else if (type == "Text")
        result = std::make_unique<Text>();
    else if (type == "List")
        result = std::make_unique<List>();
    else if (type == "ScrollableList")
        result = std::make_unique<ScrollableList>();
    else if (type == "Checkbox")
        result = std::make_unique<Checkbox>();
    else if (type == "ColorSelector")
        result = std::make_unique<ColorSelector>();
    else if (type == "LabeledObject")
        result = std::make_unique<LabeledObject>();
    else if (type == "Slider")
    {
        const char *type{ nullptr };
        if (tinyxml2::XML_SUCCESS == element->QueryStringAttribute("type", &type))
        {
            if (EXIT_SUCCESS == strcmp("int", type))
                result = std::make_unique<Slider<int>>();
            else if (EXIT_SUCCESS == strcmp("float", type))
                result = std::make_unique<Slider<float>>();
        }
    }
    else if (type == "StringInput")
        result = std::make_unique<StringInput>();

    if (result.get() != nullptr)
    {
        result->loadFromXml(element);
        return result;
    }

    std::cout << "WARNING: Could not create object of type: " << type << "\n";
    return nullptr;
}

std::unique_ptr<BaseMenuObject> ObjectFactory::createAutoVariable(const tinyxml2::XMLElement *element)
{
    const char *name;
    if (element->QueryStringAttribute("target", &name))
        return nullptr;

    auto var = settings::Manager::instance().lookup(name);

    if (!var)
    {
        printf("Could not find settings '%s'\n", name);
        return nullptr;
    }

    special::SettingsManagerList::markVariable(name);

    std::unique_ptr<BaseMenuObject> control{ nullptr };

    switch (var->getType())
    {
    case settings::VariableType::BOOL:
    {
        // Create a checkbox
        auto obj = dynamic_cast<settings::Variable<bool> *>(var);
        if (obj == nullptr)
        {
            printf("error: could not cast settings '%s' to bool\n", name);
            return nullptr;
        }
        control = std::make_unique<Checkbox>(*obj);
        break;
    }
    case settings::VariableType::INT:
    {
        // Create a slider if there is min/max
        // Create a spinner otherwise
        auto obj = dynamic_cast<settings::Variable<int> *>(var);
        if (obj == nullptr)
        {
            printf("error: could not cast settings '%s' to int\n", name);
            return nullptr;
        }
        int min, max;
        if (!element->QueryIntAttribute("min", &min) && !element->QueryIntAttribute("max", &max))
        {
            // Make a slider
            auto slider = std::make_unique<Slider<int>>(*obj);
            slider->min = min;
            slider->max = max;
            element->QueryIntAttribute("step", &slider->step);
            control = std::move(slider);
        }
        else
        {
            // Make a spinner
            control = std::make_unique<Spinner<int>>(*obj);
        }
        break;
    }
    case settings::VariableType::FLOAT:
    {
        // Same as int
        auto obj = dynamic_cast<settings::Variable<float> *>(var);
        if (obj == nullptr)
        {
            printf("error: could not cast settings '%s' to float\n", name);
            return nullptr;
        }
        float min, max;
        if (!element->QueryFloatAttribute("min", &min) && !element->QueryFloatAttribute("max", &max))
        {
            // Make a slider
            auto slider = std::make_unique<Slider<float>>(*obj);
            slider->min = min;
            slider->max = max;
            element->QueryFloatAttribute("step", &slider->step);
            control = std::move(slider);
        }
        else
        {
            // Make a spinner
            control = std::make_unique<Spinner<float>>(*obj);
        }
        break;
    }
    case settings::VariableType::COLOR:
    {
        // Create a color selector
        auto obj = dynamic_cast<settings::Variable<rgba_t> *>(var);
        if (obj == nullptr)
        {
            printf("error: could not cast settings '%s' to rgba\n", name);
            return nullptr;
        }
        control = std::make_unique<ColorSelector>(*obj);
        break;
    }
    case settings::VariableType::KEY:
    {
        // Create a key input
        auto obj = dynamic_cast<settings::Variable<settings::Key> *>(var);
        if (obj == nullptr)
        {
            printf("error: could not cast settings '%s' to key\n", name);
            return nullptr;
        }
        control = std::make_unique<InputKey>(*obj);
        break;
    }
    case settings::VariableType::STRING:
    {
        // Create a StringInput
        auto obj = dynamic_cast<settings::Variable<std::string> *>(var);
        if (obj == nullptr)
        {
            printf("error: could not cast settings '%s' to string\n", name);
            return nullptr;
        }
        control = std::make_unique<StringInput>(*obj);
        break;
    }
    }

    if (control == nullptr)
        return nullptr;

    auto result = std::make_unique<LabeledObject>();
    result->setObject(std::move(control));
    return result;
}

std::unique_ptr<BaseMenuObject> ObjectFactory::createFromPrefab(const std::string &prefab_name)
{
    auto prefab = Menu::instance->getPrefab(prefab_name);
    if (prefab)
        return createObjectFromXml(prefab->FirstChildElement());
    return nullptr;
}
} // namespace zerokernel
