/*
  Created on 02.07.18.
*/

#pragma once

#include <settings/Settings.hpp>
#include <menu/object/input/TextInput.hpp>

namespace zerokernel
{

class StringInput : public TextInput
{
public:
    ~StringInput() override = default;

    StringInput();

    explicit StringInput(settings::IVariable &option);

    const std::string &getValue() override;

    void setValue(std::string value) override;

    void setVariable(settings::IVariable *variable);

    void loadFromXml(const tinyxml2::XMLElement *data) override;

protected:
    settings::IVariable *option{ nullptr };
};
} // namespace zerokernel