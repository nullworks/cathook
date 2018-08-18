/*
  Created by Jenny White on 01.05.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <menu/object/container/TabContainer.hpp>
#include <string>
#include <glez/glez.hpp>

namespace zerokernel
{

class TabSelection;

class TabButton : public BaseMenuObject
{
public:
    ~TabButton() override = default;

    TabButton(TabSelection &parent, size_t id);

    void render() override;

    void recalculateSize() override;

    bool onLeftMouseClick() override;

    void onMove() override;

    void recursiveSizeUpdate() override;

protected:
    Text text{};
    TabSelection &parent;
    const size_t id;
};
} // namespace zerokernel