/*
  Created on 27.07.18.
*/

#include <menu/object/input/Checkbox.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>

extern settings::RVariable<rgba_t> color_border;

namespace zerokernel_checkbox
{
static settings::RVariable<int> checkbox_size{ "zk.style.checkbox.size", "12" };
static settings::RVariable<rgba_t> color_hover{ "zk.style.checkbox.color.hover", "00a098ff" };
} // namespace zerokernel_checkbox

bool zerokernel::Checkbox::onLeftMouseClick()
{
    if (option)
        option->flip();

    BaseMenuObject::onLeftMouseClick();
    return true;
}

zerokernel::Checkbox::Checkbox() : BaseMenuObject{}
{
    bb.resize(*zerokernel_checkbox::checkbox_size, *zerokernel_checkbox::checkbox_size);
    bb.setPadding(3, 3, 3, 3);
}

zerokernel::Checkbox::Checkbox(settings::Variable<bool> &option) : option(&option)
{
    bb.resize(*zerokernel_checkbox::checkbox_size, *zerokernel_checkbox::checkbox_size);
    bb.setPadding(3, 3, 3, 3);
}

void zerokernel::Checkbox::render()
{
    if (nullptr != option)
    {
        renderBorder(*color_border);
        auto cb = bb.getContentBox();
        if (**option)
            draw::Rectangle(cb.x, cb.y, cb.width, cb.height, *color_border);
        else if (isHovered())
            draw::Rectangle(cb.x, cb.y, cb.width, cb.height, *zerokernel_checkbox::color_hover);
    }
    else
    {
        renderBorder(*style::colors::error);
    }
}

void zerokernel::Checkbox::setVariable(settings::Variable<bool> &variable)
{
    option = &variable;
}

void zerokernel::Checkbox::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);

    const char *target{ nullptr };
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("target", &target))
    {
        std::string str(target);
        auto opt = settings::Manager::instance().lookup(str);
        if (opt)
        {
            option = dynamic_cast<settings::Variable<bool> *>(opt);
            zerokernel::special::SettingsManagerList::markVariable(target);
        }
    }
}
