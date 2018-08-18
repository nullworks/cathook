/*
  Created on 24.06.18.
*/

#pragma once

#include <menu/object/container/Container.hpp>

namespace zerokernel
{

class Table : public Container
{
public:
    class Column
    {
    public:
        int width;
    };

    Table();

    ~Table() override = default;

    void render() override;

    void reorderElements() override;

    void loadFromXml(const tinyxml2::XMLElement *data) override;

    //

    void removeAllRowsExceptFirst();

    int getColumnWidth(size_t columnId);

    size_t getColumnCount();

    //

    std::vector<Column> columns{};
};
} // namespace zerokernel