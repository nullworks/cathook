
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
std::vector<std::string> sepstr(const std::string& input) {

  // Get our strings
  std::vector<std::string> ret;
  std::smatch reg_result;
  auto search_str = input;
  while (std::regex_search(search_str, reg_result, std::regex("[\\S]+"))) { // regex is really slow and adds alot to file size, but is really convienient for this purpose
    ret.push_back(reg_result[0]);
    search_str = reg_result.suffix();
  }
  return ret;
}

std::string wordwrap(const std::string& in, int max_x) {

  // Our return
  std::string ret = in;

  // Get tooltip size
  auto tooltip_size = draw::GetStringLength(in.c_str(), 1, 20);

  // Check if we need to wrap x
  if (tooltip_size.first < max_x) {
    // Here we take each word and make it fit in our box.
    std::string wrapped_x;
    std::string cur_line;
    for (auto cur_str : sepstr(ret)) {
      // String goes over max x, then add curline
      if (draw::GetStringLength((cur_line + " " + cur_str).c_str(), 1, 20).first > max_x) {
        // Add the current line
        wrapped_x += cur_line + '\n';
        cur_line = cur_str;
      // String didnt go over max x
      } else {
        // Add the current string to the line
        cur_line += " " + cur_str;
      }
    }
  }
  return ret;
}
