/*
  Created on 26.07.18.
*/

#pragma once

#include <menu/object/container/List.hpp>
#include <settings/Manager.hpp>
#include <optional>

namespace zerokernel::special
{

class SettingsManagerList
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

    explicit SettingsManagerList(Container &list);

    std::vector<std::string> explodeVariableName(const std::string &name);

    void recursiveWork(TreeNode &node, size_t depth);

    void construct();

    void addCollapsible(std::string name, size_t depth);

    void addVariable(std::string name, size_t depth,
                     settings::IVariable *variable, bool registered);

    //

    static void markVariable(std::string name);

    static bool isVariableMarked(std::string name);

    static void resetMarks();

    //

    TreeNode root{};
    Container &list;
};
} // namespace zerokernel::special