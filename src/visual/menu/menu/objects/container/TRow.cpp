#include <menu/object/container/Container.hpp>
#include <menu/object/container/TRow.hpp>
#include <menu/object/container/TData.hpp>

/*
  Created on 08.07.18.
*/

void zerokernel::TRow::reorderElements()
{
    Container::reorderElements();

    int acc{ 0 };
    int max_h{ 0 };
    size_t i{ 0 };
    for (auto &o : objects)
    {
        o->move(acc, 0);
        auto h = o->bb.getFullBox().height;
        if (h > max_h)
            max_h = h;
        acc += table->getColumnWidth(i) + 1;
        ++i;
    }
}

void zerokernel::TRow::setParent(zerokernel::BaseMenuObject *parent)
{
    Container::setParent(parent);

    printf("TRow::setParent\n");

    table = dynamic_cast<Table *>(parent);

    if (!table)
        printf("WARNING: TRow::setParent is not a table\n");
}

zerokernel::TRow::TRow() : Container{}
{
    bb.width.mode  = BoundingBox::SizeMode::Mode::FILL;
    bb.height.mode = BoundingBox::SizeMode::Mode::CONTENT;
}
