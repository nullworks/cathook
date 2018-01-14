
/*
 *
 *  This is the catvars cpp file.
 *  Catvars are variables intended to be interacted with by a user.
 *  They could be simple command line variables or variables in a gui.
 *  Everything like that goes in here!
 *
 */

#include <exception>

#include "logging.hpp"

#include "catvars.hpp"

// The CatCommand map
std::unordered_map<std::string, CatVar*> __attribute__ ((init_priority (102))) CatVarMap; // Need to init this before other catvars are inited
// Our Menu tree
CatMenuTree CatMenuRoot;

// Menu tree
void CatMenuTree::AddTree(CatVar* cat_var, int recursions) {
	// Check if we reached the end if the enum info, if not we can add more to the tree
	if (cat_var->gui_position.size() <= recursions) {
    cat_children.push_back(cat_var); // We finished recursing
    return;
  }

  // Look through the children and if any have the name of one we might want to make, we can reuse the branch
  for (auto& tree_branch : children) {
    // Test if this is an existing branch with matching names
    if (tree_branch.name == cat_var->gui_position[recursions]) continue;
    // We found our branch, recurse into it
    tree_branch.AddTree(cat_var, recursions + 1);
    return;
  }
  // We dont have a branch already so we must make a new one and recurse into it.
  auto sapling = CatMenuTree(cat_var->gui_position[recursions]);
  children.push_back(sapling);
  sapling.AddTree(cat_var, recursions + 1);
}

// General catvar constructor
CatVar::CatVar(const CatEnum& _gui_position, std::string _name, std::string _desc_short, std::string _desc_long)
	: gui_position(_gui_position), name(_name), desc_short(_desc_short), desc_long(_desc_long), CatComBase("cat_" + _name) {
	CatVarMap.insert({name, this}); // Broken
  // Add the catvar to the menu tree
  CatMenuRoot.AddTree(this);
}
// Catvar Constructors
CatVarBool::CatVarBool(const CatEnum& _gui_position, std::string _name, bool _defaults, std::string _desc_short, std::string _desc_long)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults) {}
CatVarInt::CatVarInt(const CatEnum& _gui_position, std::string _name, int _defaults, std::string _desc_short, std::string _desc_long, int _min, int _max)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults), min(_min), max(_max) {}
CatVarFloat::CatVarFloat(const CatEnum& _gui_position, std::string _name, float _defaults, std::string _desc_short, std::string _desc_long, float _min, float _max)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults), min(_min), max(_max) {}
CatVarString::CatVarString(const CatEnum& _gui_position, std::string _name, std::string _defaults, std::string _desc_short, std::string _desc_long)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults) {}
CatVarColor::CatVarColor(const CatEnum& _gui_position, std::string _name, CatVector4 _defaults, std::string _desc_short, std::string _desc_long)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults) {}

// Command Callbacks
void CatVarBool::callback(std::vector<std::string> args) {
	// Empty args
	if (args.empty()) {
		g_CatLogging.log("%s: %s", name.c_str(), (value) ? "true" : "false");
		return;
	}
	// Number
	try {
		value = std::stoi(args[0]);
	// String
} catch (std::exception& e) {
		if (args[0] == "true")
			value = true;
		else if (args[0] == "false")
			value = false;
	}
}
void CatVarInt::callback(std::vector<std::string> args) {
	// Empty args
	if (args.empty()) {
		g_CatLogging.log("%s: %i", name.c_str(), value);
		return;
	}
	try {
		value = std::stoi(args[0]);
	} catch (std::exception& e){
		g_CatLogging.log("Exception: %s", e.what());
	}
}
void CatVarEnum::callback(std::vector<std::string> args) {
	// Empty args
	if (args.empty()) {
		g_CatLogging.log("%s: %f", name.c_str(), value);
		return;
	}
	// int input
	try {
		value = std::stoi(args[0]);
	// Text
	} catch (std::exception& e){
		for (int i = 0; i < cat_enum.size(); i++) {
			if (cat_enum[i] == args[0]) {
				value = i;
				return;
			}
		}
		g_CatLogging.log("No value in \"%s\" found for \"%s\"", name.c_str(), args[0]);
	}
}
void CatVarKey::callback(std::vector<std::string> args) {
	// Empty args
	if (args.empty()) {
		g_CatLogging.log("%s: %i", name.c_str(), value);
		return;
	}
	// int input
	try {
		value = std::stoi(args[0]);
	// Text
	} catch (std::exception& e){
		// TODO, look for catkey names
		/*for (int i = 0; i < cat_enum.size(); i++) {
			if (cat_enum[i] == args[0]) {
				value = i;
				return;
			}
		}*/
		g_CatLogging.log("No value in \"%s\" found for \"%s\"", name.c_str(), args[0]);
	}
}
void CatVarFloat::callback(std::vector<std::string> args) {
	// Empty args
	if (args.empty()) {
		g_CatLogging.log("%s: %f", name, value);
		return;
	}
	try {
		value = std::stof(args[0]);
	} catch (std::exception& e) {
		g_CatLogging.log("Exception: %s", e.what());
	}
}
void CatVarString::callback(std::vector<std::string> args) {
	// Empty args
	if (args.empty()) {
		g_CatLogging.log("%s: %s", name.c_str(), value.c_str());
		return;
	}
	value = args[0];
}
void CatVarColor::callback(std::vector<std::string> args) {
	// Empty args
	if (args.empty()) {
		g_CatLogging.log("%s: R:%f, G:%f, B:%f, A:%f", name.c_str(), value.x, value.y, value.z, value.a);
		return;
	}
	auto size = args.size();
	try {
		value.x = std::stof(args[0]);
		if (size >= 2) {
			value.y = std::stof(args[1]);
			if (size >= 3) {
				value.z = std::stof(args[2]);
				if (size >= 4) {
					value.a = std::stof(args[3]);
				}
			}
		}
	} catch (std::exception& e) {
		g_CatLogging.log("Exception: %s", e.what());
	}
}
