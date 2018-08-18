/*
  Created by Jenny White on 01.05.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

#include <menu/object/container/Container.hpp>
#include <menu/object/container/TabSelection.hpp>

namespace zerokernel
{

class TabContainer : public BaseMenuObject
{
public:
    ~TabContainer() override = default;

    TabContainer();

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void update() override;

    void updateIsHovered() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    void emitSizeUpdate() override;

    void recalculateSize() override;

    void recursiveSizeUpdate() override;

    void onMove() override;

    void renderDebugOverlay() override;

    BaseMenuObject *
    findElement(const std::function<bool(BaseMenuObject *)> &search) override;

    //

    Container *getActiveContainer();

    void addTab(std::string title);

    Container *getTab(std::string title);

    TabSelection selection;
    std::vector<std::unique_ptr<Container>> containers{};
};
} // namespace zerokernel