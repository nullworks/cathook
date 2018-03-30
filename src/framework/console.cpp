
/*
 *
 *  A simple console machine for nekohook
 *
 *
 */

#include "../util/strings.hpp" // sepstr()
#include "../util/logging.hpp"
#include "../util/catvars.hpp"

#include "console.hpp"

// The main list to store cat commands
std::unordered_map<std::string, CatCommand*> __attribute__ ((init_priority (102))) CatCommandMap;

CatCommand::CatCommand(std::string _name, void(*_callback)(std::vector<std::string>))
  : name(COM_PREFIX + _name), com_callback(_callback) {
  CatCommandMap.insert({name, this});
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
  auto find_com = CatCommandMap.find(tmp.at(0));
  if (find_com != CatCommandMap.end())
    command = find_com->second;

  // Find a CatVar instead
  if (!command) {
    auto find_var = CatVarMap.find(tmp.at(0));
    if (find_var == CatVarMap.end()) {
      g_CatLogging.log("Cannot find command: \"%s\"", tmp.at(0).c_str());
      return;
    }
    command = find_var->second;
  }

  // We have our command, pop it out of the vector and pass it as arguments
  tmp.erase(tmp.begin());
  command->callback(tmp);
}
