/*
  Created on 2.15.21.
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
	list.reset();	

    DIR           *dirp;
    struct dirent *directory;

    dirp = opendir(paths::getConfigPath().c_str());
    if (dirp)
    {
        while ((directory = readdir(dirp)) != NULL)
        {
            std::string cfg_name = directory->d_name;
            
            std::string::size_type i = cfg_name.find(".conf");

            if (i != std::string::npos) // Remove .conf extension
                cfg_name.erase(i, 5);
            
            if (std::string(cfg_name) != "." && std::string(cfg_name) != "..") // Hardcoded to fix a bug
                addVariable(cfg_name);

        }

        closedir(dirp);
    }

    list.onMove();
    list.recursiveSizeUpdate();
    list.reorder_needed = true;
}

void zerokernel::special::ConfigsManagerList::addVariable(std::string name)
{
    auto entry = std::make_unique<VariableListEntry>();
    entry->setText(name);
    entry->markPresentInUi();
    list.addObject(std::move(entry));
}
