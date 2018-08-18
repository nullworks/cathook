/*
  Created on 03.07.18.
*/

#pragma once

#include <menu/object/Option.hpp>
#include <menu/ModalBehavior.hpp>
#include <menu/object/container/List.hpp>
#include "ModalContainer.hpp"

namespace zerokernel
{

class ModalSelect : public ModalContainer
{
public:
    ~ModalSelect() override = default;

    ModalSelect();

    void handleMessage(Message &msg, bool is_relayed) override;

    void reorderElements() override;

    //

    void addOption(std::string name, std::string value,
                   std::optional<std::string> tooltip);
};
} // namespace zerokernel