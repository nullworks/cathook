
/*
 * stringhelpers.cpp
 *
 *  Created on: Sep 17, 2016
 *      Author: oneechan
 */

#include "stringhelpers.hpp"

// Seperates multiple strings from one string, seperated by spaces
std::vector<std::string> sepstr(std::string input) {

  // Our return vector
  std::vector<std::string> ret;

  // A temperary string
  std::string tmp = "";

  bool found = false; // When we start to read something, we need to know that we arent within a bunch of spaces
  for (int i = 0; i < input.size(); i++) {
    // If we havent found anything and our char is a space and there isnt a backslash behind it, just continue on
    if (input[i] == ' ') {
      // If we havent found anything and its a space char, continue
      if(!found) continue;
      // If we found something and we hit a space char, make sure there isnt a backslash. If we dont have one, we will push the string we have into the vector and continue on.
      else if (input[i - 1] != '\\') {
        // Push to ret and empty string
        ret.push_back(tmp);
        tmp.clear();
        // Set found status
        found = false;
        continue;
      }
    }
    found = true; // Well, looks good

    // Add our current char to the tmp string and iterate
    tmp.push_back(input[i]);
  }
  // If we still have stuff in the temporary string, we add that to the return vector
  if (!tmp.empty()) ret.push_back(tmp);

  // Return our return vector
  return ret;
}
