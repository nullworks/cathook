/*
  Created on 08.07.18.
*/

#include <menu/object/container/Table.hpp>
#include <menu/Menu.hpp>

extern settings::RVariable<rgba_t> color_border;

void zerokernel::Table::render()
{
    Container::render();

    // Main border
    renderBorder(*color_border);
    // Vertical separators
    int acc{ 1 };
    for (size_t i = 0; i < columns.size() - 1; ++i)
    {
        draw::Line(bb.getBorderBox().left() + columns[i].width + acc, bb.getBorderBox().top(), 0, bb.getBorderBox().height, *color_border, 1);
        acc += columns[i].width + 1;
    }
    // Horizontal separators
    for (size_t i = 1; i < objects.size(); ++i)
    {
        draw::Line(bb.getBorderBox().left(), objects[i]->getBoundingBox().getBorderBox().top() - 1, bb.getBorderBox().width, 0, *color_border, 1);
    }
}

void zerokernel::Table::reorderElements()
{
    Container::reorderElements();

    int acc{ 1 };

    for (auto &o : objects)
    {
        o->move(1, acc);
        acc += o->getBoundingBox().getFullBox().height + 1;
    }
}

void zerokernel::Table::removeAllRowsExceptFirst()
{
    if (objects.size() > 1)
        objects.resize(1);
}

int zerokernel::Table::getColumnWidth(size_t columnId)
{
    if (columnId >= columns.size())
        return 0;
    return columns[columnId].width;
}

size_t zerokernel::Table::getColumnCount()
{
    return columns.size();
}

void zerokernel::Table::loadFromXml(const tinyxml2::XMLElement *data)
{
    const tinyxml2::XMLElement *column = data->FirstChildElement("Column");
    while (column)
    {
        Column c{};
        if (tinyxml2::XMLError::XML_SUCCESS == column->QueryIntAttribute("width", &c.width))
        {
            bb.border_box.width += c.width + 1;
            columns.push_back(c);
        }
        column = column->NextSiblingElement("Column");
    }

    Container::loadFromXml(data);
}

zerokernel::Table::Table() : Container()
{
    bb.height.mode = BoundingBox::SizeMode::Mode::CONTENT;
}
