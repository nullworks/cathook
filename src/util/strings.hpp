
/*
 *
 *	String helpers header
 *
 */

#pragma once

#include <cstring> // strncpy()
#include <string>
#include <vector>

// Give it a buffer, the string to pull from, and the bounds to use. Keep in mind, this expects correct bounds and such.
inline void substr(char* buffer, const char* input, int start, int end) {
	strncpy(buffer, &input[start], end);
}

// Seperates multiple strings from one string, seperated by spaces
std::vector<std::string> sepstr(std::string);

// Used to simplify strings
std::string reduce_str(std::string in);

// A "Fuzzy" strstr
inline const char* fuzstrstr(const char* main, const char* sub) {
  auto find = reduce_str(main).find(reduce_str(sub));
  if (find == std::string::npos) // To retain normal strstr behaviour
    return nullptr;
  return &main[find];
}
inline size_t fuzstrstr(const std::string& main, const std::string& sub) { // std::string version
  return reduce_str(main).find(reduce_str(sub));
}
// A "fuzzy" strcmp
inline auto fuzstrcmp(const char* str1, const char* str2) {
	return strcmp(reduce_str(str1).c_str(), reduce_str(str2).c_str());
}
inline auto fuzstrcmp(const std::string& str1, const std::string& str2) { // std::string version
	return reduce_str(str1) == reduce_str(str2);
}
