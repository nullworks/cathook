/*
  Created on 08.07.18.
*/

#pragma once

#include <menu/object/container/Container.hpp>
#include <menu/object/container/TRow.hpp>

namespace zerokernel
{

class TData : public Container
{
public:
    TData();

    void setParent(BaseMenuObject *parent) override;

    //

    //

    TRow *row;
};
} // namespace zerokernel