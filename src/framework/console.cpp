
/*
 *
 *  A simple console machine for nekohook
 *
 *
 */

#include <unordered_map>

#include "../util/stringhelpers.hpp" // sepstr()
#include "../util/logging.hpp"

#include "console.hpp"

// The main list to store cat commands
static std::unordered_map<std::string, CatComBase*> CatCommandList;

CatComBase::CatComBase(std::string _command_name)
  : command_name(_command_name) {
  //CatCommandList.insert({command_name, this});
}

CatComBase::~CatComBase() {
  //CatCommandList.erase(command_name);
}

void CallCommand(std::string input) {

  /*// Seperate everything in the string to components
  auto tmp = sepstr(input);
  if (tmp.empty()) {
    g_CatLogging.log("Catcommand empty.");
    return;
  }

  // Try to find command from command list
  auto find_com = CatCommandList.find(tmp[0]);
  if (find_com == CatCommandList.end()) {
    g_CatLogging.log("Cannot find command.");
    return;
  }

  // We have our command, pop it out of the vector and pass it as arguments
  tmp.erase(0);
  find_com.second->callback(tmp);*/
}
