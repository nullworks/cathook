/*
  Created on 08.07.18.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>

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

    bool handleSdlEvent(SDL_Event *event) override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    void emitSizeUpdate() override;

    //

    void setColorText(const rgba_t *color);

    void setOwnColor(rgba_t color);

    void set(std::string text);
    
    void setWithLabel(std::string text1, std::string text2);

    const std::string &get() const;

    void calculate();

    int text_size_x;
    int text_size_y;

    int text_x{};
    int text_y{};

    // Properties

    HAlign align_x{ HAlign::LEFT };
    VAlign align_y{ VAlign::CENTER };

    fonts::font *font{ nullptr };
    const rgba_t *color_text{ nullptr };
    const rgba_t *color_outline{ nullptr };

    rgba_t own_color{};

protected:
    std::string data{};
};
} // namespace zerokernel
