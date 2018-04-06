
/*
 *
 *
 *
 *
 */

#pragma once

#include <string>
#include <vector>

#include "../util/functional.hpp"

// A define for commands to use in front of their actual command
#define COM_PREFIX "n_"

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
  inline void operator()(std::vector<std::string> args) { com_callback(args); }
private:
  CMFunction<void(std::vector<std::string>)> com_callback;
};

void CallCommand(const std::string& input);

extern std::vector<CatCommand*> CatCommandList;
