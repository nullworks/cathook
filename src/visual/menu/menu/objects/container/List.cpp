/*
  Created on 07.07.18.
*/

#include <menu/object/container/List.hpp>

zerokernel::List::List() : Container{}
{
    bb.height.mode = BoundingBox::SizeMode::Mode::CONTENT;
}

void zerokernel::List::reorderElements()
{
    int total{ 0 };
    bool first{ false };
    for (auto &object : objects)
    {
        if (object->isHidden())
            continue;
        if (!first)
            total += interval;
        object->move(0, total);
        total += object->getBoundingBox().getFullBox().height;
        first = false;
    }
}
