/*
  Created on 27.07.18.
*/
#include <menu/Debug.hpp>
#include <menu/object/container/Container.hpp>
#include <menu/wm/WindowContainer.hpp>
#include <menu/object/container/TabContainer.hpp>

zerokernel::debug::UiTreeGraph::UiTreeGraph(zerokernel::BaseMenuObject *object,
                                            int depth)
    : depth(depth)
{
    for (int i = 0; i < depth; ++i)
        printf("  ");
    printf("[%u] %s\n", object->uid, typeid(*object).name());
    auto bb = object->getBoundingBox().getBorderBox();
    for (int i = 0; i < depth; ++i)
        printf("  ");
    printf("> {%d, %d; %d x %d; offsets %d, %d}\n", bb.x, bb.y, bb.width,
           bb.height, object->xOffset, object->yOffset);
    for (int i = 0; i < depth; ++i)
        printf("  ");
    printf("> WidthMode{%d}, HeightMode{%d}\n",
           object->getBoundingBox().width.mode,
           object->getBoundingBox().height.mode);
    auto container = dynamic_cast<zerokernel::Container *>(object);
    if (container)
    {
        for (auto &o : container->objects)
        {
            auto graph = UiTreeGraph(o.get(), depth + 1);
        }
    }
    auto window_container = dynamic_cast<zerokernel::WindowContainer *>(object);
    if (window_container)
    {
        for (auto &o : window_container->windows)
        {
            auto graph = UiTreeGraph(o.get(), depth + 1);
        }
    }
    auto tab_container = dynamic_cast<zerokernel::TabContainer *>(object);
    if (tab_container)
    {
        auto g = UiTreeGraph(&tab_container->selection, depth + 1);
        for (auto &o : tab_container->containers)
        {
            auto graph = UiTreeGraph(o.get(), depth + 1);
        }
    }
}
