/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <vector>
#include <string>
#include <menu/BaseMenuObject.hpp>
#include <menu/Menu.hpp>

namespace zerokernel
{

class TabContainer;

class TabSelection : public Container
{
public:
    explicit TabSelection(TabContainer &parent);

    void render() override;

    void add(const std::string &option);

    void reorderElements() override;

    //

    void selectTab(size_t id);

public:
    TabContainer &container;
    std::vector<std::string> options{};
    int offset{ 0 };
    size_t active{ 0 };
};
} // namespace zerokernel