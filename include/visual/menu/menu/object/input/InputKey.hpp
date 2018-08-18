/*
  Created by Jenny White on 01.05.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <glez/draw.hpp>
#include <settings/Settings.hpp>
#include <menu/Menu.hpp>

namespace zerokernel
{

// TODO ValueChange
class InputKey : public BaseMenuObject
{
public:
    ~InputKey() override = default;

    InputKey();

    explicit InputKey(settings::Variable<settings::Key> &key);

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void onMove() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    Text text{};
    settings::Variable<settings::Key> *key{};
    bool capturing{ false };
};
} // namespace zerokernel