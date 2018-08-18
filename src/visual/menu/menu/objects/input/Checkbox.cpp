/*
  Created on 27.07.18.
*/

#include <menu/object/input/Checkbox.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>

static settings::RVariable<int> checkbox_size{ "zk.style.checkbox.size", "12" };
static settings::RVariable<glez::rgba> color_border{
    "zk.style.checkbox.color.border", "079797"
};
static settings::RVariable<glez::rgba> color_checked{
    "zk.style.checkbox.color.checked", "079797"
};
static settings::RVariable<glez::rgba> color_hover{
    "zk.style.checkbox.color.hover", "07979777"
};

bool zerokernel::Checkbox::onLeftMouseClick()
{
    if (option)
        option->flip();

    BaseMenuObject::onLeftMouseClick();
    return true;
}

zerokernel::Checkbox::Checkbox() : BaseMenuObject{}
{
    bb.resize(*checkbox_size, *checkbox_size);
    bb.setPadding(3, 3, 3, 3);
}

zerokernel::Checkbox::Checkbox(settings::Variable<bool> &option)
    : option(&option)
{
    bb.resize(*checkbox_size, *checkbox_size);
    bb.setPadding(3, 3, 3, 3);
}

void zerokernel::Checkbox::render()
{
    if (option)
    {
        renderBorder(*color_border);
        auto cb = bb.getContentBox();
        if (**option)
            glez::draw::rect(cb.x, cb.y, cb.width, cb.height, *color_checked);
        else if (isHovered())
            glez::draw::rect(cb.x, cb.y, cb.width, cb.height, *color_hover);
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
