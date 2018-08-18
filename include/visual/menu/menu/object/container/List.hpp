/*
  Created by Jenny White on 30.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <menu/object/container/Container.hpp>

namespace zerokernel
{

class List : public virtual Container
{
public:
    ~List() override = default;

    List();

    void reorderElements() override;

    // Properties

    int interval{ 3 };
};
} // namespace zerokernel