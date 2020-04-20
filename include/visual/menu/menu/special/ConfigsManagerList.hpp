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
        TreeNode &operator[](const std::string &path);
    };

    explicit ConfigsManagerList(Container &list);

    std::vector<std::string> explodeVariableName(const std::string &name);

    void recursiveWork(TreeNode &node, size_t depth);

    void construct();

    void addVariable(std::string name, bool registered);

    void removeVariables();


    //

    TreeNode root{};
    Container &list;
};
} // namespace zerokernel::special