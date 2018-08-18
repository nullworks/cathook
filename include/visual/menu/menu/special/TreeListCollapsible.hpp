/*
  Created on 26.07.18.
*/

#pragma once

#include <menu/object/Text.hpp>
#include <menu/special/TreeListBaseEntry.hpp>

namespace zerokernel
{

class TreeListCollapsible : public TreeListBaseEntry
{
public:
    TreeListCollapsible();

    ~TreeListCollapsible() override = default;

    bool handleSdlEvent(SDL_Event *event) override;

    void render() override;

    void onMove() override;

    void recalculateSize() override;

    //

    void setDepth(int depth) override;

    void onCollapse() override;

    void onRestore() override;

    void listCollapse();

    void listUncollapse();

    void setText(std::string data);

    //

    bool is_collapsed{ false };

    Text text{};
    std::string name{};
};
} // namespace zerokernel