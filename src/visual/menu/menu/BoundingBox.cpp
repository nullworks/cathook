/*
  Created on 27.07.18.
*/
#include <menu/BaseMenuObject.hpp>
#include <menu/BoundingBox.hpp>
#include <menu/Menu.hpp>

/*
 * When object changes size:
 *
 * Example: Text change
 * Structure: [setSize]F*Text -> [onChildSizeUpdate..extend..setSize]C*Label ->
 * [onChildSizeUpdate..extend..setSize]C*LabeledObject ->
 * [onChildSizeUpdate..extend..setSize]C*Window
 *
 * Example: Window resize
 * Structure: [setSize]C*Window ->
 * [onParentSizeUpdate..updateFillSize..setSize]F*Bar
 */

zerokernel::BoundingBox::BoundingBox(BaseMenuObject &object) : object(object)
{
}

zerokernel::BoundingBox::box zerokernel::BoundingBox::getContentBox() const
{
    box result{};
    result.x      = border_box.x + padding.left;
    result.y      = border_box.y + padding.top;
    result.width  = border_box.width - padding.left - padding.right;
    result.height = border_box.height - padding.top - padding.bottom;
    return result;
}

const zerokernel::BoundingBox::box &zerokernel::BoundingBox::getBorderBox() const
{
    return border_box;
}

zerokernel::BoundingBox::box zerokernel::BoundingBox::getFullBox() const
{
    box result{};
    result.x      = border_box.x - margin.left;
    result.y      = border_box.y - margin.top;
    result.width  = border_box.width + margin.left + margin.right;
    result.height = border_box.height + margin.top + margin.bottom;
    return result;
}

zerokernel::BoundingBox &zerokernel::BoundingBox::getParentBox()
{
    auto parent = object.parent;
    if (parent == nullptr)
        return Menu::instance->wmRootBoundingBox();
    return parent->getBoundingBox();
}

bool zerokernel::BoundingBox::move(int x, int y)
{
    bool changed{ false };
    if (x >= 0 && border_box.x != x)
    {
        border_box.x = x;
        changed      = true;
    }
    if (y >= 0 && border_box.y != y)
    {
        border_box.y = y;
        changed      = true;
    }
    return changed;
}

bool zerokernel::BoundingBox::resize(int width, int height)
{
    bool changed{ false };
    if (width != -1)
        width = this->width.clamp(width);
    if (height != -1)
        height = this->height.clamp(height);
    if (width >= 0 && width != border_box.width)
    {
        border_box.width = width;
        changed          = true;
    }
    if (height >= 0 && height != border_box.height)
    {
        border_box.height = height;
        changed           = true;
    }
    return changed;
}

bool zerokernel::BoundingBox::updateFillSize()
{
    bool changed = false;
    if (width.mode == SizeMode::Mode::FILL)
    {
        auto area = getParentBox().getContentBox();
        int max   = area.right() - border_box.x - margin.right;
        changed   = resize(max, -1);
    }
    if (height.mode == SizeMode::Mode::FILL)
    {
        auto area = getParentBox().getContentBox();
        int max   = area.bottom() - border_box.y - margin.bottom;
        changed   = resize(-1, max) || changed;
    }
    return changed;
}

bool zerokernel::BoundingBox::contains(int x, int y)
{
    return x >= border_box.left() && x <= border_box.right() && y >= border_box.top() && y <= border_box.bottom();
}

bool zerokernel::BoundingBox::containsForTooltip(int x, int y, int length)
{
    int right_end = border_box.left() + length;
    return x >= border_box.left() && x <= right_end && y >= border_box.top() && y <= border_box.bottom();
}

bool zerokernel::BoundingBox::extend(zerokernel::BoundingBox &box)
{
    if (box.isFloating())
        return false;

    auto other   = box.getBorderBox();
    bool changed = false;
    auto area    = getParentBox().getContentBox();
    if (width.mode != SizeMode::Mode::FIXED && box.width.mode != SizeMode::Mode::FILL)
    {
        // int req_w = other.right() - border_box.left() + padding.right;
        int req_w = padding.left + box.object.xOffset + other.width + box.margin.right + padding.right;
        int max   = area.right() - border_box.x - margin.right;
        req_w     = width.clamp(req_w);
        if (req_w > max && getParentBox().width.mode == SizeMode::Mode::FIXED)
            req_w = max;
        if (req_w > border_box.width)
        {
            changed = resize(req_w, -1);
        }
    }
    if (height.mode != SizeMode::Mode::FIXED && box.height.mode != SizeMode::Mode::FILL)
    {
        // int req_h = other.bottom() - border_box.top() + padding.bottom;
        int req_h = padding.top + box.object.yOffset + other.height + box.margin.bottom + padding.bottom;
        int max   = area.bottom() - border_box.y - margin.bottom;
        req_h     = height.clamp(req_h);
        if (req_h > max && getParentBox().height.mode == SizeMode::Mode::FIXED)
            req_h = max;
        if (req_h > border_box.height)
        {
            changed = resize(-1, req_h) || changed;
        }
    }

    return changed;
}

void zerokernel::BoundingBox::setPadding(int top, int bottom, int left, int right)
{
    padding.top    = top;
    padding.bottom = bottom;
    padding.left   = left;
    padding.right  = right;
}

void zerokernel::BoundingBox::setMargin(int top, int bottom, int left, int right)
{
    margin.top    = top;
    margin.bottom = bottom;
    margin.left   = left;
    margin.right  = right;
}

bool zerokernel::BoundingBox::normalizeSize()
{
    return resize(width.clamp(border_box.width), height.clamp(border_box.height));
}

bool zerokernel::BoundingBox::shrink()
{
    int w = border_box.width;
    int h = border_box.height;

    resize(0, 0);
    normalizeSize();

    return border_box.width != w || border_box.height != h;
}

void zerokernel::BoundingBox::onParentSizeUpdate()
{
    if (width.mode == SizeMode::Mode::FILL || height.mode == SizeMode::Mode::FILL)
    {
        if (updateFillSize())
            emitObjectSizeUpdate();
    }
}

void zerokernel::BoundingBox::emitObjectSizeUpdate()
{
    object.emitSizeUpdate();
}

void zerokernel::BoundingBox::onChildSizeUpdate()
{
    if (!(width.mode == SizeMode::Mode::FIXED && height.mode == SizeMode::Mode::FIXED))
    {
        auto ow = border_box.width;
        auto oh = border_box.height;
        object.recalculateSize();
        if (ow != border_box.width || oh != border_box.height)
            object.emitSizeUpdate();
    }
}

bool zerokernel::BoundingBox::shrinkContent()
{
    int w = border_box.width;
    int h = border_box.height;
    if (width.mode == SizeMode::Mode::CONTENT)
        resize(0, -1);
    if (height.mode == SizeMode::Mode::CONTENT)
        resize(-1, 0);

    return w != border_box.width || h != border_box.height;
}

bool zerokernel::BoundingBox::isFloating()
{
    return floating;
}

void zerokernel::BoundingBox::setFloating(bool flag)
{
    floating = flag;
}

bool zerokernel::BoundingBox::resizeContent(int width, int height)
{
    return resize(width >= 0 ? padding.left + width + padding.right : -1, height >= 0 ? padding.top + height + padding.bottom : -1);
}

void zerokernel::BoundingBox::SizeMode::setFixed()
{
    mode = SizeMode::Mode::FIXED;
}

void zerokernel::BoundingBox::SizeMode::setContent()
{
    mode = SizeMode::Mode::CONTENT;
}

void zerokernel::BoundingBox::SizeMode::setFill()
{
    mode = SizeMode::Mode::FILL;
}
