/*
  Created on 26.07.18.
*/

#include <menu/special/SettingsManagerList.hpp>
#include <settings/Manager.hpp>
#include <sstream>
#include <menu/special/VariableListEntry.hpp>
#include <menu/special/TreeListCollapsible.hpp>
#include <menu/menu/special/SettingsManagerList.hpp>

zerokernel::special::SettingsManagerList::SettingsManagerList(zerokernel::Container &list) : list(list)
{
}

void zerokernel::special::SettingsManagerList::construct()
{
    using pair_type = std::pair<std::string, settings::IVariable *>;
    std::vector<pair_type> all_registered{};
    for (auto &v : settings::Manager::instance().registered)
    {
        all_registered.emplace_back(std::make_pair(v.first, &v.second.variable));
    }

    std::sort(all_registered.begin(), all_registered.end(), [](const pair_type &a, const pair_type &b) -> bool { return a.first.compare(b.first) < 0; });

    for (auto &v : all_registered)
    {
        auto name      = explodeVariableName(v.first);
        TreeNode *node = &root;
        for (auto &n : name)
        {
            node = &((*node)[n]);
        }
        node->full_name = v.first;
        node->variable  = v.second;
    }

    recursiveWork(root, 0);
    list.onMove();
    list.recursiveSizeUpdate();
    list.reorder_needed = true;
}

void zerokernel::special::SettingsManagerList::recursiveWork(zerokernel::special::SettingsManagerList::TreeNode &node, size_t depth)
{
    for (auto &n : node.nodes)
    {
        if (n.second.variable)
        {
            addVariable(n.first, depth, n.second.variable, isVariableMarked(n.second.full_name));
        }

        if (!n.second.nodes.empty())
        {
            addCollapsible(n.first, depth);
            recursiveWork(n.second, depth + 1);
        }
    }
}

std::vector<std::string> zerokernel::special::SettingsManagerList::explodeVariableName(const std::string &name)
{
    std::vector<std::string> result{};
    std::ostringstream ss{};

    for (auto &c : name)
    {
        if (c == '.')
        {
            result.push_back(ss.str());
            ss.str("");
        }
        else
            ss << c;
    }
    result.push_back(ss.str());

    return result;
}

zerokernel::special::SettingsManagerList::TreeNode &zerokernel::special::SettingsManagerList::TreeNode::operator[](const std::string &path)
{
    for (auto &v : nodes)
    {
        if (v.first == path)
            return v.second;
    }
    nodes.emplace_back();
    nodes.back().first = path;
    return nodes.back().second;
}

void zerokernel::special::SettingsManagerList::addVariable(std::string name, size_t depth, settings::IVariable *variable, bool registered)
{
    auto entry = std::make_unique<VariableListEntry>();
    entry->setText(name);
    entry->setVariable(variable);
    entry->setDepth(depth);
    if (registered)
        entry->markPresentInUi();
    list.addObject(std::move(entry));
}

void zerokernel::special::SettingsManagerList::addCollapsible(std::string name, size_t depth)
{
    auto entry = std::make_unique<TreeListCollapsible>();
    entry->setText(name);
    entry->setDepth(depth);
    list.addObject(std::move(entry));
}

static boost::unordered_flat_map<std::string, bool> marks{};

void zerokernel::special::SettingsManagerList::markVariable(std::string name)
{
    marks[name] = true;
}

void zerokernel::special::SettingsManagerList::resetMarks()
{
    marks.clear();
}

bool zerokernel::special::SettingsManagerList::isVariableMarked(std::string name)
{
    return marks.find(name) != marks.end();
}
