/*
  Created on 27.07.18.
*/
#include <menu/object/input/InputKey.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>

extern settings::RVariable<rgba_t> color_border;

namespace zerokernel_inputkey
{
static settings::RVariable<int> default_width{ "zk.style.input.key.width", "60" };
static settings::RVariable<int> default_height{ "zk.style.input.key.height", "14" };

static settings::RVariable<rgba_t> color_background_capturing{ "zk.style.input.key.color.background.capturing", "38b28f88" };
} // namespace zerokernel_inputkey
zerokernel::InputKey::InputKey() : BaseMenuObject{}
{
    text.setParent(this);
    text.bb.width.setFill();
    text.bb.height.setFill();
    text.bb.setPadding(0, 0, 5, 0);
    bb.resize(*zerokernel_inputkey::default_width, *zerokernel_inputkey::default_height);
}

zerokernel::InputKey::InputKey(settings::Variable<settings::Key> &key) : BaseMenuObject{}, key(&key)
{
    text.setParent(this);
    text.bb.width.setFill();
    text.bb.height.setFill();
    text.bb.setPadding(0, 0, 5, 0);
    bb.resize(*zerokernel_inputkey::default_width, *zerokernel_inputkey::default_height);
}

bool zerokernel::InputKey::handleSdlEvent(SDL_Event *event)
{
    if (key)
    {
        if (capturing && event->type == SDL_KEYDOWN)
        {
            if (event->key.keysym.sym != SDLK_ESCAPE)
            {
                key->key(SDL_GetKeyFromScancode(event->key.keysym.scancode));
            }
            else
            {
                key->reset();
            }
            capturing = false;
            return true;
        }

        if (event->type == SDL_MOUSEBUTTONDOWN)
        {
            if (capturing)
            {
                key->mouse(event->button.button);
                capturing = false;
                return true;
            }
            if (isHovered())
            {
                capturing = true;
                return true;
            }
        }
    }

    return BaseMenuObject::handleSdlEvent(event);
}

void zerokernel::InputKey::render()
{
    if (key)
    {
        if (capturing)
            renderBackground(*zerokernel_inputkey::color_background_capturing);
        renderBorder(*color_border);
        if (capturing)
        {
            text.set("<...>");
        }
        else
            text.set(key->toString());
        text.render();
    }
    else
        renderBorder(*style::colors::error);

    BaseMenuObject::render();
}

void zerokernel::InputKey::onMove()
{
    BaseMenuObject::onMove();

    text.onParentMove();
}

void zerokernel::InputKey::loadFromXml(const tinyxml2::XMLElement *data)
{
    BaseMenuObject::loadFromXml(data);

    const char *target{ nullptr };
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("target", &target))
    {
        std::string str(target);
        auto opt = settings::Manager::instance().lookup(str);
        if (opt)
        {
            key = dynamic_cast<settings::Variable<settings::Key> *>(opt);
            zerokernel::special::SettingsManagerList::markVariable(target);
        }
    }
}
