#include <menu/BaseMenuObject.hpp>
#include <menu/wm/WindowHeader.hpp>
#include <menu/wm/WMWindow.hpp>
#include <menu/Menu.hpp>
#include <menu/Message.hpp>
#include <glez/draw.hpp>

/*
  Created on 06.07.18.
*/

static settings::RVariable<glez::rgba> color_background{
    "zk.style.window-header.color.background.inactive", "00000000"
};
static settings::RVariable<glez::rgba> color_background_focused{
    "zk.style.window-header.color.background.active", "079797"
};
static settings::RVariable<glez::rgba> color_border{
    "zk.style.window-header.color.border.inactive", "079797"
};
static settings::RVariable<glez::rgba> color_border_focused{
    "zk.style.window-header.color.border.active", "079797"
};

zerokernel::WindowHeader::WindowHeader(WMWindow &window)
    : BaseMenuObject(), window(window), close()
{
    setParent(&window);

    bb.width.mode = BoundingBox::SizeMode::Mode::FILL;
    bb.height.setContent();

    auto close_button = std::make_unique<WindowCloseButton>();
    close             = close_button.get();
    close->addMessageHandler(*this);
    addObject(std::move(close_button));

    auto title  = std::make_unique<Text>();
    this->title = title.get();
    title->bb.setPadding(0, 0, 8, 8);
    title->bb.width.setContent();
    title->bb.height.setFill();
    printf("Title: %p\n", this->title);
    addObject(std::move(title));
}

bool zerokernel::WindowHeader::handleSdlEvent(SDL_Event *event)
{
    if (Container::handleSdlEvent(event))
        return true;

    if (isHovered() && event->type == SDL_MOUSEBUTTONDOWN)
    {
        dragged = true;
        return true;
    }
    if (event->type == SDL_MOUSEBUTTONUP)
    {
        dragged = false;
    }
    return false;
}

void zerokernel::WindowHeader::render()
{
    renderBackground(window.isFocused() ? *color_background_focused
                                        : *color_background);
    // glez::draw::line(bb.getBorderBox().left(), bb.getBorderBox().bottom() -
    // 1, bb.getBorderBox().width, 0, window.focused ? *color_border_focused :
    // *color_border, 1);
    renderBorder(window.isFocused() ? *color_border_focused : *color_border);

    Container::render();
}

void zerokernel::WindowHeader::update()
{
    Container::update();

    if (dragged)
    {
        window.move(window.xOffset + Menu::instance->dx,
                    window.yOffset + Menu::instance->dy);
    }
}

void zerokernel::WindowHeader::handleMessage(zerokernel::Message &msg,
                                             bool is_relayed)
{
    BaseMenuObject::handleMessage(msg, is_relayed);

    if (!is_relayed)
    {
        if (msg.name == "LeftClick")
        {
            window.requestClose();
        }
    }
}

void zerokernel::WindowHeader::updateTitle()
{
    title->set(window.name);
}

void zerokernel::WindowHeader::reorderElements()
{
    title->move(0, 0);
    close->move(bb.getContentBox().width -
                    close->getBoundingBox().getBorderBox().width,
                0);
}

bool zerokernel::WindowHeader::isHidden()
{
    return BaseMenuObject::isHidden() || !window.shouldDrawHeader();
}
