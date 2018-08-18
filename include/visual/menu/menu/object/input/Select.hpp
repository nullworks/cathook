/*
  Created on 03.07.18.
*/

#pragma once

#include <settings/Settings.hpp>
#include <map>
#include <optional>
#include <string>
#include <glez/draw.hpp>
#include <menu/Menu.hpp>
#include <menu/object/container/ModalSelect.hpp>

namespace zerokernel
{

class Select : public BaseMenuObject
{
public:
    struct option
    {
        std::string name;
        std::string value;
        std::optional<std::string> tooltip;
    };

    ~Select() override = default;

    Select();

    explicit Select(settings::IVariable &variable);

    void render() override;

    void onMove() override;

    // Specific

    void openModal();

    void handleMessage(Message &msg, bool is_relayed) override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    bool onLeftMouseClick() override;

    // State

    // value -> [name, tooltip]

    Text text{};
    std::vector<option> options{};
    settings::IVariable *variable{ nullptr };

protected:
};
} // namespace zerokernel