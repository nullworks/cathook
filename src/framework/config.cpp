
/*
 *
 *  This file handles loading and executing configs
 *
 *
 */

#include <fstream> // to read/write to files

#include "../util/logging.hpp"
#include "../util/catvars.hpp"
#include "../util/iohelper.hpp"
#include "game.hpp" // to get ingame state
#include "gameticks.hpp"
#include "console.hpp"

#include "config.hpp"

namespace configs {

constexpr const char* SAVE_LOC() {
  #if defined(__linux__)
    return "~/.config/nekohook/";
  #endif
}

CatCommand LoadConfig("load", [](std::vector<std::string> args) {

  // args check
  if (args.empty()) {
    g_CatLogging.log("Missing input argument");
    return;
  }
  auto cfg_name = args[0];

  // Ensure we have .cfg to make loading file easy
  if (cfg_name.find(".cfg") == std::string::npos)
    cfg_name += ".cfg";

  try {
    // Open the file
  	std::ifstream cfg_stream(std::string(SAVE_LOC()) + "cfg/cat" + cfg_name);
    if (!cfg_stream.is_open()) {
      g_CatLogging.log("Couldnt Open %s", cfg_name.c_str());
      return;
    }

  	// Recurse through the lines of the file
  	while (!cfg_stream.eof()) {
  		// Get our line
  		char buffer[256];
  		cfg_stream.getline(buffer, sizeof(buffer));
      // Execute it
      CallCommand(buffer);
    }
    g_CatLogging.log("Loaded %s successfully!", cfg_name.c_str());
  } catch (const std::ios_base::failure& e) {
    g_CatLogging.log("Couldnt Load Config: %s", e.what());
  }
});

CatCommand SaveConfig("save", [](std::vector<std::string> args) {

  // args check
  if (args.empty()) {
    g_CatLogging.log("Missing input argument");
    return;
  }
  auto cfg_name = args[0];

  // Ensure we have .cfg to make saving file easy
  if (cfg_name.find(".cfg") == std::string::npos)
    cfg_name += ".cfg";

  // Make it a path to file
  cfg_name = std::string(SAVE_LOC()) + "cfg/cat" + cfg_name;

  // Make directories if needed
  CreateDirectorys(cfg_name);

  try {
    // Create the file stream
    std::fstream cfg_stream(cfg_name, std::ios::out | std::ios_base::trunc);
    if (!cfg_stream.is_open()) {
      g_CatLogging.log("Couldnt Open %s", cfg_name.c_str());
      return;
    }

  	// Recurse through the lines of the file
  	for (const auto& catvar : CatVarMap) {
      // Write a command to file
      std::string command = catvar.second->name + ' ' + catvar.second->GetValue() + '\n';
  		cfg_stream.write(command.c_str(), command.size());
    }
    g_CatLogging.log("Saved %s successfully!", cfg_name.c_str());
  } catch (const std::ios_base::failure& e) {
    g_CatLogging.log("Couldnt Save Config: %s", e.what());
  }
});

std::vector<std::string> ListConfigs() {
  return std::vector<std::string>();
}

void WorldTick() {

  // Recreation of match exec
  static bool last_ingame = false;
  if (last_ingame && last_ingame != g_GameInfo.in_game)
    CallCommand(COM_PREFIX "load catmatchexec.cfg");
  last_ingame = g_GameInfo.in_game;
}

void Init() {
  wtickmgr.REventBefore(WorldTick);
  CallCommand(COM_PREFIX "load catautoexec.cfg");
}

}
