
/*
 *
 *
 *
 *
 */

#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "../util/functions.hpp"

#define COM_PREFIX "neko_"

// The base command class
class CatComBase {
public:
  virtual void callback(std::vector<std::string>) = 0;
};

class CatCommand : public CatComBase {
public:
  CatCommand(std::string _name, void(*_com_callback)(std::vector<std::string>));
  virtual void callback(std::vector<std::string> args) { com_callback(args); };
  std::string name;
  CMFunction<void(std::vector<std::string>)> com_callback = nullptr;
  inline void operator()(std::vector<std::string> args) { com_callback(args); }
};

void CallCommand(std::string input);

extern std::unordered_map<std::string, CatCommand*> CatCommandMap;
