/*
  Created on 02.07.18.
*/

#include <menu/object/container/ModalColorSelect.hpp>

static settings::RVariable<glez::rgba> color_border{
    "zk.style.color-preview.color.border", "079797"
};

namespace zerokernel
{

void ModalColorSelect::render()
{
    ModalContainer::render();

    if (preview_enabled)
    {
        glez::draw::rect(preview_x + bb.getContentBox().x,
                         preview_y + bb.getContentBox().y, preview_size,
                         preview_size, *option);
        glez::draw::rect_outline(preview_x + bb.getContentBox().x,
                                 preview_y + bb.getContentBox().y, preview_size,
                                 preview_size, *color_border, 1);
    }
}

ModalColorSelect::ModalColorSelect(settings::Variable<glez::rgba> &option)
    : option(option), modal(this)
{
    loadFromXml(Menu::instance->getPrefab("color-picker")->FirstChildElement());
}

void ModalColorSelect::loadFromXml(const tinyxml2::XMLElement *data)
{
    if (!data)
    {
        printf("ModalColorSelect::loadFromXml(NULL)\n");
        return;
    }

    Container::loadFromXml(data);

    auto preview = data->FirstChildElement("Preview");
    if (preview)
    {
        preview_enabled = true;
        preview->QueryIntAttribute("x", &preview_x);
        preview->QueryIntAttribute("y", &preview_y);
        preview->QueryIntAttribute("size", &preview_size);
    }

    auto component_names = { "red", "green", "blue", "alpha" };
    auto components      = { &red, &green, &blue, &alpha };

    for (auto i = 0; i < components.size(); ++i)
    {
        auto c = dynamic_cast<Slider<int> *>(
            getElementById(*(component_names.begin() + i)));
        if (c == nullptr)
        {
            printf("ModalColorSelect: component %s is NULL or not Slider!\n",
                   *(component_names.begin() + i));
            continue;
        }
        c->addMessageHandler(*this);
        c->option = *(components.begin() + i);
    }

    auto hex = dynamic_cast<StringInput *>(getElementById("hex"));
    if (hex)
        hex->setVariable(&option);
    else
        printf("ModalColorSelect: component hex is NULL or not StringInput!\n");

    updateSlidersFromColor();
}

void ModalColorSelect::updateColorFromSliders()
{
    glez::rgba rgba{};
    rgba.r = *red / 255.f;
    rgba.g = *green / 255.f;
    rgba.b = *blue / 255.f;
    rgba.a = *alpha / 255.f;
    option = rgba;
}

void ModalColorSelect::updateSlidersFromColor()
{
    auto rgba = *option;
    red       = int(rgba.r * 255.f);
    green     = int(rgba.g * 255.f);
    blue      = int(rgba.b * 255.f);
    alpha     = int(rgba.a * 255.f);
}

void ModalColorSelect::handleMessage(Message &msg, bool is_relayed)
{
    if (!is_relayed && msg.name == "ValueChange")
    {
        if (msg.sender->uid == hex_input_uid)
            updateSlidersFromColor();
        else
            updateColorFromSliders();
    }

    BaseMenuObject::handleMessage(msg, is_relayed);
}
} // namespace zerokernel