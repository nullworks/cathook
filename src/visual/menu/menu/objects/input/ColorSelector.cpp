/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/object/input/ColorSelector.hpp>
#include <menu/Menu.hpp>
#include <menu/object/container/ModalColorSelect.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>

extern settings::RVariable<rgba_t> color_border;

namespace zerokernel_colorselector
{
static settings::RVariable<int> default_width{ "zk.style.input.color.width", "36" };
static settings::RVariable<int> default_height{ "zk.style.input.color.height", "14" };
} // namespace zerokernel_colorselector

namespace zerokernel
{

ColorSelector::ColorSelector() : BaseMenuObject{}
{
    resize(*zerokernel_colorselector::default_width, *zerokernel_colorselector::default_height);
    bb.setPadding(3, 3, 3, 3);
}

ColorSelector::ColorSelector(settings::Variable<rgba_t> &variable) : BaseMenuObject{}, variable(&variable)
{
    resize(*zerokernel_colorselector::default_width, *zerokernel_colorselector::default_height);
    bb.setPadding(3, 3, 3, 3);
}

void ColorSelector::render()
{
    renderBorder(variable ? *color_border : *style::colors::error);
    if (variable)
    {
        auto pb = bb.getContentBox();
        draw::Rectangle(pb.left(), pb.top(), pb.width, pb.height, **variable);
    }
}

bool ColorSelector::onLeftMouseClick()
{
    if (!variable)
        return false;
    auto modal = std::make_unique<ModalColorSelect>(*variable);
    modal->setParent(Menu::instance->wm.get());
    modal->move(bb.getBorderBox().right(), bb.getBorderBox().y);
    Menu::instance->addModalObject(std::move(modal));
    BaseMenuObject::onLeftMouseClick();
    return true;
}

void ColorSelector::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);

    const char *target{ nullptr };
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("target", &target))
    {
        std::string str(target);
        auto opt = settings::Manager::instance().lookup(str);
        if (opt)
        {
            variable = dynamic_cast<settings::Variable<rgba_t> *>(opt);
            zerokernel::special::SettingsManagerList::markVariable(target);
        }
    }
}
} // namespace zerokernel
