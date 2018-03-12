
/*
 * stringhelpers.cpp
 *
 *  Created on: Sep 17, 2016
 *      Author: oneechan
 */

#include <regex>

#include "../framework/drawing.hpp"

#include "strings.hpp"

// Seperates multiple strings from one string, seperated by spaces
std::vector<std::string> sepstr(std::string input) {

  // Get our strings
  std::vector<std::string> ret;
  std::smatch reg_result;
  while (std::regex_search(input, reg_result, std::regex("[\\S]+"))) { // regex is really slow and adds alot to file size, but is really convienient for this purpose
    ret.push_back(reg_result[0]);
    input = reg_result.suffix();
  }
  return ret;
}
