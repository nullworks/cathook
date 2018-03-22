
/*
 * stringhelpers.cpp
 *
 *  Created on: Sep 17, 2016
 *      Author: oneechan
 */

#include <regex> // stronk

#include "../framework/drawing.hpp"

#include "strings.hpp"

// Seperates multiple strings from one string, seperated by spaces, or enclosed in quotes
std::vector<std::string> sepstr(std::string input) {
  std::vector<std::string> ret;

  std::string tmp;
  char quote_enclosed = '\0'; // the last quote we recieved
  bool escaped = false;
  for (const char& i : input){
    // Check if we encountered a quote, we want to consume it if its not already within quotes or escaped
    if ((i == '\"' || i == '\'') && !escaped) {
      // Encountered our starting quote
      if (quote_enclosed == '\0') {
        quote_enclosed = i;
        continue;
      // if we encountered our closing quote, we can push to ret
      } else if (quote_enclosed == i) {
        quote_enclosed = '\0';
        ret.push_back(tmp);
        tmp.clear();
        continue;;
      }
    }

    // Check for escape
    if (i == '\\' && !escaped) {
      escaped = true;
      continue;
    }

    // if we encountered a space and arent in a quote, we can push the string to ret
    if (i == ' ' && quote_enclosed == '\0' && !tmp.empty()) {
      ret.push_back(tmp);
      tmp.clear();
    } else
      // Add our char
      tmp.push_back(i);

    // reset escaped status
    escaped = false;
  }
  // push whats left over to ret
  if (!tmp.empty())
    ret.push_back(tmp);
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
