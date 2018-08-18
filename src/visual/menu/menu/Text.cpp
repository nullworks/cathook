#include <menu/BaseMenuObject.hpp>
#include <menu/object/Text.hpp>
#include <menu/Menu.hpp>
#include <glez/draw.hpp>

/*
  Created on 08.07.18.
*/

void zerokernel::Text::render()
{
    glez::draw::outlined_string(bb.getContentBox().left() + text_x,
                                bb.getContentBox().top() + text_y, data, *font,
                                *color_text, *color_outline, nullptr, nullptr);

    BaseMenuObject::render();
}

void zerokernel::Text::set(std::string text)
{
    if (text == data)
        return;
    data   = std::move(text);
    int ow = bb.border_box.width;
    int oh = bb.border_box.height;
    recalculateSize();
    calculate();
    if (ow != bb.border_box.width || oh != bb.border_box.height)
        BaseMenuObject::emitSizeUpdate();
}

const std::string &zerokernel::Text::get() const
{
    return data;
}

void zerokernel::Text::calculate()
{
    switch (align_x)
    {
    case HAlign::LEFT:
        text_x = 0;
        break;
    case HAlign::CENTER:
        text_x = (bb.getContentBox().width - text_size_x) / 2;
        break;
    case HAlign::RIGHT:
        text_x = bb.getContentBox().width - text_size_x;
        break;
    }
    switch (align_y)
    {
    case VAlign::TOP:
        text_y = 0;
        break;
    case VAlign::CENTER:
        text_y = (bb.getContentBox().height - int(font->size)) / 2;
        break;
    case VAlign::BOTTOM:
        text_y = bb.getContentBox().height - text_size_y;
        break;
    }
}

zerokernel::Text::Text()
{
    bb.width.setContent();
    bb.height.setContent();
    font          = &resource::font::base;
    color_text    = &*style::colors::text;
    color_outline = &*style::colors::text_shadow;
}

void zerokernel::Text::recalculateSize()
{
    BaseMenuObject::recalculateSize();

    float w, h;
    font->stringSize(data, &w, &h);
    text_size_x = int(w);
    text_size_y = int(h);

    if (bb.width.mode == BoundingBox::SizeMode::Mode::CONTENT)
    {
        bb.resizeContent(int(w), -1);
    }
    if (bb.height.mode == BoundingBox::SizeMode::Mode::CONTENT)
    {
        bb.resizeContent(-1, int(h));
    }
}

void zerokernel::Text::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);
    set(data->GetText());
}

void zerokernel::Text::setColorText(const glez::rgba *color)
{
    color_text = color;
}

void zerokernel::Text::emitSizeUpdate()
{
    calculate();

    BaseMenuObject::emitSizeUpdate();
}

void zerokernel::Text::setOwnColor(glez::rgba color)
{
    own_color  = color;
    color_text = &own_color;
}
