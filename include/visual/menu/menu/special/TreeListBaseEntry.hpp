/*
  Created on 26.07.18.
*/

#pragma once

#include <menu/BaseMenuObject.hpp>

namespace zerokernel
{

class TreeListBaseEntry : public BaseMenuObject
{
public:
    TreeListBaseEntry();

    ~TreeListBaseEntry() override = default;

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    //

    int getRenderOffset();

    virtual void onCollapse();

    virtual void onRestore();

    virtual void setDepth(int depth);

    //

    size_t depth{ 0 };
};
} // namespace zerokernel