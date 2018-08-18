/*
  Created on 08.07.18.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>
#include <glez/font.hpp>

namespace zerokernel
{

class Text : public BaseMenuObject
{
public:
    enum class VAlign
    {
        TOP,
        CENTER,
        BOTTOM
    };
    enum class HAlign
    {
        LEFT,
        CENTER,
        RIGHT
    };

    //

    ~Text() override = default;

    Text();

    void render() override;

    void recalculateSize() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    void emitSizeUpdate() override;

    //

    void setColorText(const glez::rgba *color);

    void setOwnColor(glez::rgba color);

    void set(std::string text);

    const std::string &get() const;

    void calculate();

    int text_size_x;
    int text_size_y;

    int text_x{};
    int text_y{};

    // Properties

    HAlign align_x{ HAlign::LEFT };
    VAlign align_y{ VAlign::CENTER };

    glez::font *font{ nullptr };
    const glez::rgba *color_text{ nullptr };
    const glez::rgba *color_outline{ nullptr };

    glez::rgba own_color{};

protected:
    std::string data{};
};
} // namespace zerokernel