/*
  Copyright (c) 2020 nullworks. All rights reserved.
*/

#include <menu/BaseMenuObject.hpp>
#include <menu/Menu.hpp>
#include <iostream>
#include <menu/Message.hpp>
#include <sstream>
#include <settings/Settings.hpp>
#include <drawing.hpp>

namespace zerokernel
{

std::size_t BaseMenuObject::objectCount{ 0 };

static settings::RVariable<bool> render_objects{ "zk.debug.object-overlay", "false" };
static settings::RVariable<bool> render_objects_nonfocus{ "zk.debug.overlay-all", "false" };

bool BaseMenuObject::handleSdlEvent(SDL_Event *event)
{
    if (!isHidden())
    {
        if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT)
        {
            if (isHovered())
                if (onLeftMouseClick())
                    return true;
        }
    }
    return false;
}

void BaseMenuObject::render()
{
    float w, h;
    font = &resource::font::base;
    
    font->stringSize(label, &w, &h);
    int length = int(w);
    if (tooltip.has_value() && isHoveredText(length))
        Menu::instance->showTooltip(*tooltip);
}

void BaseMenuObject::update()
{
}

void BaseMenuObject::setParent(BaseMenuObject *parent)
{
    this->parent = parent;
}

void BaseMenuObject::move(int x, int y)
{
    xOffset = x;
    yOffset = y;
    updateLocation();
    if (parent)
        parent->onChildMove();
}

void BaseMenuObject::updateLocation()
{
    if (parent)
    {
        bb.move(xOffset + parent->getBoundingBox().getContentBox().left(), yOffset + parent->getBoundingBox().getContentBox().top());
    }
    else
    {
        bb.move(xOffset, yOffset);
    }
    onMove();
}

bool BaseMenuObject::isHovered()
{
    if (Menu::instance->frame != hovered_frame)
        return false;

    int mx{ 0 };
    int my{ 0 };

    if (Menu::instance)
    {
        mx = Menu::instance->mouseX;
        my = Menu::instance->mouseY;
    }

    return bb.contains(mx, my);
}

bool BaseMenuObject::isHoveredText(int text_length)
{
    if (Menu::instance->frame != hovered_frame)
        return false;

    int mx{ 0 };
    int my{ 0 };

    if (Menu::instance)
    {
        mx = Menu::instance->mouseX;
        my = Menu::instance->mouseY;
    }

    return bb.containsForTooltip(mx, my, text_length);
}

void BaseMenuObject::handleMessage(Message &msg, bool is_relayed)
{
    emit(msg, true);
}

void BaseMenuObject::addMessageHandler(IMessageHandler &handler)
{
    handlers.push_back(&handler);
}

void BaseMenuObject::emit(Message &msg, bool is_relayed)
{
    printf("%p: emitting %s\n", this, msg.name.c_str());
    if (!msg.sender)
        msg.sender = this;
    for (auto &handler : handlers)
    {
        handler->handleMessage(msg, is_relayed);
    }
}

void BaseMenuObject::loadFromXml(const tinyxml2::XMLElement *data)
{
    data->QueryIntAttribute("x", &xOffset);
    data->QueryIntAttribute("y", &yOffset);

    const char *str;

    data->QueryIntAttribute("min-width", &bb.width.min);
    data->QueryIntAttribute("max-width", &bb.width.max);
    data->QueryIntAttribute("min-height", &bb.height.min);
    data->QueryIntAttribute("max-height", &bb.height.max);

    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("padding", &str))
    {
        std::istringstream stream{ str };
        stream >> bb.padding.top >> bb.padding.right >> bb.padding.bottom >> bb.padding.left;
    }
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("margin", &str))
    {
        std::istringstream stream{ str };
        stream >> bb.margin.top >> bb.margin.right >> bb.margin.bottom >> bb.margin.left;
    }
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("width", &str))
    {
        if (EXIT_SUCCESS == strcmp("content", str))
            bb.width.mode = BoundingBox::SizeMode::Mode::CONTENT;
        else if (EXIT_SUCCESS == strcmp("fill", str))
            bb.width.mode = BoundingBox::SizeMode::Mode::FILL;
        else
        {
            bb.width.setFixed();
            std::istringstream stream{ str };
            int width;
            stream >> width;
            bb.resize(width, -1);
        }
    }
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("height", &str))
    {
        if (EXIT_SUCCESS == strcmp("content", str))
            bb.height.mode = BoundingBox::SizeMode::Mode::CONTENT;
        else if (EXIT_SUCCESS == strcmp("fill", str))
            bb.height.mode = BoundingBox::SizeMode::Mode::FILL;
        else
        {
            bb.height.setFixed();
            std::istringstream stream{ str };
            int height;
            stream >> height;
            bb.resize(-1, height);
        }
    }

    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("tooltip", &str))
        tooltip = str;

    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("label", &str))
        label = str;
    
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("id", &str))
        string_id = str;

    auto attribute = data->FirstAttribute();
    while (attribute)
    {
        kv[attribute->Name()].set(attribute->Value());
        attribute = attribute->Next();
    }

    emitSizeUpdate();
    onMove();
}

bool BaseMenuObject::isHidden()
{
    return hidden;
}

void BaseMenuObject::hide()
{
    hidden = true;
}

void BaseMenuObject::show()
{
    hidden = false;
}

size_t BaseMenuObject::object_sequence_number{ 0 };

void BaseMenuObject::updateIsHovered()
{
    hovered_frame = Menu::instance->frame;
}

bool BaseMenuObject::containsMouse()
{
    return bb.contains(Menu::instance->mouseX, Menu::instance->mouseY);
}

void BaseMenuObject::renderBackground(rgba_t color)
{
    draw::Rectangle(bb.getBorderBox().x, bb.getBorderBox().y, bb.getBorderBox().width, bb.getBorderBox().height, color);
}

void BaseMenuObject::renderBorder(rgba_t color)
{
    draw::RectangleOutlined(bb.getBorderBox().x, bb.getBorderBox().y, bb.getBorderBox().width, bb.getBorderBox().height, color, 1.0f);
}

void BaseMenuObject::onParentSizeUpdate()
{
    bb.onParentSizeUpdate();
}

void BaseMenuObject::onChildSizeUpdate()
{
    bb.onChildSizeUpdate();
}

void BaseMenuObject::emitSizeUpdate()
{
    if (parent)
        parent->onChildSizeUpdate();

    // Container classes should call onParentSizeUpdate() on all children
}

void BaseMenuObject::recalculateSize()
{
    bb.updateFillSize();
}

void BaseMenuObject::recursiveSizeUpdate()
{
    auto ow = bb.border_box.width;
    auto oh = bb.border_box.height;
    this->recalculateSize();
    if (ow != bb.border_box.width || oh != bb.border_box.height)
        emitSizeUpdate();

    // Container classes should call recursiveSizeUpdate() on all children
}

void BaseMenuObject::markForDelete()
{
    markedForDelete = true;
}

BaseMenuObject *BaseMenuObject::findElement(const std::function<bool(BaseMenuObject *)> &search)
{
    if (search(this))
        return this;
    return nullptr;
}

BaseMenuObject *BaseMenuObject::getElementById(const std::string &id)
{
    return findElement([id](BaseMenuObject *object) { return object->string_id == id; });
}

BoundingBox &BaseMenuObject::getBoundingBox()
{
    return bb;
}

void BaseMenuObject::resize(int width, int height)
{
    if (bb.resize(width, height))
        emitSizeUpdate();
}

void BaseMenuObject::onChildMove()
{
}

void BaseMenuObject::onParentMove()
{
    updateLocation();
}

void BaseMenuObject::renderDebugOverlay()
{
    if (render_objects && (render_objects_nonfocus || isHovered()))
    {
        int t = int(this);
        t *= 13737373 + 487128758242;
        draw::Rectangle(bb.getBorderBox().x, bb.getBorderBox().y, bb.getBorderBox().width, bb.getBorderBox().height, rgba_t(t & 0xFF, (t >> 8) & 0xFF, (t >> 16) & 0xFF, 80));
    }
    // Container classes should call renderDebugOverlay() on children
}

void BaseMenuObject::onMove()
{
}

bool BaseMenuObject::onLeftMouseClick()
{
    Message msg{ "LeftClick" };
    emit(msg, false);
    return false;
}
} // namespace zerokernel
