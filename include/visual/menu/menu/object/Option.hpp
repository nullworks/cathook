/*
  Created on 03.07.18.
*/

#pragma once

#include <glez/draw.hpp>
#include <menu/Menu.hpp>

namespace zerokernel
{

class Option : public BaseMenuObject
{
public:
    ~Option() override = default;

    Option(std::string name, std::string value);

    void render() override;

    void onMove() override;

    void recalculateSize() override;

    void recursiveSizeUpdate() override;

    // Style

    // textAlign

    // State

    Text text{};
    const std::string name;
    const std::string value;
};
} // namespace zerokernel