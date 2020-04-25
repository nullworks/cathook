/*
  Created on 4.8.20.
*/

#include <menu/special/ConfigsManagerList.hpp>
#include <settings/Manager.hpp>
#include <sstream>
#include <menu/special/VariableListEntry.hpp>
#include <menu/special/TreeListCollapsible.hpp>
#include <menu/menu/special/ConfigsManagerList.hpp>

#include <dirent.h> 
#include <stdio.h> 

zerokernel::special::ConfigsManagerList::ConfigsManagerList(zerokernel::Container &list) : list(list)
{
}

void zerokernel::special::ConfigsManagerList::construct()
{
	removeVariables();	

    DIR           *dirp;
    struct dirent *directory;

    dirp = opendir(paths::getConfigPath().c_str());
    if (dirp)
    {
        while ((directory = readdir(dirp)) != NULL)
        {
            if (std::string(directory->d_name) != "." && std::string(directory->d_name) != "..")
            {
                auto name      = explodeVariableName(directory->d_name);
                TreeNode *node = &root;
                for (auto &n : name)
                {
                    node = &((*node)[n]);
                }
                node->full_name = directory->d_name;
                addVariable(std::string (directory->d_name), true);
            }
        }

        closedir(dirp);
    }

    list.onMove();
    list.recursiveSizeUpdate();
    list.reorder_needed = true;
}

std::vector<std::string> zerokernel::special::ConfigsManagerList::explodeVariableName(const std::string &name)
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

zerokernel::special::ConfigsManagerList::TreeNode &zerokernel::special::ConfigsManagerList::TreeNode::operator[](const std::string &path)
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

void zerokernel::special::ConfigsManagerList::addVariable(std::string name, bool registered)
{
    auto entry = std::make_unique<VariableListEntry>();
    entry->setText(name);
    if (registered)
        entry->markPresentInUi();
    list.addObject(std::move(entry));
}

void zerokernel::special::ConfigsManagerList::removeVariables()
{
    list.reset();
}
