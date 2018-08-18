/*
  Created on 27.07.18.
*/

#include <menu/object/input/StringInput.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>

static settings::RVariable<int> default_width{ "zk.style.input.string.width",
                                               "60" };
static settings::RVariable<int> default_height{ "zk.style.input.string.height",
                                                "14" };

void zerokernel::StringInput::setVariable(settings::IVariable *variable)
{
    option = variable;
}

zerokernel::StringInput::StringInput() : TextInput{}
{
    resize(*default_width, *default_height);
    text_object.bb.setPadding(0, 0, 5, 0);
}

zerokernel::StringInput::StringInput(settings::IVariable &option)
    : TextInput(), option(&option)
{
    resize(*default_width, *default_height);
    text_object.bb.setPadding(0, 0, 5, 0);
}

const std::string &zerokernel::StringInput::getValue()
{
    if (option)
        return option->toString();
    return utility::empty_string;
}

void zerokernel::StringInput::setValue(std::string value)
{
    if (option)
        option->fromString(value);
}

void zerokernel::StringInput::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);

    const char *target{ nullptr };
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("target", &target))
    {
        option = settings::Manager::instance().lookup(target);
        if (option)
        {
            zerokernel::special::SettingsManagerList::markVariable(target);
        }
    }
}
