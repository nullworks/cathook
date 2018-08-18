/*
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <SDL2/SDL_events.h>
#include <menu/BaseMenuObject.hpp>
#include <menu/tinyxml2.hpp>
#include <menu/object/container/Box.hpp>
#include <menu/ObjectFactory.hpp>
#include <glez/draw.hpp>
#include <menu/Menu.hpp>

static settings::RVariable<glez::rgba> color_border{
    "zk.style.box.color.border", "079797"
};

void zerokernel::Box::render()
{
    // Render the frame
    if (show_title)
    {
        int hl = bb.padding.left / 2;
        int hr = bb.padding.right / 2;
        int ht = bb.padding.top / 2;
        int hb = bb.padding.bottom / 2;

        int lhl = bb.getBorderBox().left() + hl;
        int tht = bb.getBorderBox().top() + ht;

        // Top left
        glez::draw::line(lhl, tht, hl, 0, *color_border, 1);
        // Top right
        int title_width = title.getBoundingBox().getFullBox().width;
        glez::draw::line(
            bb.getBorderBox().left() + bb.padding.left + title_width, tht,
            bb.getBorderBox().width - bb.padding.left - title_width - hr, 0,
            *color_border, 1);
        // Left
        glez::draw::line(lhl, tht, 0, bb.getBorderBox().height - ht - hb,
                         *color_border, 1);
        // Bottom
        glez::draw::line(lhl, bb.getBorderBox().bottom() - hb,
                         bb.getBorderBox().width - hl - hr, 0, *color_border,
                         1);
        // Right
        glez::draw::line(bb.getBorderBox().right() - hr, tht, 0,
                         bb.getBorderBox().height - ht - hb, *color_border, 1);
    }
    else
    {
        int hl = bb.padding.left / 2;
        int hr = bb.padding.right / 2;
        int ht = bb.padding.top / 2;
        int hb = bb.padding.bottom / 2;

        glez::draw::rect_outline(
            bb.getBorderBox().left() + hl, bb.getBorderBox().top() + ht,
            bb.getBorderBox().width - hl - hr,
            bb.getBorderBox().height - ht - hb, *color_border, 1);
    }

    // Render the title
    if (show_title)
        title.render();

    // Render the child elements
    Container::render();
}

void zerokernel::Box::setTitle(const std::string &string)
{
    show_title = true;
    title.set(string);
}

zerokernel::Box::Box() : Container{}
{
    title.setParent(this);
    title.getBoundingBox().setFloating(true);
}

void zerokernel::Box::onMove()
{
    title.move(0, -bb.padding.top);

    Container::onMove();
}

void zerokernel::Box::loadFromXml(const tinyxml2::XMLElement *data)
{
    Container::loadFromXml(data);

    const char *title{ nullptr };
    if (tinyxml2::XML_SUCCESS == data->QueryStringAttribute("name", &title))
    {
        setTitle(title);
    }
}
