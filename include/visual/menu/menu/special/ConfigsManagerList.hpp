/*
  Created on 8.4.20.
*/

#pragma once

#include <menu/object/container/List.hpp>
#include <settings/Manager.hpp>
#include <optional>

namespace zerokernel::special
{

class ConfigsManagerList
{
public:
    class TreeNode;

    class TreeNode
    {
    public:
        settings::IVariable *variable{ nullptr };
        std::string full_name{};
        std::vector<std::pair<std::string, TreeNode>> nodes{};
    };

    explicit ConfigsManagerList(Container &list);

    void construct();

    void addVariable(std::string name);

    TreeNode root{};
    Container &list;
};
} // namespace zerokernel::special
