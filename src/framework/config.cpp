
/*
 *
 *  This file handles loading and executing configs
 *
 *
 */

#include "../util/logging.hpp"
#include "../util/catvars.hpp"
#include "../util/iohelper.hpp"
#include "game.hpp" // to get ingame state
#include "gameticks.hpp"
#include "console.hpp"

#include "config.hpp"

namespace configs {

CatCommand LoadConfig("load", [](std::vector<std::string> args) {

  // args check
  if (args.empty()) {
    g_CatLogging.log("Missing input argument");
    return;
  }

  // Ensure we have .cfg to make loading file easy
  if (args.at(0).find(".cfg") == std::string::npos)
    args.at(0) += ".cfg";

  // Get where the file is likely to be stored
  args.at(0) = io::GetSaveLocation() + "cfg/cat" + args.at(0);

  // Read from the file
  auto tmp = io::ReadFile(args.at(0));
  if (tmp.empty()) // if empty, something probs happened and we wont be able to load the config
    return;

  // Call all of the lines as commands
  for (auto i : tmp)
    CallCommand(i);

  g_CatLogging.log("Loaded %s successfully!", args.at(0).c_str());
});

CatCommand SaveConfig("save", [](std::vector<std::string> args) {

  // args check
  if (args.empty()) {
    g_CatLogging.log("Missing input argument");
    return;
  }

  // Ensure we have .cfg to make saving file easy
  if (args.at(0).find(".cfg") == std::string::npos)
    args.at(0) += ".cfg";

  // Make it a path to file
  args.at(0) = io::GetSaveLocation() + "cfg/cat" + args.at(0);

  // Make directories if needed
  io::CreateDirectorys(io::GetSaveLocation() + "cfg");

	// Generate a list of commands from catvars
  std::vector<std::string> cfg_lines;
	for (const auto& catvar : CatVarMap) {
    // CatVarEnums need quotes as their values can have spaces in them
    if (dynamic_cast<CatVarEnum*>(catvar.second))
      cfg_lines.push_back(catvar.second->name + " \"" + catvar.second->GetValue() + '"');
    else
      cfg_lines.push_back(catvar.second->name + ' ' + catvar.second->GetValue());
  }

  io::WriteFile(args.at(0), cfg_lines);

  g_CatLogging.log("Saved %s successfully!", args.at(0).c_str());
});

std::vector<std::string> ListConfigs() {
  return std::vector<std::string>();
}

static CatCommand ListConfig("list_configs", [](std::vector<std::string> args) {
  g_CatLogging.log("TODO!", args.at(0).c_str());
});

static void WorldTick() {

  // Recreation of match exec
  static bool last_ingame = false;
  auto ingame = game::GetInGame();
  if (last_ingame && last_ingame != ingame)
    LoadConfig({"matchexec"});
  last_ingame = ingame;
}

void Init() {
  wtickmgr.REventBefore(WorldTick);
  LoadConfig({"autoexec"});
}

}
