/*
  Created by Jenny White on 12.05.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <settings/Settings.hpp>
#include <menu/object/container/Container.hpp>

namespace zerokernel
{

class ColorSelector : public BaseMenuObject
{
public:
    ~ColorSelector() override = default;

    ColorSelector();
    explicit ColorSelector(settings::Variable<glez::rgba> &variable);

    void render() override;

    bool onLeftMouseClick() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

protected:
    settings::Variable<glez::rgba> *variable{ nullptr };
};
} // namespace zerokernel