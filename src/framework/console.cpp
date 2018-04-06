
/*
 *
 *  A simple console machine for nekohook
 *
 *
 */

#include <algorithm> // std::find_if

#include "../util/strings.hpp" // sepstr()
#include "../util/logging.hpp"
#include "../util/catvars.hpp"

#include "console.hpp"

// The main list to store cat commands
std::vector<CatCommand*> CatCommandList;

CatCommand::CatCommand(std::string _name, void(*_callback)(std::vector<std::string>))
  : name(COM_PREFIX + _name), com_callback(_callback) {
  CatCommandList.push_back(this);
}

void CallCommand(const std::string& input) {

  // Seperate everything in the string to components
  auto tmp = sepstr(input);
  if (tmp.empty()) {
    g_CatLogging.log("Catcommand empty: \"%s\"", input.c_str());
    return;
  }

  // Try to find command from command list
  CatComBase* command = nullptr;
  auto find_com = std::find_if(CatCommandList.begin(), CatCommandList.end(), [&](auto i){ return tmp[0] == i->name; });
  if (find_com != CatCommandList.end())
    command = *find_com;

  // Find a CatVar instead
  if (!command) {
    auto find_var = std::find_if(CatVar::CatVarList.begin(), CatVar::CatVarList.end(), [&](auto i){ return tmp[0] == i->name; });
    if (find_var == CatVar::CatVarList.end()) {
      g_CatLogging.log("Cannot find command: \"%s\"", tmp.at(0).c_str());
      return;
    }
    command = *find_var;
  }

  // We have our command, pop it out of the vector and pass it as arguments
  tmp.erase(tmp.begin());
  command->callback(tmp);
}
