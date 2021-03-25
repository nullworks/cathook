#include <menu/BaseMenuObject.hpp>
#include <menu/object/Text.hpp>
#include <menu/Menu.hpp>
#include <drawing.hpp>

/*
  Created on 08.07.18.
*/

void zerokernel::Text::render()
{
#if ENABLE_IMGUI_DRAWING // needed, sorry
    draw::String(bb.getContentBox().left() + text_x, bb.getContentBox().top() + text_y - 1, *color_text, data.c_str(), resource::font::base);
#else
    draw::String(bb.getContentBox().left() + text_x, bb.getContentBox().top() + text_y + 1, *color_text, data.c_str(), resource::font::base);
#endif
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

bool zerokernel::Text::handleSdlEvent(SDL_Event *event)
{
    if (!isHidden())
    {
        if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT)
        {
            if (isHovered())
            {
                if (onLeftMouseClick())
                    return true;
            }
        }
    }
    return false;
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
    if (data.empty() || !font)
    {
        w = 0.0f;
        h = 0.0f;
    }
    else
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

void zerokernel::Text::setColorText(const rgba_t *color)
{
    color_text = color;
}

void zerokernel::Text::emitSizeUpdate()
{
    calculate();

    BaseMenuObject::emitSizeUpdate();
}

void zerokernel::Text::setOwnColor(rgba_t color)
{
    own_color  = color;
    color_text = &own_color;
}
