
/*
 * stringhelpers.cpp
 *
 *  Created on: Sep 17, 2016
 *      Author: oneechan
 */

#include <regex> // stronk

#include "../framework/drawing.hpp"

#include "strings.hpp"

// Seperates multiple strings from one string, seperated by spaces
std::vector<std::string> sepstr(std::string input) { // TODO, add correction to handle unescaped strings
  std::vector<std::string> ret;
  std::smatch reg_result;
  while (std::regex_search(input, reg_result, std::regex("[\\S]+"))) { // regex is really slow and adds alot to file size, but is really convienient for this purpose
    ret.push_back(reg_result[0]);
    input = reg_result.suffix();
  }
  return ret;
}

// Used to simplify strings
std::string reduce_str(std::string in) {
  for (auto& i : in) {
    i = std::tolower(i); // Make it lowercase
    /*switch(i) { // fix leetspeak
    case '4': i = 'a'; break;
    case '3': i = 'e'; break;
    case '0': i = 'o'; break;
    case '6': i = 'g'; break;
    case '5': i = 's'; break;
    case '7': i = 't'; break;
    case '-':
    case '_': i = ' '; break;
  }*/
  }
  return in;
}
