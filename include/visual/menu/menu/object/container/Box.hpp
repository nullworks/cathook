/*
  Created on 07.07.18.
*/

#pragma once

#include <menu/object/container/Container.hpp>
#include <menu/object/Text.hpp>

/*
 * A box is a container for single element: with padding, border and title.
 */

namespace zerokernel
{

class Box : public Container
{
public:
    Box();

    ~Box() override = default;

    void render() override;

    void onMove() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    //

    void setTitle(const std::string &string);

    //

    bool show_title{ false };

    Text title{};
};
} // namespace zerokernel