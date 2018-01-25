
/*
 *
 *  This file handles loading and executing configs
 *
 *
 */

#include "config.hpp"

#define SAVE_LOC \
#if defined(__)
void LoadConfig(std::string cfg_name) {
  // Ensure we have .cfg to make loading file easy
  if (cfg_name.find(".cfg") == std::string::npos)
    cfg_name += ".cfg";
}
