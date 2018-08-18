/*
  Created on 02.07.18.
*/

#include <settings/Settings.hpp>

void settings::registerVariable(IVariable &variable, std::string name)
{
    Manager::instance().add(variable, name);
}
