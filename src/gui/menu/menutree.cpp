 
/*
 *
 *	This file helps dynamicly create a menu tree for the menu to use.
 *
 */

#include "menutree.hpp"

namespace gui { namespace menu {

// Constructor
CMenuTree(const char* string) name(string){}
	
// When you construct with a catvar for the input, it does logic here
void CMenuTree::AddTree(const CatVar& cat_var, const int& recursions) {
	// Check if we reached the end if the enum info, if not we can add more to the tree
	if (cat_var.gui_position.size() > recursions) {
		
		// Look through the children and if any have the name of one we might want to make, we can reuse the branch
		for (CMenuTree& tree_branch : this.children) {
			// Test if this is an existing branch with matching names
			if (tree_branch.name != std::string(cat_var.gui_position[recursions])) continue;
			// We found our branch, recurse into it
			tree_branch.AddTree(car_var, recursions + 1);
			return;
		}
		
		// We dont have a branch already so we must make a new one and recurse into it.
		CMenuTree sapling = CMenuTree(cat_var.gui_position[recursions]);
		this.children.push_back(sapling);
	// This is for when we finished recursing
	} else {
		menu_tree->cat_children.push_back(cat_var);
	}
}

// Use for when you wish to request the menu tree
const CMenuTree& GetMenuTree() {
	static CMenuTree menu_root;
	static menu_built = false;
	if (!menu_built) {
		for (const auto& cat_var : CatVarList) {
			menu_root.AddTree(cat_var.second);
		}
		menu_built = true;
	}
	// If we already have a tree, return it
	return menu_root;
}
	
}}

