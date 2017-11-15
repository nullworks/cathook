
/*
 *
 *  This is the catvars cpp file.
 *  Catvars are variables intended to be interacted with by a user.
 *  They could be simple command line variables or variables in a gui.
 *  Everything like that goes in here!
 *
 */

#include "catvars.hpp"

// The CatCommand map
std::unordered_map<std::string, CatVar&> CatCommandMap;
// Our Menu tree
CatMenuTree CatMenuRoot();

// Menu tree
void CatMenuTree::AddTree(const CatVar& cat_var, int recursions) {
	// Check if we reached the end if the enum info, if not we can add more to the tree
	if (cat_var.gui_position.size() <= recursions) {
    menu_tree->cat_children.push_back(cat_var); // We finished recursing
    return;
  }
  // Look through the children and if any have the name of one we might want to make, we can reuse the branch
  for (const CMenuTree& tree_branch : this.children) {
    // Test if this is an existing branch with matching names
    if (strcmp(tree_branch.name, cat_var.gui_position[recursions])) continue;
    // We found our branch, recurse into it
    tree_branch.AddTree(cat_var, recursions + 1);
    return;
  }
  // We dont have a branch already so we must make a new one and recurse into it.
  CMenuTree sapling = CMenuTree(cat_var.gui_position[recursions]);
  this.children.push_back(sapling);
  tree_branch.AddTree(cat_var, recursions + 1);
}
// General catvar constructor
void CatVar::Init() {
  // Add the catvar to the command map
  CatCommandMap.insert({name, this});
  // Add the catvar to the menu tree
  CatMenuRoot.AddTree(this);
}
// Catvar Constructors
CatVarBool(const CatEnum& _gui_position, const char* _name, const bool& _defaults, const char* _desc_short, const char* _desc_long)
  : gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long) {
    Init();
}
CatVarInt(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long, const int& _max)
  : gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long), min(0), max(_max) {
    Init();
}
CatVarInt(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long, const int& _min, const int& _max)
  : gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long), min(_min), max(_max) {
    Init();
}
CatVarFloat(const CatEnum& _gui_position, const char* _name, const float& _defaults, const char* _desc_short, const char* _desc_long, const float& _max)
  : gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long), min(0), max(_max) {
    Init();
}
CatVarFloat(const CatEnum& _gui_position, const char* _name, const float& _defaults, const char* _desc_short, const char* _desc_long, const float& _min, const float& _max)
  : gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long), min(_min), max(_max) {
    Init();
}
CatVarKey(const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long)
  : gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long) {
    Init();
}
CatVarString(const CatEnum& _gui_position, const char* _name, const char* _defaults, const char* _desc_short, const char* _desc_long)
  : gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long) {
    Init();
}
CatVarColor(const CatEnum& _gui_position, const char* _name, const CatVector4& _defaults, const char* _desc_short, const char* _desc_long)
  : gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long) {
    Init();
}
CatVarEnum(const CatEnum& _cat_enum, const CatEnum& _gui_position, const char* _name, const int& _defaults, const char* _desc_short, const char* _desc_long)
  : cat_enum(_cat_enum), gui_position(_gui_position), name(_name), default(_defaults), desc_short(_desc_short), desc_long(_desc_long), min(0), max(_cat_enum.size() - 1) {
    Init();
}
