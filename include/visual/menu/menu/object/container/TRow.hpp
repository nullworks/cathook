/*
  Created on 24.06.18.
*/

#pragma once

#include <menu/object/container/Table.hpp>

namespace zerokernel
{

class TRow : public Container
{
public:
    TRow();

    ~TRow() override = default;

    void reorderElements() override;

    void setParent(BaseMenuObject *parent) override;

    //

    //

    Table *table;
};
} // namespace zerokernel