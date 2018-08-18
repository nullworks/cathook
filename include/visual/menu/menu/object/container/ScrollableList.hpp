/*
  Created on 27.07.18.
*/

#pragma once

#include "Container.hpp"

namespace zerokernel
{

class ScrollableList : public Container
{
public:
    ~ScrollableList() override = default;

    bool handleSdlEvent(SDL_Event *event) override;

    void reorderElements() override;

    void render() override;

    void update() override;

    void updateIsHovered() override;

    void renderDebugOverlay() override;

    //

    void scrollUp();

    void scrollDown();

    void updateScroll();

    //

    size_t start_index{ 0 };
    size_t end_index{ 0 };
};
} // namespace zerokernel