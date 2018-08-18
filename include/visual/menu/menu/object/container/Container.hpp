/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <memory>
#include <vector>
#include <menu/BaseMenuObject.hpp>
#include <glez/glez.hpp>
#include <algorithm>
#include <functional>

namespace zerokernel
{

class Container : public virtual BaseMenuObject
{
public:
    ~Container() override = default;

    Container() = default;

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void emitSizeUpdate() override;

    void update() override;

    void updateIsHovered() override;

    void recursiveSizeUpdate() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    BaseMenuObject *
    findElement(const std::function<bool(BaseMenuObject *)> &search) override;

    virtual void addObject(std::unique_ptr<BaseMenuObject> &&object);

    void cleanupObjects();

    void fillFromXml(const tinyxml2::XMLElement *element);

    void onMove() override;

    void onChildMove() override;

    void recalculateSize() override;

    void renderDebugOverlay() override;

    //

    virtual void reset();

    virtual void reorderElements();

    virtual void iterateObjects(std::function<void(BaseMenuObject *)> callback);

public:
    bool reorder_needed{ true };
    std::vector<std::unique_ptr<BaseMenuObject>> objects{};
};
} // namespace zerokernel