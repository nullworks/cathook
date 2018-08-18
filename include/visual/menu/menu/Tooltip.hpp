/*
  Created by Jenny White on 01.05.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>
#include <menu/Utility.hpp>
#include <string>
#include <menu/object/Text.hpp>

namespace zerokernel
{

class Tooltip : public BaseMenuObject
{
public:
    Tooltip();

    void render() override;

    void onMove() override;

    void setText(std::string text);

    Text text{};
    std::string lastText;
    bool shown{ false };
};
} // namespace zerokernel