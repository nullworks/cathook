/*
  Created by Jenny White on 01.05.18.
  Copyright (c) 2020 nullworks. All rights reserved.
*/

#pragma once

#include <menu/object/container/Container.hpp>
#include <menu/Utility.hpp>
#include <string>

namespace zerokernel
{
class Text;
class Tooltip : public Container
{
    Text *text{ nullptr };

public:
    Tooltip();

    void createText();

    void render() override;
    void setText(std::string text);

    std::string lastText;
    bool shown{ false };
};
} // namespace zerokernel
