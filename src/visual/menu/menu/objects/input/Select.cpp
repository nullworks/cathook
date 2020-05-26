/*
  Created on 27.07.18.
*/

#include <menu/object/input/Select.hpp>
#include <menu/Message.hpp>
#include <menu/Debug.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>

extern settings::RVariable<rgba_t> color_border;

namespace zerokernel_select
{
static settings::RVariable<int> default_width{ "zk.style.input.select.width", "60" };
static settings::RVariable<int> default_height{ "zk.style.input.select.height", "14" };
} // namespace zerokernel_select

bool zerokernel::Select::onLeftMouseClick()
{
    openModal();
    BaseMenuObject::onLeftMouseClick();
    return true;
}

void zerokernel::Select::render()
{
    if (variable)
    {
        option *found{ nullptr };
        for (auto &p : options)
        {
            if (p.value == variable->toString())
            {
                found = &p;
                break;
            }
        }

        renderBorder(*color_border);
        std::string t;
        if (found)
        {
            float x, y;
            t = found->name;
            resource::font::base.stringSize(t, &x, &y);
            if (x + 5 > bb.getBorderBox().width)
            {
                while (true)
                {
                    resource::font::base.stringSize(t, &x, &y);
                    if (x + 13 > bb.getBorderBox().width)
                    {
                        t = t.substr(0, t.size() - 1);
                    }
                    else
                        break;
                }
                t.append("..");
            }
        }
        text.set(found ? t : "<?>");
        text.render();
    }
    else
        renderBorder(*style::colors::error);

    BaseMenuObject::render();
}

zerokernel::Select::Select(settings::IVariable &variable) : BaseMenuObject(), variable(&variable)
{
    resize(*zerokernel_select::default_width, *zerokernel_select::default_height);
    text.setParent(this);
    text.bb.width.setFill();
    text.bb.height.setFill();
    text.bb.setPadding(0, 0, 5, 0);
}

zerokernel::Select::Select() : BaseMenuObject{}
{
    resize(*zerokernel_select::default_width, *zerokernel_select::default_height);
    text.setParent(this);
    text.bb.width.setFill();
    text.bb.height.setFill();
    text.bb.setPadding(0, 0, 5, 0);
}

void zerokernel::Select::openModal()
{
    auto object = std::make_unique<ModalSelect>();
    object->setParent(Menu::instance->wm.get());
    object->move(bb.getBorderBox().x, bb.getBorderBox().y);
    object->addMessageHandler(*this);
    int size = bb.getBorderBox().width;
    for (auto &p : options)
    {
        object->addOption(p.name, p.value, p.tooltip);
        float x, y;
        resource::font::base.stringSize(p.name, &x, &y);
        x += 10;
        if (x > size)
            size = x;
    }
    object->resize(size, -1);
    debug::UiTreeGraph(object.get(), 0);
    Menu::instance->addModalObject(std::move(object));
}

void zerokernel::Select::handleMessage(zerokernel::Message &msg, bool is_relayed)
{
    if (variable && !is_relayed && msg.name == "OptionSelected")
        variable->fromString((std::string) msg.kv["value"]);

    BaseMenuObject::handleMessage(msg, is_relayed);
}

void zerokernel::Select::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);

    const char *target;
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("target", &target))
    {
        auto var = settings::Manager::instance().lookup(target);
        if (!var)
        {
            printf("WARNING: Creating Select element: could not find settings "
                   "'%s'\n",
                   target);
        }
        else
        {
            zerokernel::special::SettingsManagerList::markVariable(target);
            variable = var;
        }
    }

    auto child = data->FirstChildElement(nullptr);
    while (child != nullptr)
    {
        if (!strcmp("Option", child->Name()))
        {
            const char *name;
            const char *value;
            const char *tooltip;
            if (child->QueryStringAttribute("name", &name) || child->QueryStringAttribute("value", &value))
                continue;
            printf("adding %s: %s\n", name, value);
            auto has_tooltip = !(child->QueryStringAttribute("tooltip", &tooltip));
            options.push_back(option{ name, value, has_tooltip ? std::optional<std::string>(tooltip) : std::nullopt });
        }
        child = child->NextSiblingElement(nullptr);
    }
}

void zerokernel::Select::onMove()
{
    BaseMenuObject::onMove();

    text.onParentMove();
}
