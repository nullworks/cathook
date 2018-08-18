/*
  Created on 27.07.18.
*/

#include <menu/object/Option.hpp>

static settings::RVariable<glez::rgba> color_hovered{
    "zk.style.option.color.hover", "ff00aa"
};

zerokernel::Option::Option(std::string name, std::string value)
    : BaseMenuObject{}, name(std::move(name)), value(std::move(value))
{
    bb.width.mode  = BoundingBox::SizeMode::Mode::FILL;
    bb.height.mode = BoundingBox::SizeMode::Mode::CONTENT;
    // FIXME font height
    bb.height.min = 12;
    bb.normalizeSize();
    text.setParent(this);
    text.bb.width.setFill();
    text.bb.setPadding(3, 3, 5, 0);
    text.set(this->name);
}

void zerokernel::Option::render()
{
    if (isHovered())
        renderBackground(*color_hovered);
    text.render();
}

void zerokernel::Option::onMove()
{
    BaseMenuObject::onMove();

    text.onParentMove();
}

void zerokernel::Option::recalculateSize()
{
    BaseMenuObject::recalculateSize();

    bb.shrinkContent();
    bb.extend(text.getBoundingBox());
    bb.normalizeSize();
}

void zerokernel::Option::recursiveSizeUpdate()
{
    BaseMenuObject::recursiveSizeUpdate();

    text.recursiveSizeUpdate();
}
