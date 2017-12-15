
/*
 *
 *  This is the catvars cpp file.
 *  Catvars are variables intended to be interacted with by a user.
 *  They could be simple command line variables or variables in a gui.
 *  Everything like that goes in here!
 *
 */

#include <string.h>

#include "catvars.hpp"

// The CatCommand map
std::unordered_map<std::string, CatVar*> __attribute__ ((init_priority (102))) CatCommandMap; // Need to init this before other catvars are inited
// Our Menu tree
CatMenuTree CatMenuRoot;

// Menu tree
void CatMenuTree::AddTree(CatVar& cat_var, int recursions) {
	// Check if we reached the end if the enum info, if not we can add more to the tree
	if (cat_var.gui_position.size() <= recursions) {
    cat_children.push_back(&cat_var); // We finished recursing
    return;
  }

  // Look through the children and if any have the name of one we might want to make, we can reuse the branch
  for (auto& tree_branch : children) {
    // Test if this is an existing branch with matching names
    if (!strcmp(tree_branch.name, cat_var.gui_position[recursions])) continue;
    // We found our branch, recurse into it
    tree_branch.AddTree(cat_var, recursions + 1);
    return;
  }
  // We dont have a branch already so we must make a new one and recurse into it.
  auto sapling = CatMenuTree(cat_var.gui_position[recursions]);
  children.push_back(sapling);
  sapling.AddTree(cat_var, recursions + 1);
}
// General catvar constructor
CatVar::CatVar(const CatEnum& _gui_position, const char* _name, const char* _desc_short, const char* _desc_long)
	: gui_position(_gui_position), name(_name), desc_short(_desc_short), desc_long(_desc_long) {
	// Add the catvar to the command map
  CatCommandMap.insert({name, this}); // Broken
  // Add the catvar to the menu tree
  CatMenuRoot.AddTree(*this);
}
// Catvar Constructors
CatVarBool::CatVarBool(const CatEnum& _gui_position, const char* _name, const bool& _defaults, const char* _desc_short, const char* _desc_long)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults) {}
CatVarInt::CatVarInt(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long, const int& _max)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults), min(0), max(_max) {}
CatVarInt::CatVarInt(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long, const int& _min, const int& _max)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults), min(_min), max(_max) {}
CatVarFloat::CatVarFloat(const CatEnum& _gui_position, const char* _name, const float& _defaults, const char* _desc_short, const char* _desc_long, const float& _max)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults), min(0), max(_max) {}
CatVarFloat::CatVarFloat(const CatEnum& _gui_position, const char* _name, const float& _defaults, const char* _desc_short, const char* _desc_long, const float& _min, const float& _max)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults), min(_min), max(_max) {}
CatVarKey::CatVarKey(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults) {}
CatVarString::CatVarString(const CatEnum& _gui_position, const char* _name, const char* _defaults, const char* _desc_short, const char* _desc_long)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults) {}
CatVarColor::CatVarColor(const CatEnum& _gui_position, const char* _name, const CatVector4& _defaults, const char* _desc_short, const char* _desc_long)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults) {}
CatVarEnum::CatVarEnum(const CatEnum& _cat_enum, const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long)
  : CatVar(_gui_position, _name, _desc_short, _desc_long), defaults(_defaults), value(_defaults), min(0), max(_cat_enum.size() - 1), cat_enum(_cat_enum) {}
